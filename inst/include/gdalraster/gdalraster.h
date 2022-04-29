#ifndef GDALRASTER_H
#define GDALRASTER_H
#include <Rcpp.h>
#include "gdal_priv.h"
#include "gdal_utils.h"  // for GDALTranslateOptions
#include "vrtdataset.h"

namespace gdalraster {
using namespace Rcpp;

// for GHRSST this is what works
// "NETCDF:\"https://podaac-opendap.jpl.nasa.gov/opendap/allData/ghrsst/data/GDS2/L4/GLOB/JPL/MUR/v4.1/2022/114/20220424090000-JPL-L4_GHRSST-SSTfnd-MUR-GLOB-v02.0-fv04.1.nc\":analysed_sst"
//  netcdf: https://podaac-opendap.jpl.nasa.gov/opendap/allData/ghrsst/data/GDS2/L4/GLOB/JPL/MUR/v4.1/2022/114/20220424090000-JPL-L4_GHRSST-SSTfnd-MUR-GLOB-v02.0-fv04.1.nc

// topos <- c("/vsicurl/https://public.services.aad.gov.au/datasets/science/GEBCO_2019_GEOTIFF/GEBCO_2019.tif",
//            "/vsicurl/https://opentopography.s3.sdsc.edu/raster/NASADEM/NASADEM_be.vrt")
//
//   file <-"../vapour/inst/extdata/volcano.tif"
// gdalmin:::open_gdal(file)
//   gdalmin:::open_gdal(c(file, topos))
//


// does it have subdatasets?
inline bool gdal_has_subdataset(GDALDataset *poDataset) {
  // not faster, we good
  //if (poDataset->GetRasterCount() > 0) return false;
  char **MDdomain = GDALGetMetadataDomainList(poDataset);
  int mdi = 0; // iterate though MetadataDomainList
  bool has_sds = false;
  while (MDdomain && MDdomain[mdi] != NULL) {
    if (strcmp(MDdomain[mdi], "SUBDATASETS") == 0) {
      has_sds = true;
      //cleanup
      CSLDestroy(MDdomain);
      return has_sds;
    }
    mdi++;
  }
  //cleanup
  CSLDestroy(MDdomain);
  return has_sds;
}

// obtain the i-th subdataset *name* (i is 1-based)
inline CharacterVector gdal_subdataset_1(GDALDataset *poDataset, int i_sds) {
  // need error handling on this is so janky
  int sdi = 0;
  // don't call this function without calling has_subdatasets() first
  // owned by the object
  CharacterVector ret(1);
  
  char **SDS2 = poDataset->GetMetadata("SUBDATASETS");
  while (SDS2 && SDS2[sdi] != NULL) {
    if (sdi * 2 == (i_sds - 1)) {
      // ii*2 because SDS tokens come in pairs
      char  **papszTokens = CSLTokenizeString2(SDS2[sdi * 2], "=", 0);
      ret[0] = papszTokens[1];
      CSLDestroy( papszTokens );
      break;
    }
    sdi++; // count
  }
  return ret;
}

inline CharacterVector gdal_list_subdatasets(GDALDataset *poDataset) {
  // need error handling on this is so janky
  int sdi = 0;
  // don't call this function without calling has_subdatasets() first
  // owned by the object
  char **SDS = GDALGetMetadata(poDataset, "SUBDATASETS");
  while (SDS && SDS[sdi] != NULL) {
    sdi++; // count
  }
  // FIXME
  if (sdi < 1) return "";
  if (!(sdi % 2 == 0)) return "";
  // we only want the first of each pair
  int dscount = sdi / 2;
  Rcpp::CharacterVector ret(dscount);
  if (dscount > 0) {
    // we have subdatasets, so list them all
    // owned by the object
    char **SDS2 = GDALGetMetadata(poDataset, "SUBDATASETS");
    for (int ii = 0; ii < dscount; ii++) {
      // ii*2 because SDS tokens come in pairs
      char  **papszTokens = CSLTokenizeString2(SDS2[ii * 2], "=", 0);
      ret(ii) = papszTokens[1];
      CSLDestroy( papszTokens );
    }
  }
  return ret;
}

// open the DSN, open a subdataset if sds > 0 (else you get the outer shell)
inline GDALDatasetH gdalH_open_dsn(const char * dsn, IntegerVector sds) {
  GDALDatasetH DS = GDALOpen(dsn, GA_ReadOnly);
  if (sds[0] > 0 && gdal_has_subdataset((GDALDataset*) DS)) {
    CharacterVector sdsnames = gdal_subdataset_1((GDALDataset*)DS, sds[0]);
    if (sdsnames.length() > 0 && !sdsnames[0].empty()) {
      GDALClose((GDALDataset*) DS);
      DS = GDALOpen(sdsnames[0], GA_ReadOnly);
    }
    
  }
  return DS;
}


// open the DSN with gdalH_open_dsn() but translate it to VRT with extent (=a_ullr) and/or projection (=a_srs)
inline GDALDatasetH gdalH_open_avrt(const char* dsn, NumericVector extent, CharacterVector projection, IntegerVector sds, IntegerVector bands) {
  
  CPLStringList translate_argv;
  translate_argv.AddString("-of");
  translate_argv.AddString("VRT");
  if (extent.size() == 4) {
    translate_argv.AddString("-a_ullr");
    translate_argv.AddString(CPLSPrintf("%f", extent[0]));
    translate_argv.AddString(CPLSPrintf("%f", extent[3]));
    translate_argv.AddString(CPLSPrintf("%f", extent[1]));
    translate_argv.AddString(CPLSPrintf("%f", extent[2]));
  }
  if (!projection[0].empty()) {
    // have to validate this
    OGRSpatialReference srs;
    
    if (srs.SetFromUserInput(projection[0]) != OGRERR_NONE) {
      Rprintf("cannot set projection (CRS) from input 'projection' argument, ignoring: '%s'\n", (const char*)projection[0]);
    } else {
      translate_argv.AddString("-a_srs");
      translate_argv.AddString(projection[0]);
    }
  }
  if (bands[0] > 0) {
    for (int iband = 0; iband < bands.size(); iband++ ) {
      if (bands[iband] <= 0) {
        // nothing
      } else {
        translate_argv.AddString("-b");
        translate_argv.AddString(CPLSPrintf("%i", bands[iband]));
      }
    }
  }  
  GDALTranslateOptions* psTransOptions = GDALTranslateOptionsNew(translate_argv.List(), nullptr);
  GDALDatasetH a_DS = GDALTranslate("", (GDALDataset*)gdalH_open_dsn(dsn, sds), psTransOptions, nullptr);
  GDALTranslateOptionsFree( psTransOptions );
  return a_DS;
}

// open the DSN/s with gdalH_open_dsn(), possibly with a sds (1-based)
// (nothing uses this function)
inline GDALDatasetH* gdalH_open_multiple(CharacterVector dsn, IntegerVector sds) {
  
  GDALDatasetH* poHDS;
  // whoever calls this will have to CPLFree() this
  poHDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * dsn.size()));
  for (int i = 0; i < dsn.size(); i++) poHDS[i] = gdalH_open_dsn(dsn[i], sds);
  return poHDS;
}
inline GDALDatasetH* gdalH_open_avrt_multiple(CharacterVector dsn, NumericVector extent, CharacterVector projection, IntegerVector sds, IntegerVector bands) {
  
  GDALDatasetH* poHDS;
  // whoever calls this will have to CPLFree() this
  poHDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * dsn.size()));
  for (int i = 0; i < dsn.size(); i++) poHDS[i] = gdalH_open_avrt(dsn[i],  extent, projection, sds, bands);
  return poHDS;
}
// convert an opened GDALDataset to chunk-of-text VRT, if it is VRT you get it direct
//  if it's a different driver it is firs CreateCopy()ied to VRT
inline const char* gdal_vrt_text(GDALDataset* poSrcDS) {
  CharacterVector out(1);
  // can't do this unless poSrcDS really is VRT
  if (EQUAL(poSrcDS->GetDriverName(),  "VRT")) {
    VRTDataset * VRTdcDS = cpl::down_cast<VRTDataset *>(poSrcDS );
    // if (add_filename[0]) {
    //   //VRTdcDS->SetDescription( filename[0]);
    // }
    if (!(VRTdcDS == nullptr)) out[0] = VRTdcDS->GetMetadata("xml:VRT")[0];
  } else {
    GDALDriver *poDriver = (GDALDriver *)GDALGetDriverByName("VRT");
    GDALDataset *VRTDS = poDriver->CreateCopy("", poSrcDS, false, NULL, NULL, NULL);
    // if (add_filename[0]) {
    //   //VRTDS->SetDescription( filename[0]);
    // 
    // }
    if (!(VRTDS == nullptr)) out[0] = VRTDS->GetMetadata("xml:VRT")[0];
    GDALClose((GDALDatasetH) VRTDS);
  }
  return out[0];
}

// open any DSN/ss (files,urls,sdsstrings,etc.) to chunk-of-VRT text vector
// optionally with extent(=a_ullr) and/or projection(=a_srs) augmented
// [[Rcpp::export]]
inline CharacterVector gdal_dsn_vrt(CharacterVector dsn, NumericVector extent, CharacterVector projection, IntegerVector sds, IntegerVector bands) {
  CharacterVector out(dsn.size());
  GDALDatasetH DS;
  for (int i = 0; i < out.size(); i++) {
    if (extent.size() == 4 || (!projection[0].empty()) || bands[0] > 0) {
      DS = gdalH_open_avrt(dsn[0], extent, projection, sds, bands);
    } else {
      DS = gdalH_open_dsn(dsn[0], sds);
    }
    out[i] = gdal_vrt_text((GDALDataset*) DS);
    GDALClose(DS);
  }
  return out;
}


// -------------------------------------------------------------------
// these were the original workers in the gdalraster namespace, these pre-date
// CRAN version 0.8.5 and so have not been released, but vapour_sds_names 
// and similar already use so needs checking

// has_subdataset(), list_subdatasets(), gdal_open_dsn(), gdal_dataset_augment()
// these were not modular enough and were causing problems



} //  namespace gdalraster
#endif
