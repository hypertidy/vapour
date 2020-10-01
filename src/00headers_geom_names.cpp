#include <Rcpp.h>
#include "gdallibrary/gdallibrary.h"

using namespace Rcpp;




// [[Rcpp::export]]
Rcpp::CharacterVector vapour_geom_name_cpp(CharacterVector dsource,
                                           IntegerVector layer,
                                           Rcpp::CharacterVector sql,
                                           NumericVector ex) {

  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *p_layer = gdallibrary::gdal_layer(poDS, layer, sql, ex);

  CharacterVector out = gdallibrary::gdal_layer_geometry_name(p_layer);

  GDALClose(poDS);

  return out;
}
