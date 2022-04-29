#ifndef GDALWARPMEM_H
#define GDALWARPMEM_H

#include <Rcpp.h>
#include "gdal_priv.h"
#include "gdalwarper.h"
#include "gdal_utils.h"  // for GDALWarpAppOptions
#include "gdallibrary/gdallibrary.h"
#include "gdalraster/gdalraster.h"

using namespace std;

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
  poSrcDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * source_filename.size()));
  
  for (int i = 0; i < source_filename.size(); i++) {
    if (!source_WKT[0].empty() || source_extent.size() == 4 || bands[0] > 0) {
      // not dealing with subdatasets here atm
      poSrcDS[i] = gdalraster::gdalH_open_avrt(source_filename[i],   source_extent, source_WKT, 0, bands);
    } else {
      poSrcDS[i] = gdalraster::gdalH_open_dsn(source_filename[i],   0); 
    }
    // unwind everything, and stop
    if (poSrcDS[i] == nullptr) {
      if (i > 0) {
        for (int j = 0; j < i; j++) GDALClose((GDALDataset *)poSrcDS[j]);
      }
      Rprintf("input source not readable: %s\n", (char *)source_filename[i]);
      Rcpp::stop(""); 
    }
  }
  
  // handle warp settings and options
  char** papszArg = nullptr;
  
  papszArg = CSLAddString(papszArg, "-of");
  // if (band_output_type[0] == "vrt") {
  //   papszArg = CSLAddString(papszArg, "VRT");
  // } else {
  papszArg = CSLAddString(papszArg, "MEM");
    
   
  // if we don't supply it don't try to set it!
  if (!target_WKT[0].empty()){
    // if supplied check that it's valid
    OGRSpatialReference* oTargetSRS = nullptr;
    oTargetSRS = new OGRSpatialReference;
    OGRErr target_chk =  oTargetSRS->SetFromUserInput(target_WKT[0]);
    if (target_chk != OGRERR_NONE) Rcpp::stop("cannot initialize target projection");
    delete oTargetSRS;
    papszArg = CSLAddString(papszArg, "-t_srs");
    papszArg = CSLAddString(papszArg, target_WKT[0]);
  }
  
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
  
 GDALDatasetH hRet = GDALWarp( "", nullptr,
                        source_filename.size(), poSrcDS,
                        psOptions, nullptr);
  
  
  CPLAssert( hRet != NULL );
  GDALWarpAppOptionsFree(psOptions);
  for (int si = 0; si < source_filename.size(); si++) {
    GDALClose( (GDALDataset *)poSrcDS[si] );
  }
  CPLFree(poSrcDS);

  
  
  /// this doesn't work because we don't keep the file name/s
  if (band_output_type[0] == "vrt") {
    // GDALDriver * vDriver = (GDALDriver *)GDALGetDriverByName("VRT");
    // GDALDatasetH VRTDS = vDriver->CreateCopy("", (GDALDataset *) hRet, 0, nullptr, nullptr, nullptr);
    //


    CharacterVector oof(1);
    CharacterVector infile(1);
    LogicalVector filein(1);
    oof[0] = gdalraster::gdal_vrt_text((GDALDataset *) hRet);
    GDALClose(hRet);
    return List::create(oof);
  }
  
  
  // Prepare to read bands
  const int nBands = GDALGetRasterCount(hRet);// GDALGetRasterCount(poSrcDS[dsi0]);
  int band_length = bands.size();
  if (bands.size() == 1 && bands[0] == 0) {
    band_length = nBands;
  }
 
  std::vector<int> bands_to_read(band_length);
  if (bands.size() == 1 && bands[0] == 0) {
    for (int i = 0; i < nBands; i++) bands_to_read[i] = i + 1;
    
  } else {
    for (int i = 0; i < bands.size(); i++) {
      // clean this up, we need to identify which bands can't be read earlier or drop them
      if (bands[i] > nBands) stop("cannot read band %i, there are only (%i) bands\n", bands[i], nBands);
      bands_to_read[i] = bands[i];
    }
  }
  LogicalVector unscale = true;
  IntegerVector window(6);
  // default window with all zeroes results in entire read (for warp)
  // also supports vapour_raster_read  atm
  for (int i  = 0; i < window.size(); i++) window[i] = 0;
  
  
 // we can't do  gdal_warped_vrt here ... was just trying something and realized 
 // that cant work
 // 
 // GDALRasterIOExtraArg psExtraArg;
 // psExtraArg = gdallibrary::init_resample_alg(resample);
 // 
 // std::vector<double> values( GDALGetRasterXSize(hRet) * GDALGetRasterYSize(hRet) * nBands );
 // CPLErr err = 
 // GDALDataset::FromHandle(hRet)->RasterIO(GF_Read, 0, 0, GDALGetRasterXSize(hRet), GDALGetRasterYSize(hRet),
 //                         &values[0],   GDALGetRasterXSize(hRet), GDALGetRasterYSize(hRet), GDT_Float64,
 //                         nBands, &bands_to_read[0],
 //                         0, 0, 0, &psExtraArg);
 
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
