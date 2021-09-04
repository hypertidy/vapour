#include <Rcpp.h>
#include "gdalgeometry/gdalgeometry.h"
using namespace Rcpp;

// [[Rcpp::export]]
DoubleVector feature_count_gdal_cpp(CharacterVector dsn,  // double, could be a lot of features
                                    IntegerVector layer, CharacterVector sql, NumericVector ex) {
  return gdallibrary::gdal_feature_count(dsn, layer, sql, ex);
}

// [[Rcpp::export]]
List gdal_dsn_read_geom_all(CharacterVector dsn, IntegerVector layer,
                            CharacterVector sql, NumericVector ex, CharacterVector format) {
  return gdalgeometry::dsn_read_geom_all(dsn, layer, sql, ex, format);
}

// [[Rcpp::export]]
List gdal_dsn_read_geom_ij(CharacterVector dsn, IntegerVector layer,
                           CharacterVector sql, NumericVector ex,
                           CharacterVector format, NumericVector ij) {
  return gdalgeometry::dsn_read_geom_ij(dsn, layer, sql, ex, format, ij);
}

// [[Rcpp::export]]
List gdal_dsn_read_geom_ia(CharacterVector dsn, IntegerVector layer,
                           CharacterVector sql, NumericVector ex,
                           CharacterVector format, NumericVector ia) {
  return gdalgeometry::dsn_read_geom_ia(dsn, layer, sql, ex, format, ia);
}

// [[Rcpp::export]]
List gdal_dsn_read_geom_fa(CharacterVector dsn, IntegerVector layer,
                           CharacterVector sql, NumericVector ex,
                           CharacterVector format, NumericVector fa) {
  return gdalgeometry::dsn_read_geom_fa(dsn, layer, sql, ex, format, fa);
}

// [[Rcpp::export]]
List gdal_dsn_read_fields_all(CharacterVector dsn, IntegerVector layer,
                              CharacterVector sql, NumericVector ex,
                              CharacterVector fid_column_name) {
  return gdalgeometry::dsn_read_fields_all(dsn, layer, sql, ex, fid_column_name);
}

// [[Rcpp::export]]
List gdal_dsn_read_fields_ij(CharacterVector dsn, IntegerVector layer,
                             CharacterVector sql, NumericVector ex,
                             CharacterVector fid_column_name,
                             NumericVector ij) {
  return gdalgeometry::dsn_read_fields_ij(dsn, layer, sql, ex, fid_column_name, ij);
}

// [[Rcpp::export]]
List gdal_dsn_read_fields_ia(CharacterVector dsn, IntegerVector layer,
                             CharacterVector sql, NumericVector ex,
                             CharacterVector fid_column_name,
                             NumericVector ia) {
  return gdalgeometry::dsn_read_fields_ia(dsn, layer, sql, ex, fid_column_name, ia);
}

// [[Rcpp::export]]
List gdal_dsn_read_fields_fa(CharacterVector dsn, IntegerVector layer,
                             CharacterVector sql, NumericVector ex,
                             CharacterVector fid_column_name,
                             NumericVector fa) {
  return gdalgeometry::dsn_read_fields_fa(dsn, layer, sql, ex, fid_column_name, fa);
}

// [[Rcpp::export]]
NumericVector gdal_read_fids_all(CharacterVector dsn, IntegerVector layer,
                                 CharacterVector sql, NumericVector ex) {
  return gdalgeometry::dsn_read_fids_all(dsn, layer, sql, ex);
}
// [[Rcpp::export]]
NumericVector gdal_read_fids_ij(CharacterVector dsn, IntegerVector layer,
                                CharacterVector sql, NumericVector ex, NumericVector ij) {
  return gdalgeometry::dsn_read_fids_ij(dsn, layer, sql, ex, ij);
}
// [[Rcpp::export]]
NumericVector gdal_read_fids_ia(CharacterVector dsn, IntegerVector layer,
                                CharacterVector sql, NumericVector ex, NumericVector ia) {
  return gdalgeometry::dsn_read_fids_ia(dsn, layer, sql, ex, ia);
}

// [[Rcpp::export]]
List geometry_cpp(CharacterVector dsn, IntegerVector layer,
                  CharacterVector sql, NumericVector ex,
                  CharacterVector format, NumericVector fid) {
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *p_layer = gdallibrary::gdal_layer(poDS, layer, sql, ex);
  List g_list = gdalgeometry::layer_read_geom_fa(p_layer, format, fid);
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(p_layer);
  }
  GDALClose(poDS);
  return g_list;
}

// [[Rcpp::export]]
List geometry_cpp_limit_skip(CharacterVector dsn, IntegerVector layer,
                             CharacterVector sql, NumericVector ex, CharacterVector format,
                             IntegerVector limit_n, IntegerVector skip_n) {
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *p_layer = gdallibrary::gdal_layer(poDS, layer, sql, ex);
  NumericVector ij(2);
  ij[0] = skip_n[0];
  ij[1] = skip_n[0] + limit_n[0] - 1;
  List g_list = gdalgeometry::layer_read_geom_ij(p_layer, format, ij);
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(p_layer);
  }
  return g_list;
}

// [[Rcpp::export]]
List projection_info_gdal_cpp(CharacterVector dsn,
                              IntegerVector layer,
                              CharacterVector sql) {
  return gdallibrary::gdal_projection_info(dsn, layer, sql);
}

// [[Rcpp::export]]
List read_fields_gdal_cpp(CharacterVector dsn,
                          IntegerVector layer,
                          CharacterVector sql,
                          IntegerVector limit_n,
                          IntegerVector skip_n,
                          NumericVector ex,
                          CharacterVector fid_column_name) {
  return gdallibrary::gdal_read_fields(dsn, layer, sql, limit_n, skip_n, ex, fid_column_name);
}

// [[Rcpp::export]]
List read_geometry_gdal_cpp(CharacterVector dsn,
                            IntegerVector layer,
                            CharacterVector sql,
                            CharacterVector what,
                            CharacterVector textformat,
                            IntegerVector limit_n,
                            IntegerVector skip_n,
                            NumericVector ex ) {
  return gdallibrary::gdal_read_geometry(dsn, layer, sql, what, textformat, limit_n, skip_n, ex);
}

// [[Rcpp::export]]
List read_names_gdal_cpp(CharacterVector dsn,
                         IntegerVector layer,
                         CharacterVector sql,
                         IntegerVector limit_n,
                         IntegerVector skip_n,
                         NumericVector ex ) {
  return gdallibrary::gdal_read_names(dsn, layer, sql, limit_n, skip_n, ex);
}

// [[Rcpp::export]]
CharacterVector report_fields_gdal_cpp(CharacterVector dsn,
                                       IntegerVector layer,
                                       CharacterVector sql) {
  return gdallibrary::gdal_report_fields(dsn, layer, sql);
}

// [[Rcpp::export]]
Rcpp::CharacterVector vapour_geom_name_cpp(CharacterVector dsource,
                                           IntegerVector layer,
                                           Rcpp::CharacterVector sql,
                                           NumericVector ex) {
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *p_layer = gdallibrary::gdal_layer(poDS, layer, sql, ex);
  CharacterVector out = gdallibrary::gdal_layer_geometry_name(p_layer);
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(p_layer);
  }
  GDALClose(poDS);
  return out;
}
