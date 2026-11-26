#ifndef GDALRASTER_H
#define GDALRASTER_H
#include <cpp11.hpp>
#include "common/common_vapour.h"
#include "common/gdal_handles.h"
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

// --- nativeRaster helpers ---
// Allocation goes through cpp11 so that a failure unwinds the C++ stack, and
// with it any open dataset, rather than jumping straight out to R.
inline SEXP native_raster_finish(cpp11::writable::integers &res, SEXP dm, int channels) {
  cpp11::writable::integers dim = {INTEGER(dm)[1], INTEGER(dm)[0]};
  res.attr("dim") = dim;
  res.attr("class") = "nativeRaster";
  res.attr("channels") = cpp11::writable::integers({channels});
  return res;
}

inline SEXP C_native_gray(SEXP b0, SEXP dm) {
  R_xlen_t n = Rf_xlength(b0);
  cpp11::writable::integers res(n);
  int *pres = INTEGER(res);
  Rbyte *p0 = RAW(b0);
  for (R_xlen_t i = 0; i < n; i++) {
    Rbyte v = p0[i];
    pres[i] = (int)R_GRAY(v);
  }
  return native_raster_finish(res, dm, 3);
}

inline SEXP C_native_rgb(SEXP b0, SEXP b1, SEXP b2, SEXP dm) {
  R_xlen_t n = Rf_xlength(b0);
  cpp11::writable::integers res(n);
  int *pres = INTEGER(res);
  Rbyte *p0 = RAW(b0), *p1 = RAW(b1), *p2 = RAW(b2);
  for (R_xlen_t i = 0; i < n; i++) {
    pres[i] = (int)R_RGB(p0[i], p1[i], p2[i]);
  }
  return native_raster_finish(res, dm, 3);
}

inline SEXP C_native_rgba(SEXP b0, SEXP b1, SEXP b2, SEXP b3, SEXP dm) {
  R_xlen_t n = Rf_xlength(b0);
  cpp11::writable::integers res(n);
  int *pres = INTEGER(res);
  Rbyte *p0 = RAW(b0), *p1 = RAW(b1), *p2 = RAW(b2), *p3 = RAW(b3);
  for (R_xlen_t i = 0; i < n; i++) {
    pres[i] = (int)R_RGBA(p0[i], p1[i], p2[i], p3[i]);
  }
  return native_raster_finish(res, dm, 4);
}


inline list replace_nativeRaster(list inputlist, R_xlen_t dimx, R_xlen_t dimy) {
  writable::list outlist_nara;
  writable::integers dm = {(int)dimx, (int)dimy};
  // GRAY
  if (inputlist.size() == 1) {
    cpp11::sexp nr = C_native_gray(inputlist[0], dm);
    outlist_nara.push_back(nr);
  }
  // RGB
  if (inputlist.size() == 3) {
    cpp11::sexp nr = C_native_rgb(inputlist[0], inputlist[1], inputlist[2], dm);
    outlist_nara.push_back(nr);
  }
  // RGBA (we ignore bands above 4)
  if (inputlist.size() >= 4) {
    cpp11::sexp nr = C_native_rgba(inputlist[0], inputlist[1], inputlist[2], inputlist[3], dm);
    outlist_nara.push_back(nr);
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
      // SUBDATASET_n_NAME=<name>, and the name can itself contain '='
      const char *eq = strchr(SDS2[sdi], '=');
      if (eq != nullptr) ret[0] = eq + 1;
      break;
    }
    sdi = sdi + 1 + 1;
  }
  return ret;
}

// Open a raster source, or its subdataset number 'sds' (from 1) when it has
// subdatasets and one is asked for. The handle is empty when the open fails.
inline gdalcommon::DatasetPtr open_raster(const char *dsn, integers sds, strings open_options,
                                          unsigned int flags = GDAL_OF_RASTER) {
  gdalcommon::DatasetPtr DS = gdalcommon::open_dataset(dsn, flags, open_options);
  if (!DS) return DS;
  int isds = sds.size() > 0 ? sds[0] : 0;
  if (isds != NA_INTEGER && isds > 0 && gdal_has_subdataset(DS.get())) {
    strings sdsnames = gdal_subdataset_1(DS.get(), isds);
    if (sdsnames.size() > 0 && std::string(sdsnames[0]) != "") {
      std::string sdsname = std::string(sdsnames[0]);
      DS = gdalcommon::open_dataset(sdsname.c_str(), flags, open_options);
    }
  }
  return DS;
}

inline gdalcommon::DatasetPtr open_raster_or_stop(const char *dsn, integers sds, strings open_options,
                                                  unsigned int flags = GDAL_OF_RASTER) {
  gdalcommon::DatasetPtr DS = open_raster(dsn, sds, open_options, flags);
  if (!DS) gdalcommon::stop_gdal(std::string("cannot open dataset '") + dsn + "'");
  return DS;
}

// Open a raster source as a VRT of it, with the extent, projection, bands,
// geolocation arrays or overview level replaced, and any gdal_translate
// 'options' applied. The handle is empty when the source cannot be opened, a
// band is not there, or GDAL cannot build the VRT.
inline gdalcommon::DatasetPtr open_raster_vrt(const char* dsn,
                                              doubles extent,
                                              strings projection,
                                              integers sds, integers bands, strings geolocation,
                                              integers overview,
                                              strings options,
                                              strings open_options) {

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
    OGRSpatialReference srs;
    if (srs.SetFromUserInput(as_cstr(projection[0])) != OGRERR_NONE) {
      cpp11::warning("cannot set projection (CRS) from input 'projection' argument, ignoring: '%s'", as_cstr(projection[0]));
    } else {
      translate_argv.AddString("-a_srs");
      translate_argv.AddString(as_cstr(projection[0]));
    }
  }

  gdalcommon::DatasetPtr oDS = open_raster(dsn, sds, open_options);
  if (!oDS) return oDS;

  if (geolocation.size() == 2) {
    OGRSpatialReference geolsrs;
    char *pszGeoSrsWKT = nullptr;
    geolsrs.SetFromUserInput("EPSG:4326");
    geolsrs.exportToWkt(&pszGeoSrsWKT);
    gdalcommon::CPLStringPtr wkt_owner(pszGeoSrsWKT);
    // the geolocation is for the VRT, it is not to be written beside the source
    gdalcommon::no_sidecar_save(oDS.get());
    oDS->SetMetadataItem( "SRS", pszGeoSrsWKT, "GEOLOCATION" );
    oDS->SetMetadataItem( "X_DATASET", as_cstr(geolocation[0]), "GEOLOCATION" );
    oDS->SetMetadataItem( "X_BAND", "1" , "GEOLOCATION" );
    oDS->SetMetadataItem( "Y_DATASET", as_cstr(geolocation[1]), "GEOLOCATION" );
    oDS->SetMetadataItem( "Y_BAND", "1" , "GEOLOCATION" );
    oDS->SetMetadataItem( "PIXEL_OFFSET", "0", "GEOLOCATION" );
    oDS->SetMetadataItem( "LINE_OFFSET", "0", "GEOLOCATION" );
    oDS->SetMetadataItem( "PIXEL_STEP", "1", "GEOLOCATION" );
    oDS->SetMetadataItem( "LINE_STEP", "1", "GEOLOCATION" );
  }

  int nBands = oDS->GetRasterCount();

  if (bands[0] != NA_INTEGER && bands[0] > 0) {
    for (int iband = 0; iband < bands.size(); iband++ ) {
      if (bands[iband] == NA_INTEGER || bands[iband] < 1 || bands[iband] > nBands) {
        return gdalcommon::DatasetPtr();
      } else {
        translate_argv.AddString("-b");
        translate_argv.AddString(CPLSPrintf("%i", (int)bands[iband]));
      }
    }
  }

  for (int iopt = 0; iopt < options.size(); iopt++) {
    translate_argv.AddString(as_cstr(options[iopt]));
  }
  gdalcommon::TranslateOptionsPtr psTransOptions(GDALTranslateOptionsNew(translate_argv.List(), nullptr));
  if (!psTransOptions) gdalcommon::stop_gdal("invalid VRT options");

  // the VRT takes its own reference to the source, so our handle on the
  // source can go when this function returns
  GDALDatasetH a_DS = GDALTranslate("", (GDALDatasetH) oDS.get(), psTransOptions.get(), nullptr);
  return gdalcommon::DatasetPtr((GDALDataset *) a_DS);
}

// convert an opened GDALDataset to chunk-of-text VRT
inline std::string gdal_vrt_text(GDALDataset* poSrcDS, logicals nomd) {
  std::string result;
  bool drop_md = (bool)nomd[0];
  if (EQUAL(poSrcDS->GetDriverName(),  "VRT")) {
    CSLConstList xml = poSrcDS->GetMetadata("xml:VRT");
    if (xml != nullptr && xml[0] != nullptr) result = xml[0];
  } else {
    GDALDriver *poDriver = (GDALDriver *)GDALGetDriverByName("VRT");
    if (poDriver == nullptr) cpp11::stop("the VRT driver is not available");
    gdalcommon::DatasetPtr VRTDS(poDriver->CreateCopy("", poSrcDS, false, nullptr, nullptr, nullptr));
    if (VRTDS) {
      if (drop_md) {
        // metadata is dropped from the VRT copy and never from the source: on
        // the source it would be saved beside the file when the source closes
        VRTDS->SetMetadata(nullptr);
        int i_bands = VRTDS->GetRasterCount();
        for (int ii_b = 0; ii_b < i_bands; ii_b++) VRTDS->GetRasterBand(ii_b + 1)->SetMetadata(nullptr);
      }
      CSLConstList xml = VRTDS->GetMetadata("xml:VRT");
      if (xml != nullptr && xml[0] != nullptr) result = xml[0];
    }
  }
  return result;
}

inline strings gdal_dsn_vrt(strings dsn, doubles extent, strings projection,
                                    integers sds, integers bands,
                                    strings geolocation, logicals nomd,
                                    integers overview, strings options,
                                    strings open_options) {

  writable::strings out(dsn.size());
  for (int i = 0; i < out.size(); i++) {
    gdalcommon::DatasetPtr DS;
    if (extent.size() == 4 || (std::string(projection[0]) != "") || bands[0] > 0 || (std::string(geolocation[0]) != "" ) || sds[0] > 1 || overview[0] > -1 || options.size() > 0) {
      DS = open_raster_vrt(as_cstr(dsn[i]), extent, projection, sds, bands, geolocation, overview, options, open_options);
    } else {
      DS = open_raster(as_cstr(dsn[i]), sds, open_options);
    }

    if (!DS) {
     out[i] = NA_STRING;
    } else {
      std::string vrt_str = gdal_vrt_text(DS.get(), nomd);
      if (vrt_str.empty()) {
        out[i] = NA_STRING;
      } else {
        out[i] = vrt_str.c_str();
      }
    }
  }

  return out;
}


inline doubles gdal_extent_only(strings dsn) {
  writable::integers sds0 = {0};
  writable::strings no_options((R_xlen_t)0);
  gdalcommon::DatasetPtr poDataset = open_raster_or_stop(as_cstr(dsn[0]), sds0, no_options);
  double        adfGeoTransform[6];
  poDataset->GetGeoTransform(adfGeoTransform);
  int nXSize = poDataset->GetRasterXSize();
  int nYSize = poDataset->GetRasterYSize();
  writable::doubles extent(4);
  extent[0] = adfGeoTransform[0];
  extent[1] = adfGeoTransform[0] + nXSize * adfGeoTransform[1];
  extent[3] = adfGeoTransform[3];
  extent[2] = adfGeoTransform[3] + nYSize * adfGeoTransform[5];
  return extent;
}

inline list gdal_raster_info(strings dsn, logicals min_max)
  {
    writable::integers sds0 = {0};
    writable::strings no_options((R_xlen_t)0);
    gdalcommon::DatasetPtr poDataset = open_raster_or_stop(as_cstr(dsn[0]), sds0, no_options);
    GDALDatasetH hDataset = (GDALDatasetH) poDataset.get();
    int nXSize = GDALGetRasterXSize(hDataset);
    int nYSize = GDALGetRasterYSize(hDataset);

    double        adfGeoTransform[6];
    GDALGetGeoTransform(hDataset, adfGeoTransform );

    if (GDALGetRasterCount(hDataset) < 1) {
      cpp11::stop("no rasters found in dataset");
    }

    writable::doubles trans(6);
    for (int ii = 0; ii < 6; ii++) trans[ii] = adfGeoTransform[ii];

    CPLStringList filelist(GDALGetFileList(hDataset));
    writable::strings files;
    for (int i = 0; i < filelist.size(); i++) {
      files.push_back(filelist[i]);
    }

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

    // the WKT belongs to the dataset, importFromWkt() only reads it
    writable::strings projstr(1);
    projstr[0] = "";
    {
      OGRSpatialReference oSRS;
      char *stri = nullptr;
      {
        gdalcommon::QuietErrors quiet;
        if (proj != nullptr && oSRS.importFromWkt(proj) == OGRERR_NONE) {
          oSRS.exportToProj4(&stri);
        }
      }
      gdalcommon::CPLStringPtr stri_owner(stri);
      if (stri != nullptr) projstr[0] = stri;
    }
    out[6] = projstr;
    names[6] = "projstring";

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
    return out;
  }

inline list gdal_raster_gcp(strings dsn, strings open_options) {
  writable::integers sds0 = {0};
  gdalcommon::DatasetPtr poDataset = open_raster_or_stop(as_cstr(dsn[0]), sds0, open_options);
  GDALDatasetH hDataset = (GDALDatasetH) poDataset.get();

  int gcp_count;
  gcp_count = GDALGetGCPCount(hDataset);
  const char *srcWKT = GDALGetGCPProjection(hDataset);
  writable::list gcpout(6);
  writable::strings gcpnames(6);
  writable::strings gcpCRS(1);
  gcpCRS[0] = srcWKT == nullptr ? "" : srcWKT;
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
    const GDAL_GCP *gcps = GDALGetGCPs( hDataset );
    for (int igcp = 0; igcp < gcp_count; ++igcp) {
      const GDAL_GCP *gcp = gcps + igcp;
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

  // an explicit integer output type asks for the stored values, as does
  // unscale = FALSE: scale and offset are then not applied
  bool apply_scale = (bool)unscale[0];
  {
    std::string bot = std::string(band_output_type[0]);
    if (bot == "Byte" || bot == "UInt16" || bot == "Int16" || bot == "UInt32" || bot == "Int32") {
      apply_scale = false;
    }
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
    if (!apply_scale) {
      hasScale = 0;
      hasOffset = 0;
    }

    if (abs(scale - 1.0) <= 1.0e-05 || abs(scale) < 1.0e-05) {
      hasScale = 0;
    }
    if (abs(offset) < 1.0e-12) {
      hasOffset = 0;
    }
    // an offset alone also means the values are to be converted
    bool do_scale = hasScale || hasOffset;
    // Float path
    if ((src_band_type == GDT_Float64) || (src_band_type == GDT_Float32) || do_scale) {
      std::vector<double> double_scanline( static_cast<size_t>(outXSize) * static_cast<size_t>(outYSize));
      err = rasterBand->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                                 &double_scanline[0], outXSize, outYSize, GDT_Float64,
                                 0, 0, &psExtraArg);
      if (err != CE_None) gdalcommon::stop_gdal("raster read failed");
      writable::doubles res(static_cast<R_xlen_t>(outXSize) * static_cast<R_xlen_t>(outYSize));

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
    if ((!do_scale) &
        ((src_band_type == GDT_Int16) |
        (src_band_type == GDT_Int32) |
        (src_band_type == GDT_UInt16) |
        (src_band_type == GDT_UInt32)))  {
      std::vector<int32_t> integer_scanline(static_cast<size_t>(outXSize) * static_cast<size_t>(outYSize));
      err = rasterBand->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                                 &integer_scanline[0], outXSize, outYSize, GDT_Int32,
                                 0, 0, &psExtraArg);
      if (err != CE_None) gdalcommon::stop_gdal("raster read failed");
      writable::integers res(static_cast<R_xlen_t>(outXSize) * static_cast<R_xlen_t>(outYSize));

      int dval;
      double naflag = GDALGetRasterNoDataValue(rasterBand, &hasNA);

      R_xlen_t isi;
      for (isi = 0; isi < (static_cast<R_xlen_t>(integer_scanline.size())); isi++) {
        dval = integer_scanline[static_cast<size_t>(isi)];
        // only the nodata value itself is missing, not everything below it
        if (hasNA && static_cast<double>(dval) == naflag) {
          dval = NA_INTEGER;
        }
        res[isi] = dval;
      }
      outlist[iband] = res;
      band_type_not_supported = false;
    }

    // Byte path
    if (!do_scale & (src_band_type == GDT_Byte)) {
      std::vector<uint8_t> byte_scanline( static_cast<size_t>(outXSize) * static_cast<size_t>(outYSize) );
      err = rasterBand->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                                 &byte_scanline[0], outXSize, outYSize, GDT_Byte,
                                 0, 0, &psExtraArg);
      if (err != CE_None) gdalcommon::stop_gdal("raster read failed");
      writable::raws res(static_cast<R_xlen_t>(outXSize) * static_cast<R_xlen_t>(outYSize));

      R_xlen_t isi;
      for (isi = 0; isi < (static_cast<R_xlen_t>(byte_scanline.size())); isi++) {
        res[isi] = byte_scanline[static_cast<size_t>(isi)];
      }
      outlist[iband] = res;
      band_type_not_supported = false;
    }
  }
  if (band_type_not_supported) {
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
      std::vector<double> double_scanline( static_cast<size_t>(outXSize) * static_cast<size_t>(outYSize));
      err = rasterBand->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                                 &double_scanline[0], outXSize, outYSize, GDT_Float64,
                                 0, 0, &psExtraArg);
      if (err != CE_None) gdalcommon::stop_gdal("raster read failed");

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
      std::vector<int32_t> integer_scanline(static_cast<size_t>(outXSize) * static_cast<size_t>(outYSize));
      err = rasterBand->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                                 &integer_scanline[0], outXSize, outYSize, GDT_Int32,
                                 0, 0, &psExtraArg);
      if (err != CE_None) gdalcommon::stop_gdal("raster read failed");

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
      std::vector<uint8_t> byte_scanline( static_cast<size_t>(outXSize) * static_cast<size_t>(outYSize) );
      err = rasterBand->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                                 &byte_scanline[0], outXSize, outYSize, GDT_Byte,
                                 0, 0, &psExtraArg);
      if (err != CE_None) gdalcommon::stop_gdal("raster read failed");
      outlist[0] = (double)byte_scanline[0];
      band_type_not_supported = false;
    }
  }
  if (band_type_not_supported) {
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

    size_t n_values_out = static_cast<size_t>(outXSize) * static_cast<size_t>(outYSize) * bands_to_read.size();

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
      if (err != CE_None) gdalcommon::stop_gdal("raster read failed");
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
      if (err != CE_None) gdalcommon::stop_gdal("raster read failed");
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
      if (err != CE_None) gdalcommon::stop_gdal("raster read failed");
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
  writable::integers sds0 = {0};
  writable::strings no_options((R_xlen_t)0);
  gdalcommon::DatasetPtr poDataset = open_raster_or_stop(as_cstr(dsn[0]), sds0, no_options);
  int nBands = poDataset->GetRasterCount();
  std::vector<int> bands_to_read;
  if (band.size() == 1 && band[0] == 0) {
    for (int i = 0; i < nBands; i++) bands_to_read.push_back(i + 1);
  } else {
    for (int i = 0; i < band.size(); i++) {
      if (band[i] == NA_INTEGER || band[i] < 1) cpp11::stop("requested band %i should be 1 or greater", (int)band[i]);
      if (band[i] > nBands) cpp11::stop("requested band %i should be equal to or less than number of bands: %i", (int)band[i], nBands);
      bands_to_read.push_back(band[i]);
    }
  }
  if (bands_to_read.empty()) cpp11::stop("no bands to read");
  writable::logicals f_unscale(1);
  f_unscale[0] = FALSE;
  return gdal_read_dataset_values(poDataset.get(), window, bands_to_read, band_output_type, resample, f_unscale);
}

inline list gdal_raster_io(strings dsn,
                           integers window,
                           integers band,
                           strings resample,
                           strings band_output_type,
                           logicals unscale,
                           logicals nara,
                           strings open_options)
{
  writable::integers sds0 = {0};
  gdalcommon::DatasetPtr poDataset = open_raster_or_stop(as_cstr(dsn[0]), sds0, open_options);
  int nBands = poDataset->GetRasterCount();
  if (band.size() < 1) cpp11::stop("no bands to read");
  for (int i = 0; i < band.size(); i++) {
    if (band[i] == NA_INTEGER || band[i] < 1) cpp11::stop("requested band %i should be 1 or greater", (int)band[i]);
    if (band[i] > nBands) cpp11::stop("requested band %i should be equal to or less than number of bands: %i", (int)band[i], nBands);
  }
  std::vector<int> bands_to_read(static_cast<size_t>(band.size()));
  for (int i = 0; i < band.size(); i++) bands_to_read[static_cast<size_t>(i)] = band[i];
  return gdal_read_band_values(poDataset.get(), window, bands_to_read, band_output_type, resample, unscale, nara);
}


inline logicals gdal_has_geolocation(strings dsn, integers sds, strings open_options) {
  gdalcommon::DatasetPtr poDataset = open_raster_or_stop(as_cstr(dsn[0]), sds, open_options);
  CSLConstList  papszGeolocationInfo = poDataset->GetMetadata("GEOLOCATION");
  writable::logicals out(1);
  out[0] = (Rboolean)(papszGeolocationInfo != nullptr);
  return out;
}

// The GEOLOCATION metadata domain as a character vector in a fixed order, an
// item that is not set is NA. A list with one NULL when there is no geolocation.
inline list gdal_list_geolocation(strings dsn, integers sds, strings open_options) {
  writable::list out(1);

  gdalcommon::DatasetPtr poDataset = open_raster_or_stop(as_cstr(dsn[0]), sds, open_options);
  CSLConstList papszGeolocationInfo = poDataset->GetMetadata("GEOLOCATION");
  if( papszGeolocationInfo == nullptr ) {
    return out;
  }
  const char *items[11] = {"X_DATASET", "Y_DATASET", "X_BAND", "Y_BAND", "Z_DATASET", "Z_BAND",
                           "SRS", "PIXEL_OFFSET", "LINE_OFFSET", "LINE_STEP",
                           "GEOREFERENCING_CONVENTION"};
  writable::strings ret(11);
  for (int i = 0; i < 11; i++) {
    const char *value = CSLFetchNameValue(papszGeolocationInfo, items[i]);
    if (value == nullptr) {
      ret[i] = NA_STRING;
    } else {
      ret[i] = value;
    }
  }
  out[0] = ret;
  return out;
}


} //  namespace gdalraster
#endif
