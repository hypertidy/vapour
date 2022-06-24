#include <Rcpp.h>
#include "gdalapplib/gdalapplib.h"


using namespace Rcpp;

// [[Rcpp::export]]
List raster_warp_file_cpp(CharacterVector source_filename,
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
  return gdalapplib::gdalwarp_applib(source_filename,
                                     target_crs,
                                     target_extent,
                                     target_dim,
                                     target_filename,
                                     bands,
                                     resample,
                                     silent,
                                     band_output_type,
                                     warp_options,
                                     transformation_options);
}
