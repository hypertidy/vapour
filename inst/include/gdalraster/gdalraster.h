#ifndef GDALRASTER_H
#define GDALRASTER_H
#include <Rcpp.h>
#include "gdal_priv.h"
#include "gdal_utils.h"  // for GDALTranslateOptions
#include "vrtdataset.h"

namespace gdalraster {
using namespace Rcpp;


inline LogicalVector has_subdataset(GDALDataset *poDataset) {
  // not faster, we good
  //if (poDataset->GetRasterCount() > 0) return false;
  char **MDdomain = GDALGetMetadataDomainList(poDataset);
  int mdi = 0; // iterate though MetadataDomainList
  bool has_sds = false;
  while (MDdomain && MDdomain[mdi] != NULL) {
    if (strcmp(MDdomain[mdi], "SUBDATASETS") == 0) {
      has_sds = true;
    }
    mdi++;
  }
  //cleanup
  CSLDestroy(MDdomain);
  LogicalVector out(1);
  out[0] = has_sds;
  return out;
}
inline CharacterVector list_subdatasets(GDALDataset *poDataset) {
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

inline GDALDataset *gdal_open_dsn(CharacterVector dsn, IntegerVector sds) {
  GDALAllRegister();
  auto DS = GDALDataset::Open(dsn[0], GDAL_OF_RASTER | GDAL_OF_SHARED, nullptr, nullptr, nullptr);
  if( DS == nullptr ) {
    return nullptr;
  }
  int isds = sds[0];
  if (// is it worth this extra pre-checking (not much faster, no, we on par with vapour_raster_info about 1/20 second)
      // (DS->GetRasterXSize() == 512 && DS->GetRasterYSize() == 512 && DS->GetRasterCount() < 1 && default_geotransform(DS)) ||
      has_subdataset(DS)) {
    CharacterVector sdsnames = list_subdatasets(DS);
    if (sdsnames.length() > 0 && !sdsnames[0].empty()) {
      // user asked for 1-based SDS, or zero was default
      if (isds < 1) {
        isds = 1;
      }
      if (isds > (sdsnames.length())) {
        return nullptr;
      }
      DS->ReleaseRef();
      DS = GDALDataset::Open(sdsnames[isds - 1], GDAL_OF_RASTER | GDAL_OF_SHARED, nullptr, nullptr, nullptr);

    }
  }
  return GDALDataset::FromHandle( DS);
}


inline GDALDataset *gdal_dataset_augment(CharacterVector dsn,
                                  NumericVector extent,
                                  CharacterVector projection,
                                  IntegerVector sds) {
  auto poSrcDS = gdal_open_dsn(dsn, sds);
  if( poSrcDS == nullptr )
   {
     return nullptr;
  }
  CPLStringList argv;
  argv.AddString("-of");
  argv.AddString("VRT");
  bool set_extent = extent.size() == 4;
  bool set_projection = !projection[0].empty();

  if (set_extent) {
    if ((extent[1] <= extent[0]) || extent[3] <= extent[2]) {
      poSrcDS->ReleaseRef();
      Rprintf("extent must be valid c(xmin, xmax, ymin, ymax)\n");
      return nullptr;
    }
    argv.AddString("-a_ullr");
    argv.AddString(CPLSPrintf("%f", extent[0]));
    argv.AddString(CPLSPrintf("%f", extent[3]));
    argv.AddString(CPLSPrintf("%f", extent[1]));
    argv.AddString(CPLSPrintf("%f", extent[2]));
  }
  if (set_projection) {
     argv.AddString("-a_srs");
     argv.AddString(projection[0]);
  }
Rprintf("after argv: %s\n", projection[0]);
    
  GDALTranslateOptions* psOptions = GDALTranslateOptionsNew(argv.List(), nullptr);
  auto hRet = GDALTranslate("", GDALDataset::ToHandle(poSrcDS),
                            psOptions, nullptr);
  GDALTranslateOptionsFree( psOptions );
  poSrcDS->ReleaseRef();

  return GDALDataset::FromHandle(hRet);
}


inline CharacterVector open_as_vrt(CharacterVector dsn, NumericVector extent, CharacterVector projection, IntegerVector sds) {
  CharacterVector out(1);

  GDALDataset *poDATASET = gdal_dataset_augment(dsn, extent, projection, sds);

  auto poDS = cpl::down_cast<VRTDataset*>(GDALDataset::FromHandle(poDATASET));
  if( poDS )
  {
    poDS->SetDescription(CPLSPrintf("%s", (char *)dsn[0]));
    poDS->SetWritable(false);
  }
  if (poDS) {
    const char *xmlvrt = poDS->GetMetadata("xml:VRT")[0];

    out[0] = xmlvrt;
    poDS->ReleaseRef();
  }
  return out;
}

} //  namespace gdalraster
#endif
