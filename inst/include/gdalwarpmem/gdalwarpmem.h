#ifndef GDALWARPMEM_H
#define GDALWARPMEM_H

#include <Rcpp.h>
#include "gdal_priv.h"
#include "gdalwarper.h"
#include "gdal_utils.h"  // for GDALWarpAppOptions
#include "gdalraster/gdalraster.h"
#include "ogr_spatialref.h"  // for OGRCreateCoordinateTransformation
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
                                //CharacterVector warp_options, 
                                //CharacterVector transformation_options, 
                                //CharacterVector open_options, 
                                // CharacterVector output_dataset_options,
                                CharacterVector options) {
  
  
  GDALDatasetH *poSrcDS;
  poSrcDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * static_cast<size_t>(source_filename.size())));
  bool augment;  
  augment = !source_WKT[0].empty() || source_extent.size() == 4;
  
  
  for (int i = 0; i < source_filename.size(); i++) {
    if (augment) {
      // not dealing with subdatasets here atm
      // not dealing with source bands here, bands applies at read beloew
      poSrcDS[i] = gdalraster::gdalH_open_avrt(source_filename[i],   source_extent, source_WKT, 0, 0, "");
    } else {
      poSrcDS[i] = gdalraster::gdalH_open_dsn(source_filename[i],   0); 
      
    }
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
  // we manually handle -r, -te, -t_srs, -ts, -of,
  // but the rest passed in as wo, to, oo, doo, or general (non general ones get -wo/-to/-oo/-doo prepended in R)
  char** papszArg = nullptr;
  
  papszArg = CSLAddString(papszArg, "-of");
  papszArg = CSLAddString(papszArg, "MEM");
  
  if (!target_WKT[0].empty()) {
    OGRSpatialReference *oTargetSRS = nullptr;
    oTargetSRS = new OGRSpatialReference;
    const char * strforuin = (const char *)target_WKT[0];
    OGRErr target_chk =  oTargetSRS->SetFromUserInput(strforuin);
    if (target_chk != OGRERR_NONE) Rcpp::stop("cannot initialize target projection");
    OGRSpatialReference  oSRS;
    const OGRSpatialReference* poSrcSRS = ((GDALDataset *)poSrcDS[0])->GetSpatialRef();
    if( poSrcSRS ) {
      oSRS = *poSrcSRS;
      if (!oSRS.IsEmpty()) {
        OGRCoordinateTransformation *poCT;
        poCT = OGRCreateCoordinateTransformation(&oSRS, oTargetSRS);
        if( poCT == nullptr )	{
          delete oTargetSRS;
          Rcpp::stop( "Transformation to this target CRS not possible from this source dataset, target CRS given: \n\n %s \n\n", 
                      (char *)  target_WKT[0] );
        }
        // we add our target projection iff a) source crs is valid b) target crs is valid c) transformation source->target is valid
        // user may have augmented the array of datasets with source_projection
        // if the source is just not defined we ignore the target with a warning
        papszArg = CSLAddString(papszArg, "-t_srs");
        papszArg = CSLAddString(papszArg, target_WKT[0]);
      } else {
        Rcpp::warning("no source crs, target crs is ignored\n");
      }
    }
    delete oTargetSRS;
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
  
  for (int gwopt = 0; gwopt < options.length(); gwopt++) {
    papszArg = CSLAddString(papszArg, options[gwopt]);
  }
  auto psOptions = GDALWarpAppOptionsNew(papszArg, nullptr);
  CSLDestroy(papszArg);
  GDALWarpAppOptionsSetProgress(psOptions, NULL, NULL );
  
  GDALDatasetH hRet = GDALWarp( "", nullptr,
                                static_cast<int>(source_filename.size()), poSrcDS,
                                psOptions, nullptr);
  
  CPLAssert( hRet != NULL );
  GDALWarpAppOptionsFree(psOptions);
  for (int si = 0; si < source_filename.size(); si++) {
    GDALClose( (GDALDataset *)poSrcDS[si] );
  }
  CPLFree(poSrcDS);
  
  if (hRet == nullptr) {
    Rcpp::stop("something went wrong!");
  }
  
  
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
  int nBands;
  if (bands[0] == 0) {
    nBands = (int)GDALGetRasterCount(hRet);
  } else {
    nBands = (int)bands.size();
  }
  std::vector<int> bands_to_read(static_cast<size_t>(nBands));
  for (int i = 0; i < nBands; i++) {
    if (bands[0] == 0) {
      bands_to_read[static_cast<size_t>(i)] = i + 1;
    } else {
      bands_to_read[static_cast<size_t>(i)] = bands[i];
    }
    
    if (bands_to_read[static_cast<size_t>(i)] > (int)GDALGetRasterCount(hRet)) {
      GDALClose( hRet );
      stop("band number is not available: %i", bands_to_read[static_cast<size_t>(i)]);
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
  
  List outlist = gdalraster::gdal_read_band_values(GDALDataset::FromHandle(hRet),
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
