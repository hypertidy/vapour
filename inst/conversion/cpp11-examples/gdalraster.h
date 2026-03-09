#ifndef GDALRASTER_H
#define GDALRASTER_H
#include <cpp11.hpp>
#include "gdal_priv.h"
#include "gdal_utils.h"  // for GDALTranslateOptions
#include "vrtdataset.h"
#include <ogr_spatialref.h>

#include <Rinternals.h>

#define R_GRAY(v)  ((v) * 0x00010101 | 0xFF000000)
#define R_RGB(r,g,b)	  ((r)|((g)<<8)|((b)<<16)|0xFF000000)
#define R_RGBA(r,g,b,a)	((r)|((g)<<8)|((b)<<16)|((a)<<24))


namespace gdalraster {
using namespace cpp11;
namespace writable = cpp11::writable;

// --- nativeRaster helpers: pure R C API, unchanged ---
inline SEXP C_native_gray(SEXP b0, SEXP dm) {
  R_xlen_t n = Rf_xlength(b0);
  SEXP res_ = PROTECT(Rf_allocVector(INTSXP, n));
  int *pres = INTEGER(res_);
  Rbyte *p0 = RAW(b0);
  for (R_xlen_t i = 0; i < n; i++) {
    Rbyte v = p0[i];
    pres[i] = (int)R_GRAY(v);
  }
  SEXP dim = PROTECT(Rf_allocVector(INTSXP, 2));
  INTEGER(dim)[0] = INTEGER(dm)[1];
  INTEGER(dim)[1] = INTEGER(dm)[0];
  Rf_setAttrib(res_, R_DimSymbol, dim);
  SEXP cls = PROTECT(Rf_mkString("nativeRaster"));
  Rf_setAttrib(res_, R_ClassSymbol, cls);
  Rf_setAttrib(res_, Rf_install("channels"), PROTECT(Rf_ScalarInteger(3)));
  UNPROTECT(4);
  return res_;
}

inline SEXP C_native_rgb(SEXP b0, SEXP b1, SEXP b2, SEXP dm) {
  R_xlen_t n = Rf_xlength(b0);
  SEXP res_ = PROTECT(Rf_allocVector(INTSXP, n));
  int *pres = INTEGER(res_);
  Rbyte *p0 = RAW(b0), *p1 = RAW(b1), *p2 = RAW(b2);
  for (R_xlen_t i = 0; i < n; i++) {
    pres[i] = (int)R_RGB(p0[i], p1[i], p2[i]);
  }
  SEXP dim = PROTECT(Rf_allocVector(INTSXP, 2));
  INTEGER(dim)[0] = INTEGER(dm)[1];
  INTEGER(dim)[1] = INTEGER(dm)[0];
  Rf_setAttrib(res_, R_DimSymbol, dim);
  SEXP cls = PROTECT(Rf_mkString("nativeRaster"));
  Rf_setAttrib(res_, R_ClassSymbol, cls);
  SEXP chsym = Rf_install("channels");
  Rf_setAttrib(res_, chsym, PROTECT(Rf_ScalarInteger(3)));
  UNPROTECT(4);
  return res_;
}

inline SEXP C_native_rgba(SEXP b0, SEXP b1, SEXP b2, SEXP b3, SEXP dm) {
  R_xlen_t n = Rf_xlength(b0);
  SEXP res_ = PROTECT(Rf_allocVector(INTSXP, n));
  int *pres = INTEGER(res_);
  Rbyte *p0 = RAW(b0), *p1 = RAW(b1), *p2 = RAW(b2), *p3 = RAW(b3);
  for (R_xlen_t i = 0; i < n; i++) {
    pres[i] = (int)R_RGBA(p0[i], p1[i], p2[i], p3[i]);
  }
  SEXP dim = PROTECT(Rf_allocVector(INTSXP, 2));
  INTEGER(dim)[0] = INTEGER(dm)[1];
  INTEGER(dim)[1] = INTEGER(dm)[0];
  Rf_setAttrib(res_, R_DimSymbol, dim);
  SEXP cls = PROTECT(Rf_mkString("nativeRaster"));
  Rf_setAttrib(res_, R_ClassSymbol, cls);
  Rf_setAttrib(res_, Rf_install("channels"), PROTECT(Rf_ScalarInteger(4)));
  UNPROTECT(4);
  return res_;
}


inline list replace_nativeRaster(list inputlist, R_xlen_t dimx, R_xlen_t dimy) {
  writable::list outlist_nara;
  writable::integers dm = {(int)dimx, (int)dimy};
  // GRAY
  if (inputlist.size() == 1) {
    outlist_nara.push_back(C_native_gray(inputlist[0], dm));
  }
  // RGB
  if (inputlist.size() == 3) {
    outlist_nara.push_back(C_native_rgb(inputlist[0], inputlist[1], inputlist[2], dm));
  }
  // RGBA (we ignore bands above 4)
  if (inputlist.size() >= 4) {
    outlist_nara.push_back(C_native_rgba(inputlist[0], inputlist[1], inputlist[2], inputlist[3], dm));
  }
  return outlist_nara;
}

// does it have subdatasets?
inline bool gdal_has_subdataset(GDALDataset *poDataset) {
  char **MDdomain = GDALGetMetadataDomainList(poDataset);
  int mdi = 0;
  bool has_sds = false;
  while (MDdomain && MDdomain[mdi] != NULL) {
    if (strcmp(MDdomain[mdi], "SUBDATASETS") == 0) {
      has_sds = true;
      CSLDestroy(MDdomain);
      return has_sds;
    }
    mdi++;
  }
  CSLDestroy(MDdomain);
  return has_sds;
}

// obtain the i-th subdataset *name* (i is 1-based)
inline strings gdal_subdataset_1(GDALDataset *poDataset, int i_sds) {
  int sdi = 0;
  writable::strings ret(1);
  CSLConstList SDS2 = poDataset->GetMetadata("SUBDATASETS");
  while (SDS2 && SDS2[sdi] != NULL) {
    if (sdi / 2 == (i_sds -1 )) {
      char  **papszTokens = CSLTokenizeString2(SDS2[sdi ], "=", 0);
      ret[0] = papszTokens[1];
      CSLDestroy( papszTokens );
      break;
    }
    sdi = sdi + 1 + 1;
  }
  return ret;
}

inline strings gdal_list_subdatasets(GDALDataset *poDataset) {
  int sdi = 0;
  CSLConstList  SDS = GDALGetMetadata(poDataset, "SUBDATASETS");
  while (SDS && SDS[sdi] != NULL) {
    sdi++;
  }
  if (sdi < 1) {
    writable::strings empty(1);
    empty[0] = "";
    return empty;
  }
  if (!(sdi % 2 == 0)) {
    writable::strings empty(1);
    empty[0] = "";
    return empty;
  }

  writable::strings ret(sdi/2);
  int dscount = static_cast<int>(ret.size());
  int cnt = 0;
  if (dscount > 0) {
    CSLConstList SDS2 = GDALGetMetadata(poDataset, "SUBDATASETS");
    for (int ii = 0; ii <  sdi; ii++) {
      if (ii % 2 == 0) {
        ret[cnt] = SDS2[ii];
       cnt++;
      }
    }
  }
  return ret;
}

// open the DSN, open a subdataset if sds > 0
inline GDALDatasetH gdalH_open_dsn(const char * dsn, integers sds) {
  GDALDatasetH DS;
  DS = GDALOpen(dsn, GA_ReadOnly);
  if (!DS) return nullptr;
  if (sds[0] > 0 && gdal_has_subdataset((GDALDataset*) DS)) {
    strings sdsnames = gdal_subdataset_1((GDALDataset*)DS, sds[0]);
    if (sdsnames.size() > 0 && std::string(sdsnames[0]) != "") {
      GDALClose((GDALDataset*) DS);
      DS = GDALOpen(std::string(sdsnames[0]).c_str(), GA_ReadOnly);
    }
  }
  return DS;
}


inline GDALDatasetH gdalH_open_avrt(const char* dsn,
                                    doubles extent,
                                    strings projection,
                                    integers sds, integers bands, strings geolocation,
                                    integers overview,
                                    strings options) {

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
  if (overview[0] >= 0) {
    translate_argv.AddString("-ovr");
    translate_argv.AddString(CPLSPrintf("%i", (int)overview[0]));
  }
  if (std::string(projection[0]) != "") {
    OGRSpatialReference *srs = new OGRSpatialReference;
    if (srs->SetFromUserInput(std::string(projection[0]).c_str()) != OGRERR_NONE) {
      Rprintf("cannot set projection (CRS) from input 'projection' argument, ignoring: '%s'\n", std::string(projection[0]).c_str());
    } else {
      translate_argv.AddString("-a_srs");
      translate_argv.AddString(std::string(projection[0]).c_str());
    }
    delete srs;
  }

  GDALDataset* oDS = (GDALDataset*)gdalH_open_dsn(dsn, sds);

  if (geolocation.size() == 2) {
    OGRSpatialReference  *geolsrs = new OGRSpatialReference;
    char *pszGeoSrsWKT = nullptr;
    geolsrs->SetFromUserInput("EPSG:4326");
    geolsrs->exportToWkt(&pszGeoSrsWKT);
    oDS->SetMetadataItem( "SRS", pszGeoSrsWKT, "GEOLOCATION" );
    oDS->SetMetadataItem( "X_DATASET", std::string(geolocation[0]).c_str(), "GEOLOCATION" );
    oDS->SetMetadataItem( "X_BAND", "1" , "GEOLOCATION" );
    oDS->SetMetadataItem( "Y_DATASET", std::string(geolocation[1]).c_str(), "GEOLOCATION" );
    oDS->SetMetadataItem( "Y_BAND", "1" , "GEOLOCATION" );
    oDS->SetMetadataItem( "PIXEL_OFFSET", "0", "GEOLOCATION" );
    oDS->SetMetadataItem( "LINE_OFFSET", "0", "GEOLOCATION" );
    oDS->SetMetadataItem( "PIXEL_STEP", "1", "GEOLOCATION" );
    oDS->SetMetadataItem( "LINE_STEP", "1", "GEOLOCATION" );
    CPLFree(pszGeoSrsWKT);
    delete geolsrs;
  }

  if (oDS == nullptr) return(nullptr);
  int nBands = oDS->GetRasterCount();

  if (bands[0] > 0) {
    for (int iband = 0; iband < bands.size(); iband++ ) {
      if (bands[iband] > nBands) {
        return nullptr;
      } else {
        translate_argv.AddString("-b");
        translate_argv.AddString(CPLSPrintf("%i", (int)bands[iband]));
      }
    }
  }

  for (int iopt = 0; iopt < options.size(); iopt++) {
    translate_argv.AddString(std::string(options[iopt]).c_str());
  }
  GDALTranslateOptions* psTransOptions = GDALTranslateOptionsNew(translate_argv.List(), nullptr);

  GDALDatasetH a_DS = GDALTranslate("", oDS, psTransOptions, nullptr);
  GDALTranslateOptionsFree( psTransOptions );
  return a_DS;
}

inline GDALDatasetH* gdalH_open_multiple(strings dsn, integers sds) {
  GDALDatasetH* poHDS;
  poHDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * static_cast<size_t>(dsn.size())));
  for (int i = 0; i < dsn.size(); i++) poHDS[i] = gdalH_open_dsn(std::string(dsn[i]).c_str(), sds);
  return poHDS;
}
inline GDALDatasetH* gdalH_open_avrt_multiple(strings dsn, doubles extent,
                                              strings projection, integers sds, integers bands, strings options) {
  GDALDatasetH* poHDS;
  poHDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * static_cast<size_t>(dsn.size())));
  writable::strings empty_geol(1);
  empty_geol[0] = "";
  writable::integers neg1 = {-1};
  for (int i = 0; i < dsn.size(); i++) poHDS[i] = gdalH_open_avrt(std::string(dsn[i]).c_str(),  extent, projection, sds, bands, empty_geol, neg1, options);
  return poHDS;
}

// convert an opened GDALDataset to chunk-of-text VRT
inline std::string gdal_vrt_text(GDALDataset* poSrcDS, logicals nomd) {
  std::string result;
  if (EQUAL(poSrcDS->GetDriverName(),  "VRT")) {
    VRTDataset * VRTdcDS = dynamic_cast<VRTDataset *>(poSrcDS );
    if (!(VRTdcDS == nullptr)) result = VRTdcDS->GetMetadata("xml:VRT")[0];
  } else {
    GDALDriver *poDriver = (GDALDriver *)GDALGetDriverByName("VRT");
    if ((bool)nomd[0]) {
      poSrcDS->SetMetadata(nullptr);
      int i_bands = poSrcDS->GetRasterCount();
      for (int ii_b = 0; ii_b < i_bands; ii_b++) poSrcDS->GetRasterBand(ii_b + 1)->SetMetadata(nullptr);
    }
    GDALDataset *VRTDS = poDriver->CreateCopy("", poSrcDS, false, nullptr, nullptr, nullptr);
    if (!(VRTDS == nullptr)) result = VRTDS->GetMetadata("xml:VRT")[0];
    GDALClose((GDALDatasetH) VRTDS);
  }
  return result;
}

inline strings gdal_dsn_vrt(strings dsn, doubles extent, strings projection,
                                    integers sds, integers bands,
                                    strings geolocation, logicals nomd,
                                    integers overview, strings options) {

  writable::strings out(dsn.size());
  GDALDatasetH DS;
  for (int i = 0; i < out.size(); i++) {
    if (extent.size() == 4 || (std::string(projection[0]) != "") || bands[0] > 0 || (std::string(geolocation[0]) != "" ) || sds[0] > 1 || overview[0] > -1 || options.size() > 0) {
      DS = gdalH_open_avrt(std::string(dsn[i]).c_str(), extent, projection, sds, bands, geolocation, overview, options);
    } else {
      DS = gdalH_open_dsn(std::string(dsn[i]).c_str(), sds);
    }

    if (DS == nullptr) {
     out[i] = NA_STRING;
    } else {
      std::string vrt_str = gdal_vrt_text((GDALDataset*) DS, nomd);
      out[i] = vrt_str.c_str();
      GDALClose(DS);
    }
  }

  return out;
}


inline strings gdal_sds_list(const char* pszFilename)
{
  GDALDataset  *poDataset;
  poDataset = (GDALDataset *) GDALOpen( pszFilename, GA_ReadOnly );
  if( poDataset == NULL )
  {
    cpp11::stop("cannot open dataset");
  }

  strings ret;
  if (gdalraster::gdal_has_subdataset(poDataset)) {
    ret = gdalraster::gdal_list_subdatasets(poDataset);
  } else {
    writable::strings a(1);
    a[0] = pszFilename;
    ret = a;
  }
  GDALClose( (GDALDatasetH) poDataset );
  return ret;
}


inline doubles gdal_extent_only(strings dsn) {
  GDALDatasetH hDataset;
  hDataset = GDALOpen(std::string(dsn[0]).c_str(), GA_ReadOnly);
  if( hDataset == nullptr )
  {
    cpp11::stop("cannot open dataset");
  }
  double        adfGeoTransform[6];
  GDALGetGeoTransform(hDataset, adfGeoTransform );
  int nXSize = GDALGetRasterXSize(hDataset);
  int nYSize = GDALGetRasterYSize(hDataset);
  GDALClose( hDataset );
  writable::doubles extent(4);
  extent[0] = adfGeoTransform[0];
  extent[1] = adfGeoTransform[0] + nXSize * adfGeoTransform[1];
  extent[3] = adfGeoTransform[3];
  extent[2] = adfGeoTransform[3] + nYSize * adfGeoTransform[5];
  return extent;
}

inline list gdal_raster_info(strings dsn, logicals min_max)
  {
    GDALDatasetH hDataset;
    hDataset = GDALOpen(std::string(dsn[0]).c_str(), GA_ReadOnly);
    if( hDataset == nullptr )
    {
      cpp11::stop("cannot open dataset");
    }
    int nXSize = GDALGetRasterXSize(hDataset);
    int nYSize = GDALGetRasterYSize(hDataset);

    double        adfGeoTransform[6];
    GDALGetGeoTransform(hDataset, adfGeoTransform );

    if (GDALGetRasterCount(hDataset) < 1) {
      cpp11::stop("no rasters found in dataset");
    }

    writable::doubles trans(6);
    for (int ii = 0; ii < 6; ii++) trans[ii] = adfGeoTransform[ii];

    char **filelist = GDALGetFileList(hDataset);
    writable::strings files;
    if (filelist != NULL) {
      for (size_t i=0; filelist[i] != NULL; i++) {
        files.push_back(filelist[i]);
      }
    }
    CSLDestroy( filelist );

    GDALRasterBandH  hBand;
    int             nBlockXSize, nBlockYSize;
    double          adfMinMax[2];

    hBand = GDALGetRasterBand(hDataset, 1);

    GDALGetBlockSize(hBand, &nBlockXSize, &nBlockYSize);
    if ((bool)min_max[0]) {
      GDALComputeRasterMinMax(hBand, TRUE, adfMinMax);
    }

    int nn = 11;
    writable::list out(nn);
    writable::strings names(nn);
    out[0] = trans;
    names[0] = "geotransform";
    writable::integers dimxy = {nXSize, nYSize};
    out[1] = dimxy;
    names[1] = "dimXY";

    writable::doubles vmmx(2);
    if ((bool)min_max[0]) {
      vmmx[0] = adfMinMax[0];
      vmmx[1] = adfMinMax[1];
    } else {
      vmmx[0] = NA_REAL;
      vmmx[1] = NA_REAL;
    }
    out[2] = vmmx;
    names[2] = "minmax";

    writable::integers tilesxy = {nBlockXSize, nBlockYSize};
    out[3] = tilesxy;
    names[3] = "tilesXY";

    const char *proj;
    proj = GDALGetProjectionRef(hDataset);
    writable::strings projcv(1);
    projcv[0] = proj;
    out[4] = projcv;
    names[4] = "projection";

    int nBands = GDALGetRasterCount(hDataset);
    out[5] = writable::integers({nBands});
    names[5] = "bands";

    char *stri;
    OGRSpatialReference *oSRS = new OGRSpatialReference;
    const char *proj2;
    proj2 = GDALGetProjectionRef(hDataset);
    char **cwkt = (char **) &proj2;

#if GDAL_VERSION_MAJOR <= 2 && GDAL_VERSION_MINOR <= 2
    oSRS->importFromWkt(cwkt);
#else
    oSRS->importFromWkt( (const char**) cwkt);
#endif
    CSLDestroy(cwkt);
    oSRS->exportToProj4(&stri);
    writable::strings projstr(1);
    projstr[0] = stri;
    out[6] = projstr;
    names[6] = "projstring";
    CPLFree(stri);
    delete oSRS;

    int succ;
    writable::doubles ndv(1);
    ndv[0] = GDALGetRasterNoDataValue(hBand, &succ);
    out[7] = ndv;
    names[7] = "nodata_value";

    int ocount = GDALGetOverviewCount(hBand);
    writable::integers oviews(ocount * 2);
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

    out[9] = files;
    names[9] = "filelist";

    writable::strings dtname(1);
    dtname[0] = GDALGetDataTypeName(GDALGetRasterDataType(hBand));
    out[10] = dtname;
    names[10] = "datatype";

    out.names() = names;

    GDALClose( hDataset );
    return out;
  }

inline list gdal_raster_gcp(strings dsn) {
  GDALDatasetH hDataset;
  hDataset = GDALOpen( std::string(dsn[0]).c_str(), GA_ReadOnly);
  if( hDataset == nullptr )
  {
    cpp11::stop("cannot open dataset");
  }

  int gcp_count;
  gcp_count = GDALGetGCPCount(hDataset);
  const char *srcWKT = GDALGetGCPProjection(hDataset);
  writable::list gcpout(6);
  writable::strings gcpnames(6);
  writable::strings gcpCRS(1);
  gcpCRS[0] = srcWKT;
  gcpnames[0] = "Pixel";
  gcpnames[1] = "Line";
  gcpnames[2] = "X";
  gcpnames[3] = "Y";
  gcpnames[4] = "Z";
  gcpnames[5] = "CRS";
  gcpout.names() = gcpnames;
  if (gcp_count > 0) {
    writable::doubles GCPPixel(gcp_count);
    writable::doubles GCPLine(gcp_count);
    writable::doubles GCPX(gcp_count);
    writable::doubles GCPY(gcp_count);
    writable::doubles GCPZ(gcp_count);
    for (int igcp = 0; igcp < gcp_count; ++igcp) {
      const GDAL_GCP *gcp = GDALGetGCPs( hDataset ) + igcp;
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
  } else {
    Rprintf("No GCP (ground control points) found.\n");
  }
  GDALClose( hDataset );
  return gcpout;
}

inline GDALRasterIOExtraArg init_resample_alg(strings resample) {
  GDALRasterIOExtraArg psExtraArg;
  INIT_RASTERIO_EXTRA_ARG(psExtraArg);
  std::string rs = std::string(resample[0]);
  if (rs == "average")           psExtraArg.eResampleAlg = GRIORA_Average;
  if (rs == "bilinear")          psExtraArg.eResampleAlg = GRIORA_Bilinear;
  if (rs == "cubic")             psExtraArg.eResampleAlg = GRIORA_Cubic;
  if (rs == "cubicspline")       psExtraArg.eResampleAlg = GRIORA_CubicSpline;
  if (rs == "gauss")             psExtraArg.eResampleAlg = GRIORA_Gauss;
  if (rs == "lanczos")           psExtraArg.eResampleAlg = GRIORA_Lanczos;
  if (rs == "mode")              psExtraArg.eResampleAlg = GRIORA_Mode;
  if (rs == "nearestneighbour")  psExtraArg.eResampleAlg = GRIORA_NearestNeighbour;
  return psExtraArg;
}

inline list gdal_read_band_values(GDALDataset *hRet,
                                  integers window,
                                  std::vector<int> bands_to_read,
                                  strings band_output_type,
                                  strings resample,
                                  logicals unscale,
                                  logicals nara)
{
  int Xoffset = window[0];
  int Yoffset = window[1];
  int nXSize = window[2];
  int nYSize = window[3];
  int outXSize = window[4];
  int outYSize = window[5];
  int actual_XSize = -1;
  int actual_YSize = -1;

  GDALDataType src_band_type =  GDALGetRasterDataType(GDALGetRasterBand(hRet, bands_to_read[0]));

  if (std::string(band_output_type[0]) != "") {
    std::string bot = std::string(band_output_type[0]);
    if (bot == "Byte")    src_band_type = GDT_Byte;
    if (bot == "UInt16")  src_band_type = GDT_UInt16;
    if (bot == "Int16")   src_band_type = GDT_Int16;
    if (bot == "UInt32")  src_band_type = GDT_UInt32;
    if (bot == "Int32")   src_band_type = GDT_Int32;
    if (bot == "Float32") src_band_type = GDT_Float32;
    if (bot == "Float64") src_band_type = GDT_Float64;
  }

  int sbands = (int)bands_to_read.size();
  writable::list outlist(bands_to_read.size());

  bool band_type_not_supported = true;

  GDALRasterBand *rasterBand;
  int hasNA;
  int hasScale, hasOffset;
  double scale, offset;

  GDALRasterIOExtraArg psExtraArg;
  psExtraArg = init_resample_alg(resample);
  CPLErr err;

  for (int iband = 0; iband < sbands; iband++) {
    rasterBand = ( (GDALDataset *) hRet)->GetRasterBand(bands_to_read[static_cast<size_t>(iband)]);
    if (iband < 1) {
      actual_XSize = rasterBand->GetXSize();
      actual_YSize = rasterBand->GetYSize();
      if (nXSize < 1) nXSize = actual_XSize;
      if (nYSize < 1) nYSize = actual_YSize;
      if (outXSize < 1) outXSize = actual_XSize;
      if (outYSize < 1) outYSize = actual_YSize;
    }

    scale = rasterBand->GetScale(&hasScale);
    offset = rasterBand->GetOffset(&hasOffset);
    if (!(bool)unscale[0]) {
      hasScale = 0;
      hasOffset = 0;
    }

    if (abs(scale - 1.0) <= 1.0e-05 || abs(scale) < 1.0e-05) {
      hasScale = 0;
    }
    // Float path
    if ((src_band_type == GDT_Float64) || (src_band_type == GDT_Float32) || hasScale) {
      std::vector<double> double_scanline( static_cast<size_t>( outXSize * outYSize ));
      err = rasterBand->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                                 &double_scanline[0], outXSize, outYSize, GDT_Float64,
                                 0, 0, &psExtraArg);
      if (err) Rprintf("we have a problem at RasterIO\n");
      writable::doubles res(outXSize * outYSize);

      double dval;
      double naflag = rasterBand->GetNoDataValue(&hasNA);
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
    // Integer path
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
      writable::integers res(outXSize * outYSize);

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

    // Byte path
    if (!hasScale & (src_band_type == GDT_Byte)) {
      std::vector<uint8_t> byte_scanline( static_cast<size_t>( outXSize * outYSize ) );
      err = rasterBand->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                                 &byte_scanline[0], outXSize, outYSize, GDT_Byte,
                                 0, 0, &psExtraArg);
      if (err) Rprintf("we have a problem at RasterIO\n");
      writable::raws res(outXSize * outYSize);

      R_xlen_t isi;
      for (isi = 0; isi < (static_cast<R_xlen_t>(byte_scanline.size())); isi++) {
        res[isi] = byte_scanline[static_cast<size_t>(isi)];
      }
      outlist[iband] = res;
      band_type_not_supported = false;
    }
  }
  if (band_type_not_supported) {
    GDALClose(hRet);
    cpp11::stop("band type not supported (is it Complex? report at hypertidy/vapour/issues)");
  }

  if ((bool)nara[0] && std::string(band_output_type[0]) == "Byte") {
    return replace_nativeRaster(outlist, (R_xlen_t) outXSize, (R_xlen_t) outYSize);
  }
  return outlist;
}


inline doubles gdal_read_band_value(GDALDataset *hRet,
                                  integers window,
                                  std::vector<int> bands_to_read,
                                  strings band_output_type,
                                  strings resample,
                                  logicals unscale)
{
  int Xoffset = window[0];
  int Yoffset = window[1];
  int nXSize = window[2];
  int nYSize = window[3];
  int outXSize = window[4];
  int outYSize = window[5];
  int actual_XSize = -1;
  int actual_YSize = -1;

  GDALDataType src_band_type =  GDALGetRasterDataType(GDALGetRasterBand(hRet, bands_to_read[0]));

  if (std::string(band_output_type[0]) != "") {
    std::string bot = std::string(band_output_type[0]);
    if (bot == "Byte")    src_band_type = GDT_Byte;
    if (bot == "UInt16")  src_band_type = GDT_UInt16;
    if (bot == "Int16")   src_band_type = GDT_Int16;
    if (bot == "UInt32")  src_band_type = GDT_UInt32;
    if (bot == "Int32")   src_band_type = GDT_Int32;
    if (bot == "Float32") src_band_type = GDT_Float32;
    if (bot == "Float64") src_band_type = GDT_Float64;
  }

  int sbands = (int)bands_to_read.size();
  writable::doubles outlist(1);

  bool band_type_not_supported = true;

  GDALRasterBand *rasterBand;
  int hasNA;
  int hasScale, hasOffset;
  double scale, offset;

  GDALRasterIOExtraArg psExtraArg;
  psExtraArg = init_resample_alg(resample);
  CPLErr err;

  for (int iband = 0; iband < sbands; iband++) {
    rasterBand = hRet->GetRasterBand(bands_to_read[static_cast<size_t>(iband)]);
    if (iband < 1) {
      actual_XSize = rasterBand->GetXSize();
      actual_YSize = rasterBand->GetYSize();
      if (nXSize < 1) nXSize = actual_XSize;
      if (nYSize < 1) nYSize = actual_YSize;
      if (outXSize < 1) outXSize = actual_XSize;
      if (outYSize < 1) outYSize = actual_YSize;
    }

    scale = rasterBand->GetScale(&hasScale);
    offset = rasterBand->GetOffset(&hasOffset);

    if (abs(scale - 1.0) <= 1.0e-05 || abs(scale) < 1.0e-05) {
      hasScale = 0;
    }
    if ((src_band_type == GDT_Float64) || (src_band_type == GDT_Float32) || hasScale) {
      std::vector<double> double_scanline( static_cast<size_t>( outXSize * outYSize ));
      err = rasterBand->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                                 &double_scanline[0], outXSize, outYSize, GDT_Float64,
                                 0, 0, &psExtraArg);
      if (err) Rprintf("we have a problem at RasterIO\n");

      double dval;
      double naflag = rasterBand->GetNoDataValue(&hasNA);
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
      dval = double_scanline[0];
      if (hasScale) dval = dval * scale;
      if (hasOffset) dval = dval + offset;
      outlist[0] = dval;
      band_type_not_supported = false;
    }
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

      int dval;
      double naflag = GDALGetRasterNoDataValue(rasterBand, &hasNA);
      dval = integer_scanline[0];
      if (static_cast<double>(dval) <= naflag) {
        dval = NA_INTEGER;
      }
      outlist[0] = (double)dval;
      band_type_not_supported = false;
    }

    if (!hasScale & (src_band_type == GDT_Byte)) {
      std::vector<uint8_t> byte_scanline( static_cast<size_t>( outXSize * outYSize ) );
      err = rasterBand->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                                 &byte_scanline[0], outXSize, outYSize, GDT_Byte,
                                 0, 0, &psExtraArg);
      if (err) Rprintf("we have a problem at RasterIO\n");
      outlist[0] = (double)byte_scanline[0];
      band_type_not_supported = false;
    }
  }
  if (band_type_not_supported) {
    GDALClose(hRet);
    cpp11::stop("band type not supported (is it Complex? report at hypertidy/vapour/issues)");
  }

  return outlist;
}


  inline list gdal_read_dataset_values(GDALDataset *hRet,
                                       integers window,
                                       std::vector<int> bands_to_read,
                                       strings band_output_type,
                                       strings resample,
                                       logicals unscale)
  {
    int Xoffset = window[0];
    int Yoffset = window[1];
    int nXSize = window[2];
    int nYSize = window[3];
    int outXSize = window[4];
    int outYSize = window[5];

    GDALDataType src_band_type =  GDALGetRasterDataType(GDALGetRasterBand(hRet, bands_to_read[0]));

    if (std::string(band_output_type[0]) != "") {
      std::string bot = std::string(band_output_type[0]);
      if (bot == "Byte")    src_band_type = GDT_Byte;
      if (bot == "UInt16")  src_band_type = GDT_UInt16;
      if (bot == "Int16")   src_band_type = GDT_Int16;
      if (bot == "UInt32")  src_band_type = GDT_UInt32;
      if (bot == "Int32")   src_band_type = GDT_Int32;
      if (bot == "Float32") src_band_type = GDT_Float32;
      if (bot == "Float64") src_band_type = GDT_Float64;
    }

    writable::list outlist(1);
    bool band_type_not_supported = true;

    GDALRasterBand *rasterBand;
    int hasNA;
    int hasScale, hasOffset;
    double scale, offset;
    int actual_XSize = -1;
    int actual_YSize = -1;

    GDALRasterIOExtraArg psExtraArg;
    psExtraArg = init_resample_alg(resample);
    CPLErr err;

    rasterBand = ((GDALDataset *)hRet)->GetRasterBand(bands_to_read[0]);
    actual_XSize = rasterBand->GetXSize();
    actual_YSize = rasterBand->GetYSize();

    if (nXSize < 1) nXSize = actual_XSize;
    if (nYSize < 1) nYSize = actual_YSize;
    if (outXSize < 1) outXSize = actual_XSize;
    if (outYSize < 1) outYSize = actual_YSize;

    size_t n_values_out = static_cast<size_t>(outXSize * outYSize) * bands_to_read.size();

    scale = rasterBand->GetScale(&hasScale);
    offset = rasterBand->GetOffset(&hasOffset);

    if (abs(scale - 1.0) <= 1.0e-09 || abs(scale) < 1.0e-09) {
      hasScale = 0;
    }
    if ((src_band_type == GDT_Float64) || (src_band_type == GDT_Float32) || hasScale) {
      std::vector<double> double_scanline( n_values_out);
      err = ((GDALDataset*)hRet)->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
             &double_scanline[0], outXSize, outYSize, GDT_Float64,
             0, 0,
             0, 0, 0, &psExtraArg);
      if (err) Rprintf("we have a problem at RasterIO\n");
      writable::doubles res(static_cast<R_xlen_t>(n_values_out));

      double dval;
      double naflag = rasterBand->GetNoDataValue(&hasNA);
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
      writable::integers res(static_cast<R_xlen_t>(n_values_out));

      int dval;
      double naflag = GDALGetRasterNoDataValue(rasterBand, &hasNA);

      if (hasNA ) {
        std::replace(integer_scanline.begin(), integer_scanline.end(), (int) naflag, (int) NAN);
      }
      R_xlen_t isi;
      for (isi = 0; isi < (static_cast<R_xlen_t>(integer_scanline.size())); isi++) {
        dval = integer_scanline[static_cast<size_t>(isi)];
        res[isi] = dval;
      }
      outlist[0] = res;
      band_type_not_supported = false;
    }

    if (!hasScale & (src_band_type == GDT_Byte)) {
      std::vector<uint8_t> byte_scanline( n_values_out );
      err = ((GDALDataset *)hRet)->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
             &byte_scanline[0], outXSize, outYSize, GDT_Byte,
             0, 0,
             0, 0, 0,
             &psExtraArg);
      if (err) Rprintf("we have a problem at RasterIO\n");
      writable::raws res(static_cast<R_xlen_t>(n_values_out));
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

    if (band_type_not_supported) {
      GDALClose(hRet);
      cpp11::stop("band type not supported (is it Complex? report at hypertidy/vapour/issues)");
    }

    return outlist;
  }


inline list gdal_raster_dataset_io(strings dsn,
                                   integers window,
                                   integers band,
                                   strings resample,
                                   strings band_output_type)
{
  GDALDataset  *poDataset;
  poDataset = (GDALDataset *) GDALOpen(std::string(dsn[0]).c_str(), GA_ReadOnly );
  if( poDataset == NULL )
  {
    cpp11::stop("cannot open dataset");
  }
  if (band[0] < 1) { GDALClose(poDataset);  cpp11::stop("requested band %i should be 1 or greater", (int)band[0]);  }
  int nBands = poDataset->GetRasterCount();

    if (band[0] > nBands) { GDALClose(poDataset);   cpp11::stop("requested band %i should be equal to or less than number of bands: %i", (int)band[0], nBands); }

  std::vector<int> bands_to_read(static_cast<size_t>(band.size()));
  if (band.size() == 1 && band[0] == 0) {
    for (int i = 0; i < nBands; i++) bands_to_read[static_cast<size_t>(i)] = i + 1;
  } else {
    for (int i = 0; i < band.size(); i++) bands_to_read[static_cast<size_t>(i)] = band[i];
  }
  writable::logicals f_unscale(1);
  f_unscale[0] = FALSE;
  list out = gdal_read_dataset_values(poDataset, window, bands_to_read, band_output_type, resample, f_unscale);
  GDALClose(poDataset );
  return out;
}

inline list gdal_raster_io(strings dsn,
                           integers window,
                           integers band,
                           strings resample,
                           strings band_output_type,
                           logicals unscale,
                           logicals nara)
{
  GDALDataset  *poDataset;
  poDataset = (GDALDataset *) GDALOpen(std::string(dsn[0]).c_str(), GA_ReadOnly );
  if( poDataset == NULL )
  {
    cpp11::stop("cannot open dataset");
  }
  if (band[0] < 1) { GDALClose(poDataset);  cpp11::stop("requested band %i should be 1 or greater", (int)band[0]);  }
  int nBands = poDataset->GetRasterCount();
  if (band[0] > nBands) { GDALClose(poDataset);   cpp11::stop("requested band %i should be equal to or less than number of bands: %i", (int)band[0], nBands); }

  std::vector<int> bands_to_read(static_cast<size_t>(band.size()));
  if (band.size() == 1 && band[0] == 0) {
    for (int i = 0; i < nBands; i++) bands_to_read[static_cast<size_t>(i)] = i + 1;
  } else {
    for (int i = 0; i < band.size(); i++) bands_to_read[static_cast<size_t>(i)] = band[i];
  }
  list out = gdal_read_band_values(poDataset, window, bands_to_read, band_output_type, resample, unscale, nara);
  GDALClose(poDataset );
  return out;
}


inline logicals gdal_has_geolocation(strings dsn, integers sds) {
  GDALDataset* poDataset;
  poDataset = (GDALDataset*)gdalH_open_dsn(std::string(dsn[0]).c_str(), sds);
  bool has_geol = false;
  CSLConstList  papszGeolocationInfo = poDataset->GetMetadata("GEOLOCATION");
  if( papszGeolocationInfo != nullptr ) {
    has_geol = true;
  }
  GDALClose(poDataset);
  writable::logicals out(1);
  out[0] = (Rboolean)has_geol;
  return out;
}

inline list gdal_list_geolocation(strings dsn, integers sds) {
  writable::list out(1);

  if (!(bool)gdal_has_geolocation(dsn, sds)[0]) {
    return out;
  }
  GDALDataset* poDataset;
  poDataset = (GDALDataset*)gdalH_open_dsn(std::string(dsn[0]).c_str(), sds);

  CSLConstList papszGeolocationInfo = poDataset->GetMetadata("GEOLOCATION");

  if( papszGeolocationInfo == nullptr ) {
      GDALClose(poDataset);
    return out;
  }
  writable::strings ret(11);

  ret[0] = CSLFetchNameValue( papszGeolocationInfo, "X_DATASET");
  ret[1] = CSLFetchNameValue( papszGeolocationInfo, "Y_DATASET");
  ret[2] = CSLFetchNameValue( papszGeolocationInfo, "X_BAND");
  ret[3] = CSLFetchNameValue( papszGeolocationInfo, "Y_BAND");
  ret[4] = CSLFetchNameValue( papszGeolocationInfo, "Z_DATASET");
  ret[5] = CSLFetchNameValue( papszGeolocationInfo, "Z_BAND");
  ret[6] = CSLFetchNameValue( papszGeolocationInfo, "SRS");
  ret[7] = CSLFetchNameValue( papszGeolocationInfo, "PIXEL_OFFSET");
  ret[8] = CSLFetchNameValue( papszGeolocationInfo, "LINE_OFFSET");
  ret[9] = CSLFetchNameValue( papszGeolocationInfo, "LINE_STEP");
  ret[10] = CSLFetchNameValue( papszGeolocationInfo, "GEOREFERENCING_CONVENTION");

  out[0] = ret;

  return out;
}


} //  namespace gdalraster
#endif
