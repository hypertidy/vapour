#include <Rcpp.h>
#include "gdalgeometry/gdalgeometry.h"
using namespace Rcpp;


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
