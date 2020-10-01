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

  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(p_layer);
  }
  GDALClose(poDS);

  return out;
}
