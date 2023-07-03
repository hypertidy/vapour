#include <Rcpp.h>
#include "gdallibrary/gdallibrary.h"
#include "gdalwarpgeneral/gdalwarpgeneral.h"

using namespace Rcpp;

// [[Rcpp::export]]
List warp_general_cpp(CharacterVector dsn,
                      CharacterVector target_crs,
                      NumericVector target_extent,
                      IntegerVector target_dim,
                      NumericVector target_res,
                      IntegerVector bands,
                      CharacterVector resample,
                      LogicalVector silent,
                      CharacterVector band_output_type, 
                      CharacterVector options, 
                      CharacterVector dsn_outname) {
  return gdalwarpgeneral::gdal_warp_general(dsn,
                                          target_crs, 
                                          target_extent, 
                                          target_dim, 
                                          target_res,
                                          bands, 
                                          resample, 
                                          silent, 
                                          band_output_type, 
                                          options, 
                                          dsn_outname); 
}
// [[Rcpp::export]]
List warp_suggest_cpp(CharacterVector dsn, CharacterVector target_crs) {
  return gdalwarpgeneral::gdal_suggest_warp(dsn, target_crs); 
}
