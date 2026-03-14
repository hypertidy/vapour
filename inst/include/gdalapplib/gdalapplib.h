#ifndef GDALAPPLIB_H
#define GDALAPPLIB_H
#include <cpp11.hpp>
#include "gdal_priv.h"
#include "gdal_utils.h"  // for GDALTranslateOptions
#include "vrtdataset.h"
#include "cpl_string.h"
#include "ogr_spatialref.h"  // for OGRCreateCoordinateTransformation

#include "common/common_vapour.h"

#include "gdalwarper.h"




namespace gdalapplib {
using namespace cpp11;
namespace writable = cpp11::writable;


// GDALInfo() itself requires GDAL 2.1
inline strings gdalinfo_applib_cpp(strings dsn,
                                   strings options) {
  
  
  
  //put -json in the input options for this call
  char** papszArg = nullptr;
  for (R_xlen_t i = 0; i < options.size(); ++i) {
    if (std::string(options[0]) != "") {
      papszArg = CSLAddString(papszArg, as_cstr(options[i]));
    }
  }
  GDALInfoOptions* psOptions = GDALInfoOptionsNew(papszArg, nullptr);
  CSLDestroy(papszArg);
  if (psOptions == nullptr) {
    cpp11::stop("creation of GDALInfoOptions failed");
  }
  writable::strings out(dsn.size());
  for (R_xlen_t i = 0; i < out.size(); i++) {
    
    GDALDatasetH hDataset = GDALOpen(as_cstr(dsn[i]), GA_ReadOnly);
    if (hDataset == nullptr) {
      out[i] = NA_STRING;
    } else {
      char *outstr = GDALInfo(hDataset, psOptions);
      out[i] = outstr;
      CPLFree(outstr);
      GDALClose(hDataset);
    }
  }
  GDALInfoOptionsFree(psOptions);
  
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
  
  
  GDALDatasetH *poSrcDS;
  poSrcDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * (size_t) source_filename.size()));
  
  
  for (int i = 0; i < source_filename.size(); i++) {
    poSrcDS[i] = GDALOpen(as_cstr(source_filename[i]), GA_ReadOnly);
    
    // unwind everything, and stop
    if (poSrcDS[i] == nullptr) {
      if (i > 0) {
        for (int j = 0; j < i; j++) GDALClose(poSrcDS[j]);
      }
      Rprintf("input source not readable: %s\n", as_cstr(source_filename[i]));
      
      CPLFree(poSrcDS);
      cpp11::stop("");
    }
  }
  
  // handle warp settings and options
  char** papszArg = nullptr;
  
  papszArg = CSLAddString(papszArg, "-of");
  if (target_filename.size() < 1) {
    papszArg = CSLAddString(papszArg, "MEM");
  } else {
    papszArg = CSLAddString(papszArg, "COG");
  }
  
  // if we don't supply it don't try to set it!
  if (std::string(target_crs[0]) != ""){
    // if supplied check that it's valid
    OGRSpatialReference *oTargetSRS = nullptr;
    oTargetSRS = new OGRSpatialReference;
    OGRErr target_chk =  oTargetSRS->SetFromUserInput(as_cstr(target_crs[0]));
    if (oTargetSRS != nullptr) {
      delete oTargetSRS;
    }
    if (target_chk != OGRERR_NONE) {
      cpp11::stop("cannot initialize target projection");
    }
    const char *st = NULL;
    st = ((GDALDataset *)poSrcDS[0])->GetProjectionRef();
    
    if(*st == '\0')	{
      cpp11::stop( "Transformation to this target CRS not possible from this source dataset, target CRS given: \n\n %s \n\n",
                   as_cstr(target_crs[0]) );
    }
    
    papszArg = CSLAddString(papszArg, "-t_srs");
    papszArg = CSLAddString(papszArg, as_cstr(target_crs[0]));
    
  }
  
  // we always provide extent and dimension, crs is optional and just means subset/decimate
  double dfMinX = target_extent[0];
  double dfMaxX = target_extent[1];
  double dfMinY = target_extent[2];
  double dfMaxY = target_extent[3];
  
  papszArg = CSLAddString(papszArg, "-te");
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g", dfMinX));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g", dfMinY));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g", dfMaxX));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g", dfMaxY));
  
  // we otherwise set a dud dimension, the user didn't set it (so they get native for the extent)
  if (target_dim.size() > 1) {
    int nXSize = target_dim[0];
    int nYSize = target_dim[1];
    papszArg = CSLAddString(papszArg, "-ts");
    papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nXSize));
    papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nYSize));
  }
  
  
  papszArg = CSLAddString(papszArg, "-r");
  papszArg = CSLAddString(papszArg, as_cstr(resample[0]));
  
  papszArg = CSLAddString(papszArg, "-multi");
  
  
  // bundle on all user-added options
  for (int wopt = 0; wopt < warp_options.size(); wopt++) {
    papszArg = CSLAddString(papszArg, "-wo");
    papszArg = CSLAddString(papszArg, as_cstr(warp_options[wopt]));
  }
  for (int topt = 0; topt < transformation_options.size(); topt++) {
    papszArg = CSLAddString(papszArg, "-to");
    papszArg = CSLAddString(papszArg, as_cstr(transformation_options[topt]));
  }
  
  
  
  auto psOptions = GDALWarpAppOptionsNew(papszArg, nullptr);
  CSLDestroy(papszArg);
  GDALWarpAppOptionsSetProgress(psOptions, NULL, NULL );
  
  GDALDatasetH hRet = GDALWarp( as_cstr(target_filename[0]), nullptr,
                                (int)source_filename.size(), poSrcDS,
                                psOptions, nullptr);
  
  
  CPLAssert( hRet != NULL );
  GDALWarpAppOptionsFree(psOptions);
  
  GDALClose((GDALDataset*) hRet);
  for (int si = 0; si < source_filename.size(); si++) {
    GDALClose( (GDALDataset *)poSrcDS[si] );
  }
  CPLFree(poSrcDS);
  
  writable::list out;
  return out;
}

inline strings gdalbuildvrt_applib(std::vector<std::string> dsn,
                                   std::vector<std::string> options) {
  writable::strings out(1);
  int err;
  GDALBuildVRTOptions* opt = GDALBuildVRTOptionsNew(string_to_charptr(options).data(), nullptr);
  int dsn_size = (int)dsn.size();
  GDALDataset *vrt = (GDALDataset*)GDALBuildVRT("", dsn_size, nullptr, string_to_charptr(dsn).data(), opt, &err);
  out[0] = vrt->GetMetadata("xml:VRT")[0];
  GDALClose(vrt);
  return out;
}


} //  namespace gdalapplib
#endif
