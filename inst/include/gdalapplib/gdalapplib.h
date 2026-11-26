#ifndef GDALAPPLIB_H
#define GDALAPPLIB_H
#include <cpp11.hpp>
#include "gdal_priv.h"
#include "gdal_utils.h"  // for GDALTranslateOptions
#include "vrtdataset.h"
#include "cpl_string.h"
#include "ogr_spatialref.h"  // for OGRCreateCoordinateTransformation

#include "common/common_vapour.h"
#include "common/gdal_handles.h"
#include "gdalraster/gdalraster.h"

#include "gdalwarper.h"




namespace gdalapplib {
using namespace cpp11;
namespace writable = cpp11::writable;


// GDALInfo() itself requires GDAL 2.1
// One string of gdalinfo output for each source, NA for a source that cannot
// be opened. 'sds' (from 1) reports on that subdataset of each source.
inline strings gdalinfo_applib_cpp(strings dsn,
                                   strings options,
                                   integers sds,
                                   strings open_options) {
  CPLStringList argv = gdalcommon::as_string_list(options);
  gdalcommon::InfoOptionsPtr psOptions(GDALInfoOptionsNew(argv.List(), nullptr));
  if (!psOptions) gdalcommon::stop_gdal("invalid gdalinfo options");

  writable::strings out(dsn.size());
  for (R_xlen_t i = 0; i < out.size(); i++) {
    out[i] = NA_STRING;
    if (STRING_ELT(dsn, i) == NA_STRING) continue;
    gdalcommon::DatasetPtr hDataset = gdalraster::open_raster(as_cstr(dsn[i]), sds, open_options);
    if (hDataset) {
      gdalcommon::CPLStringPtr outstr(GDALInfo((GDALDatasetH) hDataset.get(), psOptions.get()));
      if (outstr) out[i] = outstr.get();
    }
  }
  return out;
}




inline list gdalwarp_applib(strings source_filename,
                            strings target_crs,
                            doubles target_extent,
                            integers target_dim,
                            strings target_filename,
                            integers bands,
                            strings resample,
                            logicals silent,
                            strings band_output_type,
                            strings warp_options,
                            strings transformation_options) {

  if (source_filename.size() < 1) cpp11::stop("no input sources");
  if (target_extent.size() != 4) cpp11::stop("'target_extent' must have 4 values: xmin, xmax, ymin, ymax");
  if (target_filename.size() < 1) cpp11::stop("'target_filename' is needed");

  std::vector<gdalcommon::DatasetPtr> src_owner;
  std::vector<GDALDatasetH> poSrcDS;
  writable::integers sds0 = {0};
  writable::strings no_options((R_xlen_t)0);
  for (int i = 0; i < source_filename.size(); i++) {
    gdalcommon::DatasetPtr ds = gdalraster::open_raster(as_cstr(source_filename[i]), sds0, no_options);
    if (!ds) {
      gdalcommon::stop_gdal(std::string("input source not readable: '") + as_cstr(source_filename[i]) + "'");
    }
    poSrcDS.push_back((GDALDatasetH) ds.get());
    src_owner.push_back(std::move(ds));
  }

  // handle warp settings and options
  CPLStringList argv;
  argv.AddString("-of");
  argv.AddString("COG");

  // if we don't supply it don't try to set it!
  if (std::string(target_crs[0]) != ""){
    // if supplied check that it's valid
    OGRSpatialReference oTargetSRS;
    OGRErr target_chk =  oTargetSRS.SetFromUserInput(as_cstr(target_crs[0]));
    if (target_chk != OGRERR_NONE) {
      gdalcommon::stop_gdal("cannot initialize target projection");
    }
    const char *st = src_owner[0]->GetProjectionRef();
    if (st == nullptr || *st == '\0')	{
      cpp11::stop( "Transformation to this target CRS not possible from this source dataset, target CRS given: \n\n %s \n\n",
                   as_cstr(target_crs[0]) );
    }

    argv.AddString("-t_srs");
    argv.AddString(as_cstr(target_crs[0]));
  }

  // we always provide extent and dimension, crs is optional and just means subset/decimate
  double dfMinX = target_extent[0];
  double dfMaxX = target_extent[1];
  double dfMinY = target_extent[2];
  double dfMaxY = target_extent[3];

  argv.AddString("-te");
  argv.AddString(CPLSPrintf("%.18g", dfMinX));
  argv.AddString(CPLSPrintf("%.18g", dfMinY));
  argv.AddString(CPLSPrintf("%.18g", dfMaxX));
  argv.AddString(CPLSPrintf("%.18g", dfMaxY));

  // we otherwise set a dud dimension, the user didn't set it (so they get native for the extent)
  if (target_dim.size() > 1) {
    int nXSize = target_dim[0];
    int nYSize = target_dim[1];
    argv.AddString("-ts");
    argv.AddString(CPLSPrintf("%d", nXSize));
    argv.AddString(CPLSPrintf("%d", nYSize));
  }

  if (resample.size() > 0) {
    argv.AddString("-r");
    argv.AddString(as_cstr(resample[0]));
  }

  argv.AddString("-multi");

  // bundle on all user-added options
  for (int wopt = 0; wopt < warp_options.size(); wopt++) {
    argv.AddString("-wo");
    argv.AddString(as_cstr(warp_options[wopt]));
  }
  for (int topt = 0; topt < transformation_options.size(); topt++) {
    argv.AddString("-to");
    argv.AddString(as_cstr(transformation_options[topt]));
  }

  gdalcommon::WarpAppOptionsPtr psOptions(GDALWarpAppOptionsNew(argv.List(), nullptr));
  if (!psOptions) gdalcommon::stop_gdal("invalid warp options");
  GDALWarpAppOptionsSetProgress(psOptions.get(), NULL, NULL );

  gdalcommon::DatasetPtr hRet((GDALDataset *) GDALWarp( as_cstr(target_filename[0]), nullptr,
                                (int)poSrcDS.size(), poSrcDS.data(),
                                psOptions.get(), nullptr));
  if (!hRet) {
    gdalcommon::stop_gdal(std::string("warp to '") + as_cstr(target_filename[0]) + "' failed");
  }

  writable::list out;
  return out;
}

inline strings gdalbuildvrt_applib(std::vector<std::string> dsn,
                                   std::vector<std::string> options) {
  writable::strings out(1);
  int err = 0;
  // the strings in 'options' and 'dsn' outlive these arrays of pointers to them
  std::vector<char *> options_ptr = string_to_charptr(options);
  std::vector<char *> dsn_ptr = string_to_charptr(dsn);
  gdalcommon::BuildVRTOptionsPtr opt(GDALBuildVRTOptionsNew(options_ptr.data(), nullptr));
  if (!opt) gdalcommon::stop_gdal("invalid buildvrt options");
  int dsn_size = (int)dsn.size();
  gdalcommon::DatasetPtr vrt((GDALDataset*)GDALBuildVRT("", dsn_size, nullptr, dsn_ptr.data(), opt.get(), &err));
  if (!vrt) gdalcommon::stop_gdal("unable to build VRT from input sources");
  CSLConstList xml = vrt->GetMetadata("xml:VRT");
  if (xml == nullptr || xml[0] == nullptr) cpp11::stop("unable to build VRT from input sources");
  out[0] = xml[0];
  return out;
}


} //  namespace gdalapplib
#endif
