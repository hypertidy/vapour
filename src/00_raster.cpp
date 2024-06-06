#include <Rcpp.h>
#include "gdallibrary/gdallibrary.h"
#include "gdalwarpmem/gdalwarpmem.h"

using namespace Rcpp;

// [[Rcpp::export]]
List raster_gcp_gdal_cpp(CharacterVector dsn) {
  return gdalraster::gdal_raster_gcp(dsn);
}
// [[Rcpp::export]]
LogicalVector raster_has_geolocation_gdal_cpp(CharacterVector dsn, IntegerVector sds) {
  return gdalraster::gdal_has_geolocation(dsn, sds);
}
// [[Rcpp::export]]
List raster_list_geolocation_gdal_cpp(CharacterVector dsn, IntegerVector sds) {
  return gdalraster::gdal_list_geolocation(dsn, sds);
}

// [[Rcpp::export]]
List raster_info_gdal_cpp(CharacterVector dsn, LogicalVector min_max) {
  return gdalraster::gdal_raster_info(dsn, min_max);
}

// [[Rcpp::export]]
NumericVector raster_extent_cpp(CharacterVector dsn) {
  return gdalraster::gdal_extent_only(dsn); 
}

// [[Rcpp::export]]
List raster_io_gdal_cpp(CharacterVector dsn,
                        IntegerVector window,
                        IntegerVector band,
                        CharacterVector resample,
                        CharacterVector band_output_type, 
                        LogicalVector unscale, 
                        LogicalVector nara) {
  return gdalraster::gdal_raster_io(dsn, window, band, resample, band_output_type, unscale, nara);
}

// [[Rcpp::export]]
CharacterVector sds_list_gdal_cpp(CharacterVector dsn) {
  return gdalraster::gdal_sds_list(dsn[0]);
}

// [[Rcpp::export]]
List sds_list_list_gdal_cpp(CharacterVector dsn) {
  List outlist(dsn.length()); 
  for (int i = 0; i < dsn.length(); i++) {
    outlist[i] = gdalraster::gdal_sds_list(dsn[i]);
  }
  return outlist; 
}


// // [[Rcpp::export]]
// List warp_in_memory_gdal_cpp(CharacterVector dsn,
//                              CharacterVector source_WKT,
//                              CharacterVector target_WKT,
//                              NumericVector target_extent,
//                              IntegerVector target_dim,
//                              IntegerVector bands,
//                              NumericVector source_extent,
//                              CharacterVector resample,
//                              LogicalVector silent,
//                              CharacterVector band_output_type, 
//                      
//                              CharacterVector options, 
//                              LogicalVector nomd, IntegerVector overview, 
//                              LogicalVector nara) {
//   return gdalwarpmem::gdal_warp_in_memory(dsn,
//                                           source_WKT,
//                                           target_WKT,
//                                           target_extent,
//                                           target_dim,
//                                           bands,
//                                           source_extent,
//                                           resample,
//                                           silent,
//                                           band_output_type, 
//                                         options, nomd, overview, nara);
// }
