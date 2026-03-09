#include <cpp11.hpp>
#include "gdalgeometry/gdalgeometry.h"
#include "gdalmiscutils/gdalmiscutils.h"

using namespace cpp11;

[[cpp11::register]]
doubles feature_count_gdal_cpp(strings dsn,
                               integers layer, strings sql, doubles ex) {
  return gdallibrary::gdal_feature_count(dsn, layer, sql, ex);
}

[[cpp11::register]]
list projection_info_gdal_cpp(strings dsn,
                              integers layer,
                              strings sql) {
  return gdallibrary::gdal_projection_info(dsn, layer, sql);
}

[[cpp11::register]]
strings report_fields_gdal_cpp(strings dsn,
                               integers layer,
                               strings sql) {
  return gdallibrary::gdal_report_fields(dsn, layer, sql);
}

[[cpp11::register]]
strings vapour_geom_name_cpp(strings dsource,
                             integers layer,
                             strings sql,
                             doubles ex) {
  return gdallibrary::with_ogr_layer(dsource, layer, sql, ex,
    [](OGRLayer *poLayer) {
      return gdallibrary::gdal_layer_geometry_name(poLayer);
    });
}

[[cpp11::register]]
doubles vapour_layer_extent_cpp(strings dsource,
                                integers layer,
                                strings sql,
                                doubles ex) {
  return gdallibrary::with_ogr_layer(dsource, layer, sql, ex,
    [](OGRLayer *poLayer) {
      return gdallibrary::gdal_layer_extent(poLayer);
    });
}

[[cpp11::register]]
list gdal_dsn_read_geom_all(strings dsn, integers layer,
                            strings sql, doubles ex, strings format) {
  return gdalgeometry::dsn_read_geom_all(dsn, layer, sql, ex, format);
}

[[cpp11::register]]
list gdal_dsn_read_geom_ij(strings dsn, integers layer,
                           strings sql, doubles ex,
                           strings format, doubles ij) {
  return gdalgeometry::dsn_read_geom_ij(dsn, layer, sql, ex, format, ij);
}

[[cpp11::register]]
list gdal_dsn_read_geom_ia(strings dsn, integers layer,
                           strings sql, doubles ex,
                           strings format, doubles ia) {
  return gdalgeometry::dsn_read_geom_ia(dsn, layer, sql, ex, format, ia);
}

[[cpp11::register]]
list gdal_dsn_read_geom_fa(strings dsn, integers layer,
                           strings sql, doubles ex,
                           strings format, doubles fa) {
  return gdalgeometry::dsn_read_geom_fa(dsn, layer, sql, ex, format, fa);
}

[[cpp11::register]]
list gdal_dsn_read_fields_all(strings dsn, integers layer,
                              strings sql, doubles ex,
                              strings fid_column_name) {
  return gdalgeometry::dsn_read_fields_all(dsn, layer, sql, ex, fid_column_name);
}

[[cpp11::register]]
list gdal_dsn_read_fields_ij(strings dsn, integers layer,
                             strings sql, doubles ex,
                             strings fid_column_name,
                             doubles ij) {
  return gdalgeometry::dsn_read_fields_ij(dsn, layer, sql, ex, fid_column_name, ij);
}

[[cpp11::register]]
list gdal_dsn_read_fields_ia(strings dsn, integers layer,
                             strings sql, doubles ex,
                             strings fid_column_name,
                             doubles ia) {
  return gdalgeometry::dsn_read_fields_ia(dsn, layer, sql, ex, fid_column_name, ia);
}

[[cpp11::register]]
list gdal_dsn_read_fields_fa(strings dsn, integers layer,
                             strings sql, doubles ex,
                             strings fid_column_name,
                             doubles fa) {
  return gdalgeometry::dsn_read_fields_fa(dsn, layer, sql, ex, fid_column_name, fa);
}

[[cpp11::register]]
doubles gdal_dsn_read_fids_all(strings dsn, integers layer,
                               strings sql, doubles ex) {
  return gdalgeometry::dsn_read_fids_all(dsn, layer, sql, ex);
}

[[cpp11::register]]
doubles gdal_dsn_read_fids_ij(strings dsn, integers layer,
                              strings sql, doubles ex, doubles ij) {
  return gdalgeometry::dsn_read_fids_ij(dsn, layer, sql, ex, ij);
}

[[cpp11::register]]
doubles gdal_dsn_read_fids_ia(strings dsn, integers layer,
                              strings sql, doubles ex, doubles ia) {
  return gdalgeometry::dsn_read_fids_ia(dsn, layer, sql, ex, ia);
}

[[cpp11::register]]
list read_fields_gdal_cpp(strings dsn,
                          integers layer,
                          strings sql,
                          integers limit_n,
                          integers skip_n,
                          doubles ex,
                          strings fid_column_name) {
  doubles ij = gdalmiscutils::limit_skip_n_to_start_end_len(skip_n, limit_n,
                                                            feature_count_gdal_cpp(dsn, layer, sql, ex));
  return gdalgeometry::dsn_read_fields_ij(dsn, layer, sql, ex, fid_column_name, ij);
}

[[cpp11::register]]
list read_geometry_gdal_cpp(strings dsn,
                            integers layer,
                            strings sql,
                            strings what,
                            strings textformat,
                            integers limit_n,
                            integers skip_n,
                            doubles ex ) {
  doubles ij = gdalmiscutils::limit_skip_n_to_start_end_len(skip_n, limit_n,
                                                            feature_count_gdal_cpp(dsn, layer, sql, ex));
  return gdalgeometry::dsn_read_geom_ij(dsn, layer, sql, ex, what, ij);
}

[[cpp11::register]]
doubles read_fids_gdal_cpp(strings dsn,
                           integers layer,
                           strings sql,
                           integers limit_n,
                           integers skip_n,
                           doubles ex ) {
  doubles ij = gdalmiscutils::limit_skip_n_to_start_end_len(skip_n, limit_n,
                                                            feature_count_gdal_cpp(dsn, layer, sql, ex));
  return gdal_dsn_read_fids_ij(dsn, layer, sql, ex, ij);
}
