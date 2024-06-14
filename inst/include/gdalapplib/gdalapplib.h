#ifndef GDALAPPLIB_H
#define GDALAPPLIB_H
#include <Rcpp.h>
#include "gdal_priv.h"
#include "gdal_utils.h"  // for GDALTranslateOptions
#include "vrtdataset.h"
#include "cpl_string.h"
#include "ogr_spatialref.h"  // for OGRCreateCoordinateTransformation

#include "common/common_vapour.h"

#include "gdalwarper.h"
//#include "gdal_utils.h"  // for GDALWarpAppOptions




namespace gdalapplib {
using namespace Rcpp;


// GDALInfo() itself requires GDAL 2.1
//
// Note: doesn't sf do this? yes, but not vectorized in cpp (not slow though), not callable from Cpp, doesn't work with subdatasets (that is is gdalinfo_bin.cpp)
// [[Rcpp::export]]
inline CharacterVector gdalinfo_applib_cpp(CharacterVector dsn,
                              CharacterVector options) {
  
  
  
//put -json in the input options for this call
 //Rcpp::CharacterVector argv = {"-json", "-stats", "-hist"};
 char** papszArg = nullptr;
for (R_xlen_t i = 0; i < options.size(); ++i) {
  if (!options[0].empty()) {
    papszArg = CSLAddString(papszArg, options[i]);
  }
}
  GDALInfoOptions* psOptions = GDALInfoOptionsNew(papszArg, nullptr);
  CSLDestroy(papszArg); 
  if (psOptions == nullptr) {
    Rcpp::stop("creation of GDALInfoOptions failed");
  }
  CharacterVector out(dsn.size()); 
  for (R_xlen_t i = 0; i < out.size(); i++) {
  
    GDALDatasetH hDataset = GDALOpen(dsn[i], GA_ReadOnly); 
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









inline List gdalwarp_applib(CharacterVector source_filename,
                                CharacterVector target_crs,
                                NumericVector target_extent,
                                IntegerVector target_dim,
                                CharacterVector target_filename,
                                IntegerVector bands,
                                CharacterVector resample,
                                LogicalVector silent,
                                CharacterVector band_output_type, 
                                CharacterVector warp_options, 
                                CharacterVector transformation_options) {
  
  
  GDALDatasetH *poSrcDS;
  poSrcDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * (size_t) source_filename.size()));
  
  
  for (int i = 0; i < source_filename.size(); i++) {
     poSrcDS[i] = GDALOpen(source_filename[i], GA_ReadOnly); 
      
    // unwind everything, and stop
    if (poSrcDS[i] == nullptr) {
      if (i > 0) {
        for (int j = 0; j < i; j++) GDALClose(poSrcDS[j]);
      }
      Rprintf("input source not readable: %s\n", (char *)source_filename[i]); 
      
      CPLFree(poSrcDS);
      Rcpp::stop(""); 
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
  if (!target_crs[0].empty()){
    // if supplied check that it's valid
    OGRSpatialReference *oTargetSRS = nullptr;
    oTargetSRS = new OGRSpatialReference;
    OGRErr target_chk =  oTargetSRS->SetFromUserInput((const char*)target_crs[0]);
    if (oTargetSRS != nullptr) {
      delete oTargetSRS; 
    }
    if (target_chk != OGRERR_NONE) {
      Rcpp::stop("cannot initialize target projection");
    }
    const char *st = NULL;
    st = ((GDALDataset *)poSrcDS[0])->GetProjectionRef(); 
    
    if(*st == '\0')	{
      Rcpp::stop( "Transformation to this target CRS not possible from this source dataset, target CRS given: \n\n %s \n\n", 
                  (char *)  target_crs[0] );
    }

    papszArg = CSLAddString(papszArg, "-t_srs");
  papszArg = CSLAddString(papszArg, target_crs[0]);
    
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
  papszArg = CSLAddString(papszArg, resample[0]);
  
  papszArg = CSLAddString(papszArg, "-multi");
  //papszArg = CSLAddString(papszArg, "-wo");
  //  papszArg = CSLAddString(papszArg, "NUM_THREADS=ALL_CPUS");  
  

  // bundle on all user-added options
  for (int wopt = 0; wopt < warp_options.length(); wopt++) {
    papszArg = CSLAddString(papszArg, "-wo");
    papszArg = CSLAddString(papszArg, warp_options[wopt]);
  }
  for (int topt = 0; topt < transformation_options.length(); topt++) {
    papszArg = CSLAddString(papszArg, "-to");
    papszArg = CSLAddString(papszArg, transformation_options[topt]);
  }


  
  auto psOptions = GDALWarpAppOptionsNew(papszArg, nullptr);
  CSLDestroy(papszArg);
  GDALWarpAppOptionsSetProgress(psOptions, NULL, NULL );
 
  GDALDatasetH hRet = GDALWarp( target_filename[0], nullptr,
                                (int)source_filename.size(), poSrcDS,
                                psOptions, nullptr);
  
  
  CPLAssert( hRet != NULL );
  GDALWarpAppOptionsFree(psOptions);

  GDALClose((GDALDataset*) hRet);
  for (int si = 0; si < source_filename.size(); si++) {
    GDALClose( (GDALDataset *)poSrcDS[si] );
  }
  CPLFree(poSrcDS);
  
  List out(0);
  return out;
}

inline CharacterVector gdalbuildvrt_applib(std::vector<std::string> dsn, 
                                           std::vector<std::string> options) {
  CharacterVector out(1); 
  int err;
  GDALBuildVRTOptions* opt = GDALBuildVRTOptionsNew(string_to_charptr(options).data(), nullptr);
  GDALDataset *vrt = (GDALDataset*)GDALBuildVRT("", dsn.size(), nullptr, string_to_charptr(dsn).data(), opt, &err); 
  out[0] = vrt->GetMetadata("xml:VRT")[0];
  GDALClose(vrt); 
  return out;
}


} //  namespace gdalapplib
#endif
