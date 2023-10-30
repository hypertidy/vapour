#include <Rcpp.h>
#include "gdalarrowstream/gdalvectorstream.h"


using namespace Rcpp;

// see process_cpl_read_ogr_stream
// // [[Rcpp::export]]
// List gdal_dsn_read_vector_stream(RObject stream_xptr, 
//                            CharacterVector dsn, 
//                           CharacterVector layer,
//                            CharacterVector sql, 
//                            std::vector<std::string> options, 
//                            bool quiet, 
//                            std::vector<std::string> drivers,
//                            Rcpp::NumericVector extent,
//                            bool dsn_exists,
//                            Rcpp::CharacterVector fid_column_name,
//                            int width) {
//   return gdalvectorstream::read_gdal_stream(stream_xptr, dsn, layer, sql, 
//                                                options, quiet, drivers, extent, dsn_exists, 
//                                                fid_column_name, width);
// }


