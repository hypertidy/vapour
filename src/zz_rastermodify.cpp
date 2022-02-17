#include <Rcpp.h>
#include "gdalvrtmodify/gdalvrtmodify.h"


using namespace Rcpp;

// [[Rcpp::export]]
CharacterVector vrt_raster_modify_cpp(CharacterVector dsn, CharacterVector tempfile) {
  return gdalvrtmodify::gdal_vrt_raster_modify(dsn, tempfile);
}
