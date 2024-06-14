#ifndef GDALWARPGENERAL_H
#define GDALWARPGENERAL_H

#include <Rcpp.h>
#include "gdal_priv.h"
#include "gdalwarper.h"
#include "gdal_utils.h"  // for GDALWarpAppOptions
#include "gdalraster/gdalraster.h"
#include "ogr_spatialref.h"  // for OGRCreateCoordinateTransformation
using namespace std;

namespace gdalwarpgeneral{

using namespace Rcpp;


List gdal_suggest_warp(GDALDataset* poSrcDS, void *pfnTransformerArg) {
  double        adfGeoTransform[6];
    poSrcDS->GetGeoTransform( adfGeoTransform );
 
    int nXSize, nYSize;
    double adfExtent[4]; 
    
    GDALTransformerFunc pfnTransformer; 
    pfnTransformer = GDALGenImgProjTransform;
  

  GDALSuggestedWarpOutput2(poSrcDS, pfnTransformer, pfnTransformerArg,
                           adfGeoTransform, &nXSize, &nYSize, adfExtent,
                           0); 
  IntegerVector dimension(2);
  dimension[0] = nXSize;
  dimension[1] = nYSize;
  
  NumericVector extent(4); 
  extent[0] = adfExtent[0]; 
  extent[1] = adfExtent[2];
  extent[2] = adfExtent[1];
  extent[3] = adfExtent[3]; 
  
  List out_i(2); 
  out_i[0] = extent; 
  out_i[1] = dimension; 
  return out_i;
}

List gdal_suggest_warp(CharacterVector dsn, CharacterVector target_crs) {
  List out(dsn.size()); 
  IntegerVector sds0 = IntegerVector::create(0); 
  for (int i = 0; i < dsn.size(); i++) {
    GDALDataset* poSrcDS = (GDALDataset*) gdalraster::gdalH_open_dsn(dsn[0],  sds0);
    //GDALTransformerFunc pfnTransformer; 
    //pfnTransformer = GDALGenImgProjTransform;
  
    void *pfnTransformerArg = nullptr;
    pfnTransformerArg =
     GDALCreateGenImgProjTransformer( poSrcDS,
                                     nullptr,
                                     nullptr,
                                     target_crs[0],
                                               FALSE, 0.0, 1 );
    
  out[i] = gdal_suggest_warp(poSrcDS, pfnTransformerArg);  
  if (!(poSrcDS == nullptr)) {
    GDALClose(poSrcDS); 
  }
  }
  return out; 
}

// remove source_WKT (because VRT)
// remove source_extent (VRT)
//  target_extent must be optional
// target_dim must be optional, 
// expose SuggestedWarpOutput to R
// band_output_type -ot
// resample         -r
// bands            -b n0 -b n1
inline List gdal_warp_general(CharacterVector dsn,
                              CharacterVector target_crs,
                              NumericVector target_extent,
                              IntegerVector target_dim,
                              NumericVector target_res,
                              IntegerVector bands,
                              CharacterVector resample,
                              LogicalVector silent,
                              CharacterVector band_output_type, 
                              CharacterVector options, 
                              CharacterVector dsn_outname, 
                              LogicalVector include_meta, 
                              LogicalVector nara) {
  
  
  ///GDALDatasetH *poSrcDS = nullptr;;
//  poSrcDS = static_cast<GDALDatasetH *>(CPLRealloc(poSrcDS, sizeof(GDALDatasetH) * static_cast<size_t>(dsn.size())));
  
  std::vector<GDALDatasetH> src_ds(dsn.size());
  
  IntegerVector sds0 = IntegerVector::create(0); 
  for (int i = 0; i < dsn.size(); i++) {
    GDALDatasetH hDS = GDALOpen(dsn[i], GA_ReadOnly);
    //src_ds[i] = gdalraster::gdalH_open_dsn(dsn[i],   sds0); 
    // unwind everything, and stop (why not unwind if all are null, message how many succeed)
    if (hDS == nullptr) {
      if (i > 0) {
        for (int j = 0; j < i; j++) GDALClose(src_ds[j]);
      }
      Rprintf("input source not readable: %s\n", (char *)dsn[i]); 
     // CPLFree(poSrcDS);
      Rcpp::stop(""); 
    } else {
      src_ds[i] = hDS; 
    }
  }
  
  // handle warp settings and options
  // we manually handle -r, -te, -t_srs, -ts, -of,
  // but the rest passed in as wo, to, oo, doo, or general (non general ones get -wo/-to/-oo/-doo prepended in R)
  char** papszArg = nullptr;
 
  bool write_dsn = false; 
  if (EQUAL(dsn_outname[0], "")) {
    papszArg = CSLAddString(papszArg, "-of");
    papszArg = CSLAddString(papszArg, "MEM");
    
  } else {

    write_dsn = true; 
  }
  
  if (!target_crs[0].empty()) {
    OGRSpatialReference oTargetSRS;
    //const char * strforuin = (const char *)target_crs[0];
    OGRErr target_chk =  oTargetSRS.SetFromUserInput((const char*)target_crs[0]);
    if (target_chk != OGRERR_NONE) Rcpp::stop("cannot initialize target projection");
  
    papszArg = CSLAddString(papszArg, "-t_srs");
    papszArg = CSLAddString(papszArg, target_crs[0]);
    
  }
  
  if (target_extent.size() > 0) {
    double dfMinX = target_extent[0];
    double dfMaxX = target_extent[1];
    double dfMinY = target_extent[2];
    double dfMaxY = target_extent[3];
    
    papszArg = CSLAddString(papszArg, "-te");
    papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g", dfMinX));
    papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g", dfMinY));
    papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g", dfMaxX));
    papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g", dfMaxY));
  }
 
  if (target_dim.size() > 0) {
    int nXSize = target_dim[0];
    int nYSize = target_dim[1];
    papszArg = CSLAddString(papszArg, "-ts");
    papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nXSize));
    papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nYSize));
  }
 
  if (target_res.size() > 0) {
    double XRes = target_res[0];
    double YRes = target_res[1];
    if (! (XRes > 0 && YRes > 0)) {
      Rcpp::stop("invalid value/s for 'target_res' (not greater than zero)\n");
    } 
    papszArg = CSLAddString(papszArg, "-tr");
    papszArg = CSLAddString(papszArg, CPLSPrintf("%f", XRes));
    papszArg = CSLAddString(papszArg, CPLSPrintf("%f", YRes));
  }
  if (resample.size() > 0) {
    papszArg = CSLAddString(papszArg, "-r");
    papszArg = CSLAddString(papszArg, resample[0]);
  }
  for (int gwopt = 0; gwopt < options.length(); gwopt++) {
    if (!options[gwopt].empty()) {
     papszArg = CSLAddString(papszArg, options[gwopt]);
    }
  }
  
        
  GDALWarpAppOptions* psOptions = GDALWarpAppOptionsNew(papszArg, nullptr);
  
  CSLDestroy(papszArg);
  
  GDALWarpAppOptionsSetProgress(psOptions, NULL, NULL );

    GDALDatasetH hRet = GDALWarp(dsn_outname[0], nullptr,
                                  static_cast<int>(dsn.size()), src_ds.data(),
                                  psOptions, nullptr);
  
  GDALWarpAppOptionsFree(psOptions);
  
  

  for (int si = 0; si < dsn.size(); si++) {
    GDALClose( src_ds[si] );
  }
 // CPLFree(poSrcDS);
  
  if (hRet == nullptr) {
    Rcpp::stop("data source could not be processed with GDALWarp api");
  }
  
  
  List outlist; 
  if (write_dsn) {
    outlist.push_back(dsn_outname); 
    
  } else {
    // Prepare to read bands
    int nBands;
     nBands = (int)GDALGetRasterCount(hRet);
     int nbands_to_read = (int)bands.size();
/// if user set bands to NULL, then all bands read (default is bands = 1L)
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
        stop("band number is not available: %i", bands[i]);
      }
      
    }
    LogicalVector unscale = true;
    IntegerVector window(6);
    // default window with all zeroes results in entire read (for warp)
    // also supports vapour_raster_read  atm
    for (int i  = 0; i < window.size(); i++) window[i] = 0;
    
    
    outlist = gdalraster::gdal_read_band_values(((GDALDataset*) hRet),
                                                window,
                                                bands_to_read,
                                                band_output_type,
                                                resample,
                                                unscale, nara);
  }
  
  
  
  if (include_meta[0]) {
  // shove the grid details on as attributes
  // get the extent ...
  R_xlen_t dimx =  ((GDALDataset*)hRet)->GetRasterXSize(); 
  R_xlen_t dimy =  ((GDALDataset*)hRet)->GetRasterYSize();
  double        adfGeoTransform[6];
  //poDataset->GetGeoTransform( adfGeoTransform );
  GDALGetGeoTransform(hRet, adfGeoTransform );
  double xmin = adfGeoTransform[0];
  double xmax = adfGeoTransform[0] + (double)dimx * adfGeoTransform[1];
  double ymin = adfGeoTransform[3] + (double)dimy * adfGeoTransform[5];
  double ymax = adfGeoTransform[3];
  
  
  const char *proj;
  proj = GDALGetProjectionRef(hRet);
  //https://gis.stackexchange.com/questions/164279/how-do-i-create-ogrspatialreference-from-raster-files-georeference-c
  outlist.attr("dimension") = NumericVector::create(dimx, dimy);
  outlist.attr("extent") = NumericVector::create(xmin, xmax, ymin, ymax);
  outlist.attr("projection") = CharacterVector::create(proj);
  }  
  GDALClose( hRet );
  return outlist;
}

} // namespace gdalwarpgeneral
#endif
