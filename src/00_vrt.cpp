#include <cpp11.hpp>
#include "gdalraster/gdalraster.h"
#include "gdalapplib/gdalapplib.h"

using namespace cpp11;
using gdalcommon::gdal_call;

[[cpp11::register]]
strings raster_vrt_cpp(strings dsn,
                       doubles extent,
                       strings projection,
                       integers sds, integers bands, strings geolocation, logicals nomd, integers overview, strings options,
                       strings open_options) {
  return gdal_call([&]() {
    return gdalraster::gdal_dsn_vrt(dsn, extent, projection, sds, bands, geolocation, nomd, overview, options, open_options);
  });
}

[[cpp11::register]]
strings raster_buildvrt_cpp(std::vector<std::string> dsn,
                            std::vector<std::string> options) {
  return gdal_call([&]() { return gdalapplib::gdalbuildvrt_applib(dsn, options); });
}
