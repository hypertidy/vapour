#include <cpp11.hpp>
#include "gdallibrary/gdallibrary.h"

using namespace cpp11;
using gdalcommon::gdal_call;

[[cpp11::register]]
list raster_gcp_gdal_cpp(strings dsn, strings open_options) {
  return gdal_call([&]() { return gdalraster::gdal_raster_gcp(dsn, open_options); });
}

[[cpp11::register]]
logicals raster_has_geolocation_gdal_cpp(strings dsn, integers sds, strings open_options) {
  return gdal_call([&]() { return gdalraster::gdal_has_geolocation(dsn, sds, open_options); });
}

[[cpp11::register]]
list raster_list_geolocation_gdal_cpp(strings dsn, integers sds, strings open_options) {
  return gdal_call([&]() { return gdalraster::gdal_list_geolocation(dsn, sds, open_options); });
}

[[cpp11::register]]
list raster_info_gdal_cpp(strings dsn, logicals min_max) {
  return gdal_call([&]() { return gdalraster::gdal_raster_info(dsn, min_max); });
}

[[cpp11::register]]
doubles raster_extent_cpp(strings dsn) {
  return gdal_call([&]() { return gdalraster::gdal_extent_only(dsn); });
}

[[cpp11::register]]
list raster_io_gdal_cpp(strings dsn,
                        integers window,
                        integers band,
                        strings resample,
                        strings band_output_type,
                        logicals unscale,
                        logicals nara,
                        strings open_options) {
  return gdal_call([&]() {
    return gdalraster::gdal_raster_io(dsn, window, band, resample, band_output_type, unscale, nara, open_options);
  });
}

[[cpp11::register]]
strings sds_list_gdal_cpp(strings dsn) {
  return gdal_call([&]() { return gdalraster::gdal_sds_list(as_cstr(dsn[0])); });
}

[[cpp11::register]]
list sds_list_list_gdal_cpp(strings dsn) {
  return gdal_call([&]() {
    writable::list outlist(dsn.size());
    for (int i = 0; i < dsn.size(); i++) {
      outlist[i] = gdalraster::gdal_sds_list(as_cstr(dsn[i]));
    }
    return outlist;
  });
}
