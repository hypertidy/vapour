#include <cpp11.hpp>
#include "gdallibrary/gdallibrary.h"
#include "gdalwarpmem/gdalwarpmem.h"

using namespace cpp11;

[[cpp11::register]]
list raster_gcp_gdal_cpp(strings dsn) {
  return gdalraster::gdal_raster_gcp(dsn);
}

[[cpp11::register]]
logicals raster_has_geolocation_gdal_cpp(strings dsn, integers sds) {
  return gdalraster::gdal_has_geolocation(dsn, sds);
}

[[cpp11::register]]
list raster_list_geolocation_gdal_cpp(strings dsn, integers sds) {
  return gdalraster::gdal_list_geolocation(dsn, sds);
}

[[cpp11::register]]
list raster_info_gdal_cpp(strings dsn, logicals min_max) {
  return gdalraster::gdal_raster_info(dsn, min_max);
}

[[cpp11::register]]
doubles raster_extent_cpp(strings dsn) {
  return gdalraster::gdal_extent_only(dsn);
}

[[cpp11::register]]
list raster_io_gdal_cpp(strings dsn,
                        integers window,
                        integers band,
                        strings resample,
                        strings band_output_type,
                        logicals unscale,
                        logicals nara) {
  return gdalraster::gdal_raster_io(dsn, window, band, resample, band_output_type, unscale, nara);
}

[[cpp11::register]]
strings sds_list_gdal_cpp(strings dsn) {
  return gdalraster::gdal_sds_list(std::string(dsn[0]).c_str());
}

[[cpp11::register]]
list sds_list_list_gdal_cpp(strings dsn) {
  writable::list outlist(dsn.size());
  for (int i = 0; i < dsn.size(); i++) {
    outlist[i] = gdalraster::gdal_sds_list(std::string(dsn[i]).c_str());
  }
  return outlist;
}
