#include <cpp11.hpp>
#include "gdalapplib/gdalapplib.h"

using namespace cpp11;

[[cpp11::register]]
strings raster_gdalinfo_app_cpp(strings dsn, strings options) {
  return gdalapplib::gdalinfo_applib_cpp(dsn, options);
}
