#include <cpp11.hpp>
#include "gdalapplib/gdalapplib.h"

using namespace cpp11;
using gdalcommon::gdal_call;

[[cpp11::register]]
strings raster_gdalinfo_app_cpp(strings dsn, strings options, integers sds, strings open_options) {
  return gdal_call([&]() { return gdalapplib::gdalinfo_applib_cpp(dsn, options, sds, open_options); });
}
