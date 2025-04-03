#include <Rcpp.h>
#include "gdallibrary/gdallibrary.h"

using namespace Rcpp;

// [[Rcpp::export]]
LogicalVector cleanup_gdal_cpp() {
  gdallibrary::ogr_cleanup_all();
  gdallibrary::osr_cleanup();

  LogicalVector out(1);
  out[0] = true;
  return out;
}

// [[Rcpp::export]]
CharacterVector driver_gdal_cpp(CharacterVector dsn) {
  return gdallibrary::gdal_driver(dsn);
}

// [[Rcpp::export]]
CharacterVector driver_id_gdal_cpp(CharacterVector dsn) {
  return gdallibrary::gdal_driver(dsn);
}

// [[Rcpp::export]]
CharacterVector layer_names_gdal_cpp(CharacterVector dsn) {
  return gdallibrary::gdal_layer_names(dsn);
}

// [[Rcpp::export]]
List drivers_list_gdal_cpp() {
  return gdallibrary::gdal_list_drivers();
}
// [[Rcpp::export]]
CharacterVector proj_to_wkt_gdal_cpp(CharacterVector proj4string) {
  return gdallibrary::gdal_proj_to_wkt(proj4string);
}

// [[Rcpp::export]]
LogicalVector crs_is_lonlat_cpp(CharacterVector input_string) {
  return gdallibrary::gdal_crs_is_lonlat(input_string);
}


// [[Rcpp::export]]
LogicalVector register_gdal_cpp() {
  gdallibrary::gdal_register_all();
  gdallibrary::ogr_register_all();

  LogicalVector out(1);
  out[0] = true;
  return out;
}

// [[Rcpp::export]]
CharacterVector version_gdal_cpp() {
  return gdallibrary::gdal_version();
}
// [[Rcpp::export]]
IntegerVector version_proj_cpp() {
  return gdallibrary::proj_version();
}
// [[Rcpp::export]]
CharacterVector vsi_list_gdal_cpp(CharacterVector dsn) {
  return gdallibrary::gdal_vsi_list(dsn);
}
