#include <Rcpp.h>
#include "gdalvrtmodify/gdalvrtmodify.h"


using namespace Rcpp;

// [[Rcpp::export]]
CharacterVector raster_vrt_cpp(CharacterVector dsn, 
                               NumericVector extent, 
                               CharacterVector projection) {
   return gdalvrtmodify::gdal_raster_vrt(dsn, extent, projection);
 }
