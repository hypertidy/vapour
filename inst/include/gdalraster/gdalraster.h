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
   // Rprintf("%i\n", sdi/2);
  //  Rprintf("%i\n\n", i_sds - 1);
    if (sdi / 2 == (i_sds -1 )) {
      char  **papszTokens = CSLTokenizeString2(SDS2[sdi ], "=", 0);
      ret[0] = papszTokens[1];
      CSLDestroy( papszTokens );
      break;
    }
    // ii*2 because SDS tokens come in pairs
      sdi = sdi + 1 + 1;
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

  Rcpp::CharacterVector ret(sdi/2);
  int dscount = static_cast<int>(ret.size());
  int cnt = 0;
  if (dscount > 0) {
    // we have subdatasets, so list them all
    // owned by the object
    char **SDS2 = GDALGetMetadata(poDataset, "SUBDATASETS");
    for (int ii = 0; ii <  sdi; ii++) {
      // SDS tokens come in pairs
      if (ii % 2 == 0) {
        ret[cnt] = SDS2[ii];
       cnt++;
      }
    }
  }
  return ret;
}

// open the DSN, open a subdataset if sds > 0 (else you get the outer shell)
inline GDALDatasetH gdalH_open_dsn(const char * dsn, IntegerVector sds) {
  GDALDatasetH DS; 
  DS = GDALOpen(dsn, GA_ReadOnly);
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
inline GDALDatasetH gdalH_open_avrt(const char* dsn, 
                                    NumericVector extent, 
                                    CharacterVector projection, 
                                    IntegerVector sds, IntegerVector bands, CharacterVector geolocation) {
  
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
  
  GDALDataset* oDS = (GDALDataset*)gdalH_open_dsn(dsn, sds);

  
  if (geolocation.size() == 2) {
      // OGRSpatialReference* geolsrs = nullptr;
      // geolsrs = new OGRSpatialReference;
      // OGRErr chk = geolsrs->SetFromUserInput("OGC:CRS84"); 
      oDS->SetMetadataItem( "SRS", "OGC:CRS84", "GEOLOCATION" ); 
      oDS->SetMetadataItem( "X_DATASET", geolocation[0], "GEOLOCATION" );
        oDS->SetMetadataItem( "X_BAND", "1" , "GEOLOCATION" );
        oDS->SetMetadataItem( "Y_DATASET", geolocation[1], "GEOLOCATION" );
        oDS->SetMetadataItem( "Y_BAND", "1" , "GEOLOCATION" );


        oDS->SetMetadataItem( "PIXEL_OFFSET", "0", "GEOLOCATION" );
        oDS->SetMetadataItem( "PIXEL_STEP", "1", "GEOLOCATION" );
        oDS->SetMetadataItem( "LINE_OFFSET", "0", "GEOLOCATION" );
        oDS->SetMetadataItem( "LINE_STEP", "1", "GEOLOCATION" );
  }
  
  
  
  if (oDS == nullptr) return(nullptr);
  int nBands = oDS->GetRasterCount();
  // Rprintf("%i\n", nBands);
  //  if (bands[0] > 0) {
  //    for (int iband = 0; iband < bands.size(); iband++ ) {
  //      if (bands[iband] > nBands) {
  //        Rprintf("%i\n", bands[iband]);
  //        Rprintf("mismatch bands\n");
  //      }
  //      }
  //  }
  if (bands[0] > 0) {
    for (int iband = 0; iband < bands.size(); iband++ ) {
      if (bands[iband] > nBands) {
        // FIXME: here consider just dropping bands that arne't available, that's what gdal_translate does
        return nullptr;
      } else {
        translate_argv.AddString("-b");
        translate_argv.AddString(CPLSPrintf("%i", bands[iband]));
      }
    }
  }

  
  GDALTranslateOptions* psTransOptions = GDALTranslateOptionsNew(translate_argv.List(), nullptr);

  GDALDatasetH a_DS = GDALTranslate("", oDS, psTransOptions, nullptr);
  GDALTranslateOptionsFree( psTransOptions );
  return a_DS;
}

// open the DSN/s with gdalH_open_dsn(), possibly with a sds (1-based)
// (nothing uses this function)
inline GDALDatasetH* gdalH_open_multiple(CharacterVector dsn, IntegerVector sds) {
  
  GDALDatasetH* poHDS;
  // whoever calls this will have to CPLFree() this
  poHDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * static_cast<size_t>(dsn.size())));
  for (int i = 0; i < dsn.size(); i++) poHDS[i] = gdalH_open_dsn(dsn[i], sds);
  return poHDS;
}
inline GDALDatasetH* gdalH_open_avrt_multiple(CharacterVector dsn, NumericVector extent, 
                                              CharacterVector projection, IntegerVector sds, IntegerVector bands) {
  
  GDALDatasetH* poHDS;
  // whoever calls this will have to CPLFree() this
  poHDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * static_cast<size_t>(dsn.size())));
  for (int i = 0; i < dsn.size(); i++) poHDS[i] = gdalH_open_avrt(dsn[i],  extent, projection, sds, bands, "");
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
inline CharacterVector gdal_dsn_vrt(CharacterVector dsn, NumericVector extent, CharacterVector projection, 
                                    IntegerVector sds, IntegerVector bands, 
                                    CharacterVector geolocation) {
  CharacterVector out(dsn.size());
  GDALDatasetH DS;
  for (int i = 0; i < out.size(); i++) {
    if (extent.size() == 4 || (!projection[0].empty()) || bands[0] > 0) {
      DS = gdalH_open_avrt(dsn[i], extent, projection, sds, bands, geolocation);
      
    } else {
      DS = gdalH_open_dsn(dsn[i], sds);
    }
    
    if (DS == nullptr) {
     out[i] = NA_STRING;  
    } else {
      out[i] = gdal_vrt_text((GDALDataset*) DS);
      GDALClose(DS);
    }
  }
  return out;
}




// move from gdallibrary:: 2022-03-05
inline CharacterVector gdal_sds_list(const char* pszFilename)
{
  GDALDataset  *poDataset;
  
  poDataset = (GDALDataset *) GDALOpen( pszFilename, GA_ReadOnly );  // gdal_sds_list()
  if( poDataset == NULL )
  {
    Rcpp::stop("cannot open dataset");
  }
  
  Rcpp::CharacterVector ret; 
  if (gdalraster::gdal_has_subdataset(poDataset)) {
    ret = gdalraster::gdal_list_subdatasets(poDataset);
  } else {
    CharacterVector a(1);
    a[0] = pszFilename;
    ret = a;
  }
  GDALClose( (GDALDatasetH) poDataset );
  return ret;
}


// move from gdallibrary:: 2022-03-05
inline NumericVector gdal_extent_only(CharacterVector dsn) {
  GDALDatasetH hDataset;
  
  hDataset = GDALOpen(dsn[0], GA_ReadOnly); //gdal_extent_only()
  
  if( hDataset == nullptr )
  {
    Rcpp::stop("cannot open dataset");
  }
  
  double        adfGeoTransform[6];
  //poDataset->GetGeoTransform( adfGeoTransform );
  GDALGetGeoTransform(hDataset, adfGeoTransform );
  int nXSize = GDALGetRasterXSize(hDataset);
  int nYSize = GDALGetRasterYSize(hDataset);
  
  GDALClose( hDataset );
  NumericVector extent(4); 
  extent[0] = adfGeoTransform[0];
  extent[1] = adfGeoTransform[0] + nXSize * adfGeoTransform[1];
  extent[3] = adfGeoTransform[3]; 
  extent[2] = adfGeoTransform[3] + nYSize * adfGeoTransform[5]; 
  return extent; 
}

// moved from gdallibrary:: 2022-03-05
inline List gdal_raster_info(CharacterVector dsn, LogicalVector min_max)
  {
    GDALDatasetH hDataset;
    
    hDataset = GDALOpen(dsn[0], GA_ReadOnly); // gdal_raster_info()
    
    if( hDataset == nullptr )
    {
      Rcpp::stop("cannot open dataset");
    }
    int nXSize = GDALGetRasterXSize(hDataset);
    int nYSize = GDALGetRasterYSize(hDataset);
    
    
    double        adfGeoTransform[6];
    
    //poDataset->GetGeoTransform( adfGeoTransform );
    GDALGetGeoTransform(hDataset, adfGeoTransform );
    
    
    // bail out NOW (we have no SDS and/or no rasters)
    // #f <- system.file("h5ex_t_enum.h5", package = "h5")
    if (GDALGetRasterCount(hDataset) < 1) {
      Rcpp::stop("no rasters found in dataset");
    }
    
    Rcpp::DoubleVector trans(6);
    for (int ii = 0; ii < 6; ii++) trans[ii] = adfGeoTransform[ii];
    
    char **pfilelist = GDALGetFileList(hDataset);
    int fdi = 0;
    while (pfilelist && pfilelist[fdi] != NULL) {
      fdi++; // count
    }
    int ilist = fdi;
    if (ilist < 1) {
      ilist = 1; 
    }
    CharacterVector FileList(ilist);
    // might be no files, because image server
    if (pfilelist == nullptr) {
      FileList[0] = NA_STRING;
    } else {
      for (int ifile = 0; ifile < fdi; ifile++) {
        FileList[ifile] = pfilelist[ifile]; 
      }
    }
    CSLDestroy(pfilelist);
    GDALRasterBandH  hBand;
    int             nBlockXSize, nBlockYSize;
    //int             bGotMin, bGotMax;
    double          adfMinMax[2];
    
    hBand = GDALGetRasterBand(hDataset, 1);
    // if we don't bail out above with no rasters things go bad here
    GDALGetBlockSize(hBand, &nBlockXSize, &nBlockYSize);
    if (min_max[0]) {
      GDALComputeRasterMinMax(hBand, TRUE, adfMinMax);
    }
    
    int nn = 11;
    Rcpp::List out(nn);
    Rcpp::CharacterVector names(nn);
    out[0] = trans;
    names[0] = "geotransform";
    out[1] = Rcpp::IntegerVector::create(nXSize, nYSize);
    names[1] = "dimXY";
    //GDALGetMetadataDomainList(hBand,  )
    
    
    
    
    DoubleVector vmmx(2);
    if (min_max[0]) {
      vmmx[0] = adfMinMax[0];
      vmmx[1] = adfMinMax[1];
    } else {
      vmmx[0] = NA_REAL;
      vmmx[1] = NA_REAL;
    }
    out[2] = vmmx;
    names[2] = "minmax";
    
    out[3] = Rcpp::IntegerVector::create(nBlockXSize, nBlockYSize);
    names[3] = "tilesXY";
    
    const char *proj;
    proj = GDALGetProjectionRef(hDataset);
    //https://gis.stackexchange.com/questions/164279/how-do-i-create-ogrspatialreference-from-raster-files-georeference-c
    out[4] = Rcpp::CharacterVector::create(proj);
    names[4] = "projection";
    
    // get band number
    int nBands = GDALGetRasterCount(hDataset);
    out[5] = nBands;
    names[5] = "bands";
    
    char *stri;
    OGRSpatialReference oSRS;
    const char *proj2;
    proj2 = GDALGetProjectionRef(hDataset);
    char **cwkt = (char **) &proj2;
    
#if GDAL_VERSION_MAJOR <= 2 && GDAL_VERSION_MINOR <= 2
    oSRS.importFromWkt(cwkt);
#else
    oSRS.importFromWkt( (const char**) cwkt);
#endif
    oSRS.exportToProj4(&stri);
    out[6] =  Rcpp::CharacterVector::create(stri); //Rcpp::CharacterVector::create(stri);
    names[6] = "projstring";
    CPLFree(stri);
    
    int succ;
    out[7] = GDALGetRasterNoDataValue(hBand, &succ);
    names[7] = "nodata_value";
    
    int ocount = GDALGetOverviewCount(hBand);
    // f <- system.file("extdata/volcano_overview.tif", package = "vapour", mustWork = TRUE)
    IntegerVector oviews = IntegerVector(ocount * 2);
    //oviews[0] = ocount;
    if (ocount > 0) {
      GDALRasterBandH  oBand;
      for (int ii = 0; ii < ocount; ii++) {
        oBand = GDALGetOverview(hBand, ii);
        
        
        int xsize = GDALGetRasterBandXSize(oBand);
        int ysize = GDALGetRasterBandYSize(oBand);
        
        oviews[(ii * 2) + 0 ] = xsize;
        oviews[(ii * 2) + 1 ] = ysize;
        
        
        
      }
    }
    out[8] = oviews;
    names[8] = "overviews";
    
    out[9] = FileList;
    names[9] = "filelist";
    
    
    out[10] = CharacterVector::create(GDALGetDataTypeName(GDALGetRasterDataType(hBand))); 
    names[10] = "datatype"; 
    
    out.attr("names") = names;
    
    // close up
    GDALClose( hDataset );
    return out;
    
  }

// moved from gdallibrary:: 2022-03-05
inline List gdal_raster_gcp(CharacterVector dsn) {
  // get GCPs if any
  GDALDatasetH hDataset;
  //GDALDataset  *poDataset;
  
  hDataset = GDALOpen( dsn[0], GA_ReadOnly); // gdal_raster_gcp()
  if( hDataset == nullptr )
  {
    Rcpp::stop("cannot open dataset");
  }
  
  int gcp_count;
  gcp_count = GDALGetGCPCount(hDataset);
  const char *srcWKT = GDALGetGCPProjection(hDataset);
  Rcpp::List gcpout(6);
  Rcpp::CharacterVector gcpnames(6);
  Rcpp::CharacterVector gcpCRS(1);
  gcpCRS[0] = srcWKT;
  gcpnames[0] = "Pixel";
  gcpnames[1] = "Line";
  gcpnames[2] = "X";
  gcpnames[3] = "Y";
  gcpnames[4] = "Z";
  gcpnames[5] = "CRS";
  gcpout.attr("names") = gcpnames;
  if (gcp_count > 0) {
    Rcpp::NumericVector GCPPixel(gcp_count);
    Rcpp::NumericVector GCPLine(gcp_count);
    Rcpp::NumericVector GCPX(gcp_count);
    Rcpp::NumericVector GCPY(gcp_count);
    Rcpp::NumericVector GCPZ(gcp_count);
    for (int igcp = 0; igcp < gcp_count; ++igcp) {
      const GDAL_GCP *gcp = GDALGetGCPs( hDataset ) + igcp;
      //const GDAL_GCP *gcp = poDataset->GetGCPs() + igcp;
      GCPPixel[igcp] = gcp->dfGCPPixel;
      GCPLine[igcp] = gcp->dfGCPLine;
      GCPX[igcp] = gcp->dfGCPX;
      GCPY[igcp] = gcp->dfGCPY;
      GCPZ[igcp] = gcp->dfGCPZ;
    }
    gcpout[0] = GCPPixel;
    gcpout[1] = GCPLine;
    gcpout[2] = GCPX;
    gcpout[3] = GCPY;
    gcpout[4] = GCPZ;
    gcpout[5] = gcpCRS;
    //gcp_proj = poDataset->GetGCPProjection();
  } else {
    Rprintf("No GCP (ground control points) found.\n");
  }
  GDALClose( hDataset );
  return gcpout;
}

// moved from gdallibrary:: 2022-03-05
inline GDALRasterIOExtraArg init_resample_alg(CharacterVector resample) {
  GDALRasterIOExtraArg psExtraArg;
  INIT_RASTERIO_EXTRA_ARG(psExtraArg);
  if (resample[0] == "average") {
    psExtraArg.eResampleAlg = GRIORA_Average;
  }
  if (resample[0] == "bilinear") {
    psExtraArg.eResampleAlg = GRIORA_Bilinear;
  }
  if (resample[0] == "cubic") {
    psExtraArg.eResampleAlg = GRIORA_Cubic;
  }
  if (resample[0] == "cubicspline") {
    psExtraArg.eResampleAlg = GRIORA_CubicSpline;
  }
  if (resample[0] == "gauss") {
    psExtraArg.eResampleAlg = GRIORA_Gauss;
  }
  if (resample[0] == "lanczos") {
    psExtraArg.eResampleAlg = GRIORA_Lanczos;
  }
  if (resample[0] == "mode") {
    psExtraArg.eResampleAlg = GRIORA_Mode;
  }
  if (resample[0] == "nearestneighbour") {
    psExtraArg.eResampleAlg = GRIORA_NearestNeighbour;
  }
  return psExtraArg;
}

// moved from gdallibrary:: 2022-03-05
inline List gdal_read_band_values(GDALDataset *hRet, 
                                  IntegerVector window,
                                  std::vector<int> bands_to_read, 
                                  CharacterVector band_output_type, 
                                  CharacterVector resample,
                                  LogicalVector unscale) 
{
  int Xoffset = window[0];
  int Yoffset = window[1];
  int nXSize = window[2];
  int nYSize = window[3];
  
  int outXSize = window[4];
  int outYSize = window[5];
  int actual_XSize = -1; //GDALGetRasterBandXSize(hRet);
  int actual_YSize = -1; //GDALGetRasterBandYSize(hRet);
  
  
  // double scale, double offset, 
  // int hasScale, int hasOffset, int hasNA,
  // GDALDataType src_band_type) {
  // if band_output_type is not empty, possible override:
  // complex types not supported Byte, UInt16, Int16, UInt32, Int32, Float32, Float64
  GDALDataType src_band_type =  GDALGetRasterDataType(GDALGetRasterBand(hRet, bands_to_read[0])); 
  //bool output_type_set = false; 
  
  if (!band_output_type[0].empty()) {
    if (band_output_type[0] == "Byte")   src_band_type = GDT_Byte;
    if (band_output_type[0] == "UInt16") src_band_type = GDT_UInt16;
    if (band_output_type[0] == "Int16")  src_band_type = GDT_Int16;
    
    if (band_output_type[0] == "UInt32") src_band_type = GDT_UInt32;
    if (band_output_type[0] == "Int32") src_band_type = GDT_Int32;
    
    if (band_output_type[0] == "Float32") src_band_type = GDT_Float32;
    if (band_output_type[0] == "Float64") src_band_type = GDT_Float64;
    
    //output_type_set = true;
  }
  
  int sbands = (int)bands_to_read.size();
  Rcpp::List outlist(bands_to_read.size());
  
  bool band_type_not_supported = true;
  
  GDALRasterBand *rasterBand; 
  int hasNA;
  int hasScale, hasOffset;
  double scale, offset;

  GDALRasterIOExtraArg psExtraArg;
  psExtraArg = init_resample_alg(resample);
  CPLErr err;
  
  for (int iband = 0; iband < sbands; iband++) {
    rasterBand = GDALDataset::FromHandle(hRet)->GetRasterBand(bands_to_read[static_cast<size_t>(iband)]);
    //rasterBand = GDALGetRasterBand(hRet, bands_to_read[iband]);
    if (iband < 1) {
      // actual_XSize = GDALGetRasterBandXSize(rasterBand); 
      // actual_YSize = GDALGetRasterBandYSize(rasterBand); 
      actual_XSize = rasterBand->GetXSize();
      actual_YSize = rasterBand->GetYSize();
      
      if (nXSize < 1) nXSize = actual_XSize;
      if (nYSize < 1) nYSize = actual_YSize;
      if (outXSize < 1) outXSize = actual_XSize;
      if (outYSize < 1) outYSize = actual_YSize;
      
      
    }
    
    scale = rasterBand->GetScale(&hasScale);
    offset = rasterBand->GetOffset(&hasOffset);
    
    // if scale is 1 or 0 then don't override the type
    if (abs(scale - 1.0) <= 1.0e-05 || abs(scale) < 1.0e-05) {
      hasScale = 0; 
    }
    // if hasScale we ignore integer or byte and go with float
    if ((src_band_type == GDT_Float64) || (src_band_type == GDT_Float32) || hasScale) {
      std::vector<double> double_scanline( static_cast<size_t>( outXSize * outYSize ));
      err = rasterBand->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                                 &double_scanline[0], outXSize, outYSize, GDT_Float64,
                                 0, 0, &psExtraArg);
      if (err) Rprintf("we have a problem at RasterIO\n");
      NumericVector res(outXSize * outYSize  );
      
      // consider doing at R level, at least for MEM
      double dval;
      double naflag = rasterBand->GetNoDataValue(&hasNA);// GDALGetRasterNoDataValue(rasterBand, &hasNA);
      if (hasNA && (!std::isnan(naflag))) {
        if (naflag < -3.4e+37) {
          naflag = -3.4e+37;
          
          for (size_t i=0; i< double_scanline.size(); i++) {
            if (double_scanline[i] <= naflag) {
              double_scanline[i] = NA_REAL; 
            }
          }
        } else {
          
          std::replace(double_scanline.begin(), double_scanline.end(), naflag, (double) NAN);
        }
      }
      R_xlen_t isi;
      
      for (isi = 0; isi < (static_cast<R_xlen_t>(double_scanline.size())); isi++) {
        dval = double_scanline[static_cast<size_t>(isi)];
        if (hasScale) dval = dval * scale;
        if (hasOffset) dval = dval + offset;
        res[isi] = dval;
      }
      
      outlist[iband] = res;
      band_type_not_supported = false;
    }
    // if hasScale we assume to never use scale/offset in integer case (see block above we already dealt)
    if ((!hasScale) & 
        ((src_band_type == GDT_Int16) |
        (src_band_type == GDT_Int32) |
        (src_band_type == GDT_UInt16) |
        (src_band_type == GDT_UInt32)))  {
      std::vector<int32_t> integer_scanline(static_cast<size_t>( outXSize * outYSize ));
      err = rasterBand->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                                 &integer_scanline[0], outXSize, outYSize, GDT_Int32,
                                 0, 0, &psExtraArg);
      
      if (err) Rprintf("we have a problem at RasterIO\n");
      IntegerVector res(outXSize * outYSize );
      
      // consider doing at R level, at least for MEM
      int dval;
      double naflag = GDALGetRasterNoDataValue(rasterBand, &hasNA);
   
      R_xlen_t isi;
      for (isi = 0; isi < (static_cast<R_xlen_t>(integer_scanline.size())); isi++) {
        dval = integer_scanline[static_cast<size_t>(isi)];
        if (static_cast<double>(dval) <= naflag) {
          dval = NA_INTEGER;
        }
        res[isi] = dval;
      }
      outlist[iband] = res;
      band_type_not_supported = false;
    }
    
    // if hasScale we assume to never use scale/offset in integer case (see block above we already dealt)
    
    if (!hasScale & (src_band_type == GDT_Byte)) {
      std::vector<uint8_t> byte_scanline( static_cast<size_t>( outXSize * outYSize ) );
      err = rasterBand->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                                 &byte_scanline[0], outXSize, outYSize, GDT_Byte,
                                 0, 0, &psExtraArg);
      
      if (err) Rprintf("we have a problem at RasterIO\n");
      RawVector res(outXSize * outYSize );


   
      R_xlen_t isi;
      for (isi = 0; isi < (static_cast<R_xlen_t>(byte_scanline.size())); isi++) {
        res[isi] = byte_scanline[static_cast<size_t>(isi)];
      }
      outlist[iband] = res;
      band_type_not_supported = false;
    }
    
    
    
  }
  // safe but lazy way of not supporting Complex, TypeCount or Unknown types
  // (see GDT_ checks above)
  if (band_type_not_supported) {
    GDALClose(hRet); 
    Rcpp::stop("band type not supported (is it Complex? report at hypertidy/vapour/issues)");
  }
  
  return outlist; 
}




  inline List gdal_read_dataset_values(GDALDataset *hRet, 
                                       IntegerVector window,
                                       std::vector<int> bands_to_read, 
                                       CharacterVector band_output_type, 
                                       CharacterVector resample,
                                       LogicalVector unscale) 
  {
    int Xoffset = window[0];
    int Yoffset = window[1];
    int nXSize = window[2];
    int nYSize = window[3];
    
    int outXSize = window[4];
    int outYSize = window[5];
    
    
    // double scale, double offset, 
    // int hasScale, int hasOffset, int hasNA,
    // GDALDataType src_band_type) {
    // if band_output_type is not empty, possible override:
    // complex types not supported Byte, UInt16, Int16, UInt32, Int32, Float32, Float64
    GDALDataType src_band_type =  GDALGetRasterDataType(GDALGetRasterBand(hRet, bands_to_read[0])); 
    //bool output_type_set = false; 
    
    if (!band_output_type[0].empty()) {
      if (band_output_type[0] == "Byte")   src_band_type = GDT_Byte;
      if (band_output_type[0] == "UInt16") src_band_type = GDT_UInt16;
      if (band_output_type[0] == "Int16")  src_band_type = GDT_Int16;
      
      if (band_output_type[0] == "UInt32") src_band_type = GDT_UInt32;
      if (band_output_type[0] == "Int32") src_band_type = GDT_Int32;
      
      if (band_output_type[0] == "Float32") src_band_type = GDT_Float32;
      if (band_output_type[0] == "Float64") src_band_type = GDT_Float64;
      
      //output_type_set = true;
    }
    
    // int sbands = (int)bands_to_read.size();
    Rcpp::List outlist(1);
    
    bool band_type_not_supported = true;
    
    GDALRasterBand *rasterBand; 
    int hasNA;
    int hasScale, hasOffset;
    double scale, offset;
    int actual_XSize = -1; //GDALGetRasterBandXSize(hRet);
    int actual_YSize = -1; //GDALGetRasterBandYSize(hRet);
    
    GDALRasterIOExtraArg psExtraArg;
    psExtraArg = init_resample_alg(resample);
    CPLErr err;
    
    rasterBand = GDALDataset::FromHandle(hRet)->GetRasterBand(bands_to_read[0]);
    actual_XSize = rasterBand->GetXSize();
    actual_YSize = rasterBand->GetYSize();
    
    if (nXSize < 1) nXSize = actual_XSize;
    if (nYSize < 1) nYSize = actual_YSize;
    if (outXSize < 1) outXSize = actual_XSize;
    if (outYSize < 1) outYSize = actual_YSize;
    
    size_t n_values_out = static_cast<size_t>(outXSize * outYSize) * bands_to_read.size();
    
    scale = rasterBand->GetScale(&hasScale);
    offset = rasterBand->GetOffset(&hasOffset);
    
    // if scale is 1 or 0 then don't override the type
    if (abs(scale - 1.0) <= 1.0e-09 || abs(scale) < 1.0e-09) {
      hasScale = 0; 
    }
    // if hasScale we ignore integer or byte and go with float
    if ((src_band_type == GDT_Float64) || (src_band_type == GDT_Float32) || hasScale) {
      std::vector<double> double_scanline( n_values_out);
      err = ((GDALDataset*)hRet)->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
             &double_scanline[0], outXSize, outYSize, GDT_Float64,
             0, 0, 
             0, 0, 0, &psExtraArg);
      if (err) Rprintf("we have a problem at RasterIO\n");
      NumericVector res(static_cast<R_xlen_t>(n_values_out));
      
      // consider doing at R level, at least for MEM
      double dval;
      double naflag = rasterBand->GetNoDataValue(&hasNA);// GDALGetRasterNoDataValue(rasterBand, &hasNA);
      if (hasNA && (!std::isnan(naflag))) {
        if (naflag < -3.4e+37) {
          naflag = -3.4e+37;
          
          for (size_t i=0; i< double_scanline.size(); i++) {
            if (double_scanline[i] <= naflag) {
              double_scanline[i] = NA_REAL; 
            }
          }
        } else {
          
          std::replace(double_scanline.begin(), double_scanline.end(), naflag, (double) NAN);
        }
      }
      R_xlen_t isi;
      for (isi = 0; isi < (static_cast<R_xlen_t>(double_scanline.size())); isi++) {
        dval = double_scanline[static_cast<size_t>(isi)];
        if (hasScale) dval = dval * scale;
        if (hasOffset) dval = dval + offset;
        res[isi] = dval;
      }
      outlist[0] = res;
      band_type_not_supported = false;
    }
    // if hasScale we assume to never use scale/offset in integer case (see block above we already dealt)
    if ((!hasScale) & 
        ((src_band_type == GDT_Int16) |
        (src_band_type == GDT_Int32) |
        (src_band_type == GDT_UInt16) |
        (src_band_type == GDT_UInt32)))  {
      std::vector<int32_t> integer_scanline( n_values_out);
      err = ((GDALDataset*)hRet)->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
             &integer_scanline[0], outXSize, outYSize, GDT_Int32,
             0, 0, 
             0, 0, 0, &psExtraArg);
      
      if (err) Rprintf("we have a problem at RasterIO\n");
      IntegerVector res(static_cast<R_xlen_t>(n_values_out) );
      
      // consider doing at R level, at least for MEM
      int dval;
      double naflag = GDALGetRasterNoDataValue(rasterBand, &hasNA);
      
      if (hasNA ) {
        std::replace(integer_scanline.begin(), integer_scanline.end(), (int) naflag, (int) NAN);
        
      }
      R_xlen_t isi;
      for (isi = 0; isi < (static_cast<R_xlen_t>(integer_scanline.size())); isi++) {
        dval = integer_scanline[static_cast<size_t>(isi)];
        // if (hasScale) dval = dval * scale;
        // if (hasOffset) dval = dval + offset;
        res[isi] = dval;
      }
      outlist[0] = res;
      band_type_not_supported = false;
    }
    
    // if hasScale we assume to never use scale/offset in integer case (see block above we already dealt)
    
    if (!hasScale & (src_band_type == GDT_Byte)) {
      std::vector<uint8_t> byte_scanline( n_values_out );
      err = ((GDALDataset *)hRet)->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
             &byte_scanline[0], outXSize, outYSize, GDT_Byte,
             0, 0, 
             0, 0, 0, 
             &psExtraArg);
      
      if (err) Rprintf("we have a problem at RasterIO\n");
      RawVector res(static_cast<R_xlen_t>(n_values_out));
      double naflag = GDALGetRasterNoDataValue(rasterBand, &hasNA);
      
      if (hasNA ) {
        std::replace(byte_scanline.begin(), byte_scanline.end(), (uint8_t)naflag, (uint8_t) NAN);
        
      }
      R_xlen_t isi;
      for (isi = 0; isi < (static_cast<R_xlen_t>(byte_scanline.size())); isi++) {
        res[isi] = byte_scanline[static_cast<size_t>(isi)];
      }
      outlist[0] = res;
      band_type_not_supported = false;
    }
    
    // safe but lazy way of not supporting Complex, TypeCount or Unknown types
    // (see GDT_ checks above)
    if (band_type_not_supported) {
      GDALClose(hRet); 
      Rcpp::stop("band type not supported (is it Complex? report at hypertidy/vapour/issues)");
    }
    
    return outlist; 
  }


inline List gdal_raster_dataset_io(CharacterVector dsn,
                                   IntegerVector window,
                                   IntegerVector band,
                                   CharacterVector resample,
                                   CharacterVector band_output_type)
{
  
  GDALDataset  *poDataset;
  poDataset = (GDALDataset *) GDALOpen(dsn[0], GA_ReadOnly ); // gdal_raster_dataset_io
  if( poDataset == NULL )
  {
    Rcpp::stop("cannot open dataset");
  }
  if (band[0] < 1) { GDALClose(poDataset);  Rcpp::stop("requested band %i should be 1 or greater", band[0]);  }
  int nBands = poDataset->GetRasterCount(); 
    
    if (band[0] > nBands) { GDALClose(poDataset);   Rcpp::stop("requested band %i should be equal to or less than number of bands: %l", band[0], nBands); }
  
  std::vector<int> bands_to_read(static_cast<size_t>(band.size()));
  if (band.size() == 1 && band[0] == 0) {
    for (int i = 0; i < nBands; i++) bands_to_read[static_cast<size_t>(i)] = i + 1;
  } else {
    for (int i = 0; i < band.size(); i++) bands_to_read[static_cast<size_t>(i)] = band[i];
  }
  List out = gdal_read_dataset_values(poDataset, window, bands_to_read, band_output_type, resample, false);
  // close up
  GDALClose(poDataset );
  return out;
}

inline List gdal_raster_io(CharacterVector dsn,
                           IntegerVector window,
                           IntegerVector band,
                           CharacterVector resample,
                           CharacterVector band_output_type)
{
  
  GDALDataset  *poDataset;
  poDataset = (GDALDataset *) GDALOpen(dsn[0], GA_ReadOnly );  // gdal_raster_io()
  if( poDataset == NULL )
  {
    Rcpp::stop("cannot open dataset");
  }
  if (band[0] < 1) { GDALClose(poDataset);  Rcpp::stop("requested band %i should be 1 or greater", band[0]);  }
  int nBands = poDataset->GetRasterCount();
  if (band[0] > nBands) { GDALClose(poDataset);   Rcpp::stop("requested band %i should be equal to or less than number of bands: %i", band[0], nBands); }

  std::vector<int> bands_to_read(static_cast<size_t>(band.size()));
  if (band.size() == 1 && band[0] == 0) {
    for (int i = 0; i < nBands; i++) bands_to_read[static_cast<size_t>(i)] = i + 1;
  } else {
    for (int i = 0; i < band.size(); i++) bands_to_read[static_cast<size_t>(i)] = band[i];
  }
  List out = gdal_read_band_values(poDataset, window, bands_to_read, band_output_type, resample, false);
  // close up
  GDALClose(poDataset );
  return out;
}



// does it have geolocation arrays?
inline LogicalVector gdal_has_geolocation(CharacterVector dsn, IntegerVector sds) {
  
  GDALDataset* poDataset;
  poDataset = (GDALDataset*)gdalH_open_dsn(dsn[0], sds);
  
  bool has_geol = false;
  char **papszGeolocationInfo = poDataset->GetMetadata("GEOLOCATION");
  if( papszGeolocationInfo != nullptr ) {
    has_geol = true;
  }
  GDALClose(poDataset);
  LogicalVector out(1);
  out[0] = has_geol;
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
