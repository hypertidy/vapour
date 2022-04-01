#include <Rcpp.h>
//#include "gdalvrtmodify/gdalvrtmodify.h"
#include "gdalraster/gdalraster.h"


using namespace Rcpp;

// [[Rcpp::export]]
CharacterVector raster_vrt_cpp(CharacterVector dsn, 
                               NumericVector extent, 
                               CharacterVector projection, 
                               IntegerVector sds) {
  return gdalraster::open_as_vrt(dsn, extent, projection, sds);
   //return gdalvrtmodify::gdal_raster_vrt(dsn, extent, projection);
 }
