#ifndef GDALWARPGENERAL_H
#define GDALWARPGENERAL_H

#include <cpp11.hpp>
#include "common/common_vapour.h"
#include "common/gdal_handles.h"
#include "gdal_priv.h"
#include "gdalwarper.h"
#include "gdal_utils.h"
#include "gdalraster/gdalraster.h"
#include "ogr_spatialref.h"

namespace gdalwarpgeneral{

using namespace cpp11;
namespace writable = cpp11::writable;


inline list gdal_suggest_warp(GDALDataset* poSrcDS, void *pfnTransformerArg) {
  double        adfGeoTransform[6];
  int nXSize = 0, nYSize = 0;
  double adfExtent[4] = {NA_REAL, NA_REAL, NA_REAL, NA_REAL};
  CPLErr err = GDALSuggestedWarpOutput2(poSrcDS, GDALGenImgProjTransform, pfnTransformerArg,
                                        adfGeoTransform, &nXSize, &nYSize, adfExtent, 0);
  if (err != CE_None) {
    GDALDestroyGenImgProjTransformer(pfnTransformerArg);
    gdalcommon::stop_gdal("cannot suggest a warp output");
  }
  writable::integers dimension = {nXSize, nYSize};
  writable::doubles extent = {adfExtent[0], adfExtent[2], adfExtent[1], adfExtent[3]};
  writable::list out_i = {extent, dimension};
  return out_i;
}

inline list gdal_suggest_warp(strings dsn, strings target_crs) {
  writable::list out(dsn.size());
  writable::integers sds0 = {0};
  writable::strings no_options((R_xlen_t)0);
  for (int i = 0; i < dsn.size(); i++) {
    gdalcommon::DatasetPtr poSrcDS = gdalraster::open_raster_or_stop(as_cstr(dsn[i]), sds0, no_options);
    void *pfnTransformerArg =
     GDALCreateGenImgProjTransformer( (GDALDatasetH) poSrcDS.get(), nullptr, nullptr,
                                     as_cstr(target_crs[0]),
                                               FALSE, 0.0, 1 );
    if (pfnTransformerArg == nullptr) {
      gdalcommon::stop_gdal(std::string("cannot create a transformer for '") + as_cstr(dsn[i]) + "'");
    }
    out[i] = gdal_suggest_warp(poSrcDS.get(), pfnTransformerArg);
    GDALDestroyGenImgProjTransformer(pfnTransformerArg);
  }
  return out;
}

inline list gdal_warp_general(strings dsn,
                              strings target_crs,
                              doubles target_extent,
                              integers target_dim,
                              doubles target_res,
                              integers bands,
                              strings resample,
                              logicals silent,
                              strings band_output_type,
                              strings options,
                              strings dsn_outname,
                              logicals include_meta,
                              logicals nara,
                              logicals unscale,
                              strings open_options) {

  if (dsn.size() < 1) cpp11::stop("no input sources");
  // owners first, then the plain handles GDALWarp() takes
  std::vector<gdalcommon::DatasetPtr> src_owner;
  std::vector<GDALDatasetH> src_ds;
  writable::integers sds0 = {0};
  for (int i = 0; i < dsn.size(); i++) {
    gdalcommon::DatasetPtr ds = gdalraster::open_raster(as_cstr(dsn[i]), sds0, open_options);
    if (!ds) {
      gdalcommon::stop_gdal(std::string("input source not readable: '") + as_cstr(dsn[i]) + "'");
    }
    src_ds.push_back((GDALDatasetH) ds.get());
    src_owner.push_back(std::move(ds));
  }

  CPLStringList argv;

  bool write_dsn = false;
  std::string outname = std::string(dsn_outname[0]);
  if (outname == "") {
    argv.AddString("-of");
    argv.AddString("MEM");
  } else {
    write_dsn = true;
  }

  std::string tcrs = std::string(target_crs[0]);
  if (tcrs != "") {
    OGRSpatialReference oTargetSRS;
    OGRErr target_chk =  oTargetSRS.SetFromUserInput(tcrs.c_str());
    if (target_chk != OGRERR_NONE) gdalcommon::stop_gdal("cannot initialize target projection");
    argv.AddString("-t_srs");
    argv.AddString(tcrs.c_str());
  }

  if (target_extent.size() > 0) {
    if (target_extent.size() != 4) cpp11::stop("'target_extent' must have 4 values: xmin, xmax, ymin, ymax");
    double dfMinX = target_extent[0]; double dfMaxX = target_extent[1];
    double dfMinY = target_extent[2]; double dfMaxY = target_extent[3];
    argv.AddString("-te");
    argv.AddString(CPLSPrintf("%.18g", dfMinX));
    argv.AddString(CPLSPrintf("%.18g", dfMinY));
    argv.AddString(CPLSPrintf("%.18g", dfMaxX));
    argv.AddString(CPLSPrintf("%.18g", dfMaxY));
  }

  if (target_dim.size() > 0) {
    if (target_dim.size() != 2) cpp11::stop("'target_dim' must have 2 values: ncol, nrow");
    int nXSize = target_dim[0]; int nYSize = target_dim[1];
    argv.AddString("-ts");
    argv.AddString(CPLSPrintf("%d", nXSize));
    argv.AddString(CPLSPrintf("%d", nYSize));
  }

  if (target_res.size() > 0) {
    if (target_res.size() != 2) cpp11::stop("'target_res' must have 2 values: xres, yres");
    double XRes = target_res[0]; double YRes = target_res[1];
    if (! (XRes > 0 && YRes > 0)) {
      cpp11::stop("invalid value/s for 'target_res' (not greater than zero)");
    }
    argv.AddString("-tr");
    argv.AddString(CPLSPrintf("%f", XRes));
    argv.AddString(CPLSPrintf("%f", YRes));
  }
  if (resample.size() > 0) {
    argv.AddString("-r");
    argv.AddString(as_cstr(resample[0]));
  }
  for (int gwopt = 0; gwopt < options.size(); gwopt++) {
    if (std::string(options[gwopt]) != "") {
     argv.AddString(as_cstr(options[gwopt]));
    }
  }

  // GDAL returns NULL when it cannot parse the arguments (unknown resampling
  // method, unknown switch, missing value)
  gdalcommon::WarpAppOptionsPtr psOptions(GDALWarpAppOptionsNew(argv.List(), nullptr));
  if (!psOptions) gdalcommon::stop_gdal("invalid warp options");
  GDALWarpAppOptionsSetProgress(psOptions.get(), NULL, NULL );

  gdalcommon::DatasetPtr hRet((GDALDataset *) GDALWarp(outname.c_str(), nullptr,
                                  static_cast<int>(src_ds.size()), src_ds.data(),
                                  psOptions.get(), nullptr));

  if (!hRet) {
    gdalcommon::stop_gdal("data source could not be processed with GDALWarp api");
  }

  writable::list outlist;
  if (write_dsn) {
    outlist.push_back(dsn_outname);
  } else {
    int nBands = hRet->GetRasterCount();
    if (nBands < 1) cpp11::stop("the warped result has no bands");
    int nbands_to_read = (int)bands.size();
    bool all_bands = bands.size() < 1 || bands[0] == NA_INTEGER || bands[0] < 1;
    if (all_bands) {
       nbands_to_read = nBands;
    }
    std::vector<int> bands_to_read(static_cast<size_t>(nbands_to_read));
    for (int i = 0; i < nbands_to_read; i++) {
      if (!all_bands) {
        bands_to_read[static_cast<size_t>(i)] = bands[i];
      } else {
        bands_to_read[static_cast<size_t>(i)] = i + 1;
      }
      int b = bands_to_read[static_cast<size_t>(i)];
      if (b == NA_INTEGER || b < 1 || b > nBands) {
        cpp11::stop("band number is not available: %i", b);
      }
    }
    writable::integers window(6);
    for (int i  = 0; i < window.size(); i++) window[i] = 0;

    outlist = writable::list(
      gdalraster::gdal_read_band_values(hRet.get(),
                                                window,
                                                bands_to_read,
                                                band_output_type,
                                                resample,
                                                unscale, nara));
  }

  if ((bool)include_meta[0]) {
  R_xlen_t dimx =  hRet->GetRasterXSize();
  R_xlen_t dimy =  hRet->GetRasterYSize();
  double        adfGeoTransform[6];
  hRet->GetGeoTransform(adfGeoTransform);
  double xmin = adfGeoTransform[0];
  double xmax = adfGeoTransform[0] + (double)dimx * adfGeoTransform[1];
  double ymin = adfGeoTransform[3] + (double)dimy * adfGeoTransform[5];
  double ymax = adfGeoTransform[3];

  const char *proj = hRet->GetProjectionRef();
  writable::doubles dim_attr = {(double)dimx, (double)dimy};
  writable::doubles ext_attr = {xmin, xmax, ymin, ymax};
  writable::strings proj_attr(1);
  proj_attr[0] = proj == nullptr ? "" : proj;
  outlist.attr("dimension") = dim_attr;
  outlist.attr("extent") = ext_attr;
  outlist.attr("projection") = proj_attr;
  }
  return outlist;
}

} // namespace gdalwarpgeneral
#endif
