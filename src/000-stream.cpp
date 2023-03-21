#include <Rcpp.h>
#include "gdalarrowstream/gdalvectorstream.h"


using namespace Rcpp;

// see process_cpl_read_ogr_stream
// [[Rcpp::export]]
List gdal_dsn_read_vector_stream(RObject stream_xptr, 
                           CharacterVector dsn, 
                          CharacterVector layer,
                           CharacterVector sql, 
                           Rcpp::CharacterVector options, 
                           bool quiet, 
                           Rcpp::CharacterVector drivers,
                           Rcpp::CharacterVector wkt_filter,
                           bool dsn_exists,
                           bool dsn_isdb,
                           Rcpp::CharacterVector fid_column_name,
                           int width) {
  return gdalvectorstream::read_gdal_stream(stream_xptr, dsn, layer, sql, 
                                               options, quiet, drivers, wkt_filter, dsn_exists, dsn_isdb, 
                                               fid_column_name, width);
}
