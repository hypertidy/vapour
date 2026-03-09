#include <cpp11.hpp>
#include "gdalarrowstream/gdalvectorstream.h"

using namespace cpp11;

[[cpp11::register]]
list gdal_dsn_read_vector_stream(cpp11::sexp stream_xptr,
                           strings dsn,
                           strings layer,
                           strings sql,
                           std::vector<std::string> options,
                           bool quiet,
                           std::vector<std::string> drivers,
                           doubles extent,
                           bool dsn_exists,
                           strings fid_column_name,
                           int width) {
  return gdalvectorstream::read_gdal_stream(stream_xptr, dsn, layer, sql,
                                               options, quiet, drivers, extent, dsn_exists,
                                               fid_column_name, width);
}
