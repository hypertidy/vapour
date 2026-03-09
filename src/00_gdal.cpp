#include <cpp11.hpp>
#include "gdallibrary/gdallibrary.h"

using namespace cpp11;

[[cpp11::register]]
logicals cleanup_gdal_cpp() {
  gdallibrary::ogr_cleanup_all();
  gdallibrary::osr_cleanup();
  writable::logicals out(1);
  out[0] = TRUE;
  return out;
}

[[cpp11::register]]
strings driver_gdal_cpp(strings dsn) {
  return gdallibrary::gdal_driver(dsn);
}

[[cpp11::register]]
strings driver_id_gdal_cpp(strings dsn) {
  return gdallibrary::gdal_driver(dsn);
}

[[cpp11::register]]
strings layer_names_gdal_cpp(strings dsn) {
  return gdallibrary::gdal_layer_names(dsn);
}

[[cpp11::register]]
list drivers_list_gdal_cpp() {
  return gdallibrary::gdal_list_drivers();
}

[[cpp11::register]]
strings proj_to_wkt_gdal_cpp(strings proj4string) {
  return gdallibrary::gdal_proj_to_wkt(proj4string);
}

[[cpp11::register]]
logicals crs_is_lonlat_cpp(strings input_string) {
  return gdallibrary::gdal_crs_is_lonlat(input_string);
}

[[cpp11::register]]
logicals register_gdal_cpp() {
  gdallibrary::gdal_register_all();
  gdallibrary::ogr_register_all();
  writable::logicals out(1);
  out[0] = TRUE;
  return out;
}

[[cpp11::register]]
strings version_gdal_cpp() {
  return gdallibrary::gdal_version();
}

[[cpp11::register]]
integers version_proj_cpp() {
  return gdallibrary::proj_version();
}

[[cpp11::register]]
strings vsi_list_gdal_cpp(strings dsn) {
  return gdallibrary::gdal_vsi_list(dsn);
}
