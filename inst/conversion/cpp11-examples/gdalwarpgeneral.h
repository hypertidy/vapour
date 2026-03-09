#ifndef GDALWARPGENERAL_H
#define GDALWARPGENERAL_H

#include <cpp11.hpp>
#include "gdal_priv.h"
#include "gdalwarper.h"
#include "gdal_utils.h"
#include "gdalraster/gdalraster.h"
#include "ogr_spatialref.h"
using namespace std;

namespace gdalwarpgeneral{

using namespace cpp11;
namespace writable = cpp11::writable;


list gdal_suggest_warp(GDALDataset* poSrcDS, void *pfnTransformerArg) {
  double        adfGeoTransform[6];
    poSrcDS->GetGeoTransform( adfGeoTransform );
    int nXSize, nYSize;
    double adfExtent[4];
    GDALTransformerFunc pfnTransformer;
    pfnTransformer = GDALGenImgProjTransform;

  GDALSuggestedWarpOutput2(poSrcDS, pfnTransformer, pfnTransformerArg,
                           adfGeoTransform, &nXSize, &nYSize, adfExtent, 0);
  writable::integers dimension = {nXSize, nYSize};
  writable::doubles extent = {adfExtent[0], adfExtent[2], adfExtent[1], adfExtent[3]};
  writable::list out_i = {extent, dimension};
  return out_i;
}

list gdal_suggest_warp(strings dsn, strings target_crs) {
  writable::list out(dsn.size());
  writable::integers sds0 = {0};
  for (int i = 0; i < dsn.size(); i++) {
    GDALDataset* poSrcDS = (GDALDataset*) gdalraster::gdalH_open_dsn(std::string(dsn[0]).c_str(), sds0);
    void *pfnTransformerArg = nullptr;
    pfnTransformerArg =
     GDALCreateGenImgProjTransformer( poSrcDS, nullptr, nullptr,
                                     std::string(target_crs[0]).c_str(),
                                               FALSE, 0.0, 1 );
  out[i] = gdal_suggest_warp(poSrcDS, pfnTransformerArg);
  if (!(poSrcDS == nullptr)) {
    GDALClose(poSrcDS);
  }
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
                              logicals nara) {

  std::vector<GDALDatasetH> src_ds(dsn.size());

  for (int i = 0; i < dsn.size(); i++) {
    GDALDatasetH hDS = GDALOpen(std::string(dsn[i]).c_str(), GA_ReadOnly);
    if (hDS == nullptr) {
      if (i > 0) {
        for (int j = 0; j < i; j++) GDALClose(src_ds[j]);
      }
      Rprintf("input source not readable: %s\n", std::string(dsn[i]).c_str());
      cpp11::stop("");
    } else {
      src_ds[i] = hDS;
    }
  }

  char** papszArg = nullptr;

  bool write_dsn = false;
  std::string outname = std::string(dsn_outname[0]);
  if (outname == "") {
    papszArg = CSLAddString(papszArg, "-of");
    papszArg = CSLAddString(papszArg, "MEM");
  } else {
    write_dsn = true;
  }

  std::string tcrs = std::string(target_crs[0]);
  if (tcrs != "") {
    OGRSpatialReference oTargetSRS;
    OGRErr target_chk =  oTargetSRS.SetFromUserInput(tcrs.c_str());
    if (target_chk != OGRERR_NONE) cpp11::stop("cannot initialize target projection");
    papszArg = CSLAddString(papszArg, "-t_srs");
    papszArg = CSLAddString(papszArg, tcrs.c_str());
  }

  if (target_extent.size() > 0) {
    double dfMinX = target_extent[0]; double dfMaxX = target_extent[1];
    double dfMinY = target_extent[2]; double dfMaxY = target_extent[3];
    papszArg = CSLAddString(papszArg, "-te");
    papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g", dfMinX));
    papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g", dfMinY));
    papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g", dfMaxX));
    papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g", dfMaxY));
  }

  if (target_dim.size() > 0) {
    int nXSize = target_dim[0]; int nYSize = target_dim[1];
    papszArg = CSLAddString(papszArg, "-ts");
    papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nXSize));
    papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nYSize));
  }

  if (target_res.size() > 0) {
    double XRes = target_res[0]; double YRes = target_res[1];
    if (! (XRes > 0 && YRes > 0)) {
      cpp11::stop("invalid value/s for 'target_res' (not greater than zero)");
    }
    papszArg = CSLAddString(papszArg, "-tr");
    papszArg = CSLAddString(papszArg, CPLSPrintf("%f", XRes));
    papszArg = CSLAddString(papszArg, CPLSPrintf("%f", YRes));
  }
  if (resample.size() > 0) {
    papszArg = CSLAddString(papszArg, "-r");
    papszArg = CSLAddString(papszArg, std::string(resample[0]).c_str());
  }
  for (int gwopt = 0; gwopt < options.size(); gwopt++) {
    if (std::string(options[gwopt]) != "") {
     papszArg = CSLAddString(papszArg, std::string(options[gwopt]).c_str());
    }
  }

  GDALWarpAppOptions* psOptions = GDALWarpAppOptionsNew(papszArg, nullptr);
  CSLDestroy(papszArg);
  GDALWarpAppOptionsSetProgress(psOptions, NULL, NULL );

    GDALDatasetH hRet = GDALWarp(outname.c_str(), nullptr,
                                  static_cast<int>(dsn.size()), src_ds.data(),
                                  psOptions, nullptr);

  GDALWarpAppOptionsFree(psOptions);

  for (int si = 0; si < dsn.size(); si++) {
    GDALClose( src_ds[si] );
  }

  if (hRet == nullptr) {
    cpp11::stop("data source could not be processed with GDALWarp api");
  }

  writable::list outlist;
  if (write_dsn) {
    outlist.push_back(dsn_outname);
  } else {
    int nBands = (int)GDALGetRasterCount(hRet);
    int nbands_to_read = (int)bands.size();
    if (bands[0] < 1) {
       nbands_to_read = nBands;
    }
    std::vector<int> bands_to_read(static_cast<size_t>(nbands_to_read));
    for (int i = 0; i < nbands_to_read; i++) {
      if (bands[0] >= 1) {
        bands_to_read[static_cast<size_t>(i)] = bands[i];
      } else {
        bands_to_read[static_cast<size_t>(i)] = i + 1;
      }
      if (bands_to_read[static_cast<size_t>(i)] > nBands) {
        GDALClose( hRet );
        cpp11::stop("band number is not available: %i", (int)bands[i]);
      }
    }
    writable::logicals unscale(1);
    unscale[0] = TRUE;
    writable::integers window(6);
    for (int i  = 0; i < window.size(); i++) window[i] = 0;

    outlist = writable::list(
      gdalraster::gdal_read_band_values(((GDALDataset*) hRet),
                                                window,
                                                bands_to_read,
                                                band_output_type,
                                                resample,
                                                unscale, nara));
  }

  if ((bool)include_meta[0]) {
  R_xlen_t dimx =  ((GDALDataset*)hRet)->GetRasterXSize();
  R_xlen_t dimy =  ((GDALDataset*)hRet)->GetRasterYSize();
  double        adfGeoTransform[6];
  GDALGetGeoTransform(hRet, adfGeoTransform );
  double xmin = adfGeoTransform[0];
  double xmax = adfGeoTransform[0] + (double)dimx * adfGeoTransform[1];
  double ymin = adfGeoTransform[3] + (double)dimy * adfGeoTransform[5];
  double ymax = adfGeoTransform[3];

  const char *proj;
  proj = GDALGetProjectionRef(hRet);
  writable::doubles dim_attr = {(double)dimx, (double)dimy};
  writable::doubles ext_attr = {xmin, xmax, ymin, ymax};
  writable::strings proj_attr(1);
  proj_attr[0] = proj;
  outlist.attr("dimension") = dim_attr;
  outlist.attr("extent") = ext_attr;
  outlist.attr("projection") = proj_attr;
  }
  GDALClose( hRet );
  return outlist;
}

} // namespace gdalwarpgeneral
#endif
