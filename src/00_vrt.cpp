#include <Rcpp.h>
#include "gdalraster/gdalraster.h"
#include "gdalapplib/gdalapplib.h"


using namespace Rcpp;


// [[Rcpp::export]]
CharacterVector raster_vrt_cpp(CharacterVector dsn, 
                               NumericVector extent, 
                               CharacterVector projection, 
                               IntegerVector sds, IntegerVector bands, CharacterVector geolocation, LogicalVector nomd, IntegerVector overview, CharacterVector options) {
  return gdalraster::gdal_dsn_vrt(dsn, extent, projection, sds, bands, geolocation, nomd, overview, options);
 }


// [[Rcpp::export]]
CharacterVector raster_buildvrt_cpp(std::vector<std::string> dsn, 
                               
                               std::vector<std::string> options) {
  return gdalapplib::gdalbuildvrt_applib(dsn, options); 
 }

