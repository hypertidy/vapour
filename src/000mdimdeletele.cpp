#include <Rcpp.h>
#include "gdalmdimarray/gdalmdimarray.h"


using namespace Rcpp;

// [[Rcpp::export]]
IntegerVector gdalmdimarray_cpp(CharacterVector dsn, CharacterVector array) {
  return gdalmdimarray::gdalmdimarray(dsn, array);
}
