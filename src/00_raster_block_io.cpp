#include <Rcpp.h>
#include "gdalreadwrite/gdalreadwrite.h"

using namespace Rcpp;


// [[Rcpp::export]]
Rcpp::List vapour_read_raster_block_cpp(CharacterVector dsource,
                                           IntegerVector offset, IntegerVector dimension, IntegerVector band,
                                           CharacterVector band_output_type) {
  return gdalreadwrite::gdal_read_block(dsource, offset, dimension, band, band_output_type);
}

// [[Rcpp::export]]
Rcpp::LogicalVector vapour_write_raster_block_cpp(CharacterVector dsource, NumericVector data,
                                        IntegerVector offset, IntegerVector dimension, IntegerVector band) {
  return gdalreadwrite::gdal_write_block(dsource, data, offset, dimension, band);
}



// [[Rcpp::export]]
Rcpp::CharacterVector vapour_create_copy_cpp(CharacterVector dsource, CharacterVector dtarget, CharacterVector driver) {
  return gdalreadwrite::gdal_create_copy(dsource, dtarget, driver);
}

// [[Rcpp::export]]
Rcpp::CharacterVector vapour_create_cpp(CharacterVector filename, CharacterVector driver,
                                                                    NumericVector extent, IntegerVector dimension,
                                                                    CharacterVector projection,
                                                                    IntegerVector n_bands) {
  return gdalreadwrite::gdal_create(filename, driver, extent, dimension, projection, n_bands);   
}
