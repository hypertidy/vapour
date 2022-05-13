#include <Rcpp.h>
#include "gdalapplib/gdalapplib.h"


using namespace Rcpp;

// [[Rcpp::export]]
CharacterVector raster_gdalinfo_app_cpp(CharacterVector dsn, CharacterVector options) {
  return gdalapplib::gdalinfo_applib_cpp(dsn, options);
}
