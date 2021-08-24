#include <Rcpp.h>
#include "gdalreadwrite/gdalreadwrite.h"

using namespace Rcpp;


// [[Rcpp::export]]
Rcpp::List vapour_read_raster_block_cpp(CharacterVector dsource,
                                           IntegerVector offset, IntegerVector dimension, IntegerVector band) {

  return gdalreadwrite::gdal_read_block(dsource, offset, dimension, band);
}

// [[Rcpp::export]]
Rcpp::LogicalVector vapour_write_raster_block_cpp(CharacterVector dsource, NumericVector data,
                                        IntegerVector offset, IntegerVector dimension, IntegerVector band) {

  return gdalreadwrite::gdal_write_block(dsource, data, offset, dimension, band);
}
