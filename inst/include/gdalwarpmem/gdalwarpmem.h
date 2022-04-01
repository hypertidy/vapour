#ifndef GDALWARPMEM_H
#define GDALWARPMEM_H

#include <Rcpp.h>
#include "gdal_priv.h"
#include "gdalwarper.h"
#include "gdal_utils.h"  // for GDALWarpAppOptions
#include "gdallibrary/gdallibrary.h"
#include "gdalraster/gdalraster.h"
namespace gdalwarpmem{

using namespace Rcpp;

inline List gdal_warp_in_memory(CharacterVector source_filename,
                                CharacterVector source_WKT,
                                CharacterVector target_WKT,
                                NumericVector target_extent,
                                IntegerVector target_dim,
                                IntegerVector bands,
                                NumericVector source_extent,
                                CharacterVector resample,
                                LogicalVector silent,
                                CharacterVector band_output_type, 
                                CharacterVector warp_options, 
                                CharacterVector transformation_options) {
  
  
  
  GDALDatasetH *poSrcDS;
  GDALAllRegister();
  poSrcDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * source_filename.size()));
  
  Rprintf("before source: %s\n", source_WKT[0]);
      
  if ( !source_WKT[0].empty()) {
    if (silent[0] != true) Rprintf("setting projection");
    // if supplied check that it's valid
    OGRSpatialReference* oSourceSRS = nullptr;
    oSourceSRS = new OGRSpatialReference;
    OGRErr source_chk =  oSourceSRS->SetFromUserInput((char *)source_WKT[0]);
    delete oSourceSRS;
    if (source_chk != OGRERR_NONE) Rcpp::stop("cannot initialize source projection: %s\n", (char *)source_WKT[0]);
  }
  
  Rprintf("after source: %s\n", source_WKT[0]);
  
  char** papszArg = nullptr;
  int dsi0 = 0;
  for (int si = 0; si < source_filename.size(); si++) {
    
    poSrcDS[si] = gdalraster::gdal_dataset_augment((char *)source_filename[si], source_extent, source_WKT, 1);
    
    if (poSrcDS[si] == NULL) {
      Rprintf("cannot open %s\n", (char *)source_filename[si]);
      
      if (si > 0) {
        for (int ii = 0; ii < si; ii++) {
          GDALClose( poSrcDS[ii] );
        }
      }
      Rprintf("failed to open\n");
      Rcpp::stop("\n");
    }
    
    // https://github.com/OSGeo/gdal/blob/fec15b146f8a750c23c5e765cac12ed5fc9c2b85/gdal/frmts/gtiff/cogdriver.cpp#L512
    papszArg = CSLAddString(papszArg, "-of");
    papszArg = CSLAddString(papszArg, "MEM");
    
    // if we don't supply it don't try to set it!
    Rprintf("skip target\n");
    if (false && !target_WKT[0].empty()){
      // if supplied check that it's valid
      OGRSpatialReference* oTargetSRS = nullptr;
      oTargetSRS = new OGRSpatialReference;
      OGRErr target_chk =  oTargetSRS->SetFromUserInput(target_WKT[0]);
      if (target_chk != OGRERR_NONE) Rcpp::stop("cannot initialize target projection");
      delete oTargetSRS;
      papszArg = CSLAddString(papszArg, "-t_srs");
      papszArg = CSLAddString(papszArg, target_WKT[0]);
    }
    
  } // int si
  
  // we always provide extent and dimension, crs is optional and just means subset/decimate
  double dfMinX = target_extent[0];
  double dfMaxX = target_extent[1];
  double dfMinY = target_extent[2];
  double dfMaxY = target_extent[3];
  
  papszArg = CSLAddString(papszArg, "-te");
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMinX));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMinY));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMaxX));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMaxY));
  
  // we otherwise set a dud dimension, the user didn't set it (so they get native for the extent)
  if (target_dim.size() > 1) {
    int nXSize = target_dim[0];
    int nYSize = target_dim[1];
    papszArg = CSLAddString(papszArg, "-ts");
    papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nXSize));
    papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nYSize));
  }
  
  //const auto poFirstBand = GDALGetRasterBand(poSrcDS[dsi0], 1);
  
  papszArg = CSLAddString(papszArg, "-r");
  papszArg = CSLAddString(papszArg, resample[0]);
  
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
  auto hRet = GDALWarp( "", nullptr,
                        source_filename.size(), poSrcDS,
                        psOptions, nullptr);
  CPLAssert( hRet != NULL );
  GDALWarpAppOptionsFree(psOptions);
  for (int si = 0; si < source_filename.size(); si++) {
        GDALClose( poSrcDS[si] );
  }
  CPLFree(poSrcDS);
      
  
  const int nBands = GDALGetRasterCount(hRet);// GDALGetRasterCount(poSrcDS[dsi0]);
  int band_length = bands.size();
  if (bands.size() == 1 && bands[0] == 0) {
    band_length = nBands;
  }
  
  std::vector<int> bands_to_read(band_length);
  if (bands.size() == 1 && bands[0] == 0) {
    for (int i = 0; i < nBands; i++) bands_to_read[i] = i + 1;
    // Rprintf("index bands\n");
  } else {
    for (int i = 0; i < bands.size(); i++) {
      // clean this up, we need to identify which bands can't be read earlier or drop them
      GDALClose( hRet );
      if (bands[i] > nBands) stop("cannot read band %i, there are only (%i) bands\n", bands[i], nBands);
      bands_to_read[i] = bands[i];
    }
    // Rprintf("input bands\n");
  }
  LogicalVector unscale = true;
  IntegerVector window(6);
  // default window with all zeroes results in entire read (for warp)
  // also supports vapour_raster_read  atm
  for (int i  = 0; i < window.size(); i++) window[i] = 0;
  List outlist = gdallibrary::gdal_read_band_values(GDALDataset::FromHandle(hRet),
                                                    window,
                                                    bands_to_read,
                                                    band_output_type,
                                                    resample,
                                                    unscale);
  GDALClose( hRet );
  return outlist;
}

} // namespace gdalwarpmem
#endif
