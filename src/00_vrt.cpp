#include <Rcpp.h>
#include "gdalraster/gdalraster.h"


using namespace Rcpp;


// [[Rcpp::export]]
CharacterVector raster_vrt_cpp(CharacterVector dsn, 
                               NumericVector extent, 
                               CharacterVector projection, 
                               IntegerVector sds, IntegerVector bands) {
  return gdalraster::gdal_dsn_vrt(dsn, extent, projection, sds, bands);
 }


