#include <Rcpp.h>
#include "gdalgeometry/gdalgeometry.h"
#include "gdalmiscutils/gdalmiscutils.h"

using namespace Rcpp;



// [[Rcpp::export]]
NumericVector feature_count_gdal_cpp(CharacterVector dsn,  // double, could be a lot of features
                                     IntegerVector layer, CharacterVector sql, NumericVector ex) {
  return gdallibrary::gdal_feature_count(dsn, layer, sql, ex);
}


// [[Rcpp::export]]
List projection_info_gdal_cpp(CharacterVector dsn,
                              IntegerVector layer,
                              CharacterVector sql) {
  return gdallibrary::gdal_projection_info(dsn, layer, sql);
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

// [[Rcpp::export]]
Rcpp::NumericVector vapour_layer_extent_cpp(CharacterVector dsource, 
                                            IntegerVector layer, 
                                            CharacterVector sql, 
                                            NumericVector ex) {
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *p_layer = gdallibrary::gdal_layer(poDS, layer, sql, ex);
  NumericVector out = gdallibrary::gdal_layer_extent(p_layer);
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(p_layer);
  }
  GDALClose(poDS);
  return out;
  
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
NumericVector gdal_dsn_read_fids_all(CharacterVector dsn, IntegerVector layer,
                                 CharacterVector sql, NumericVector ex) {
  return gdalgeometry::dsn_read_fids_all(dsn, layer, sql, ex);
}
// [[Rcpp::export]]
NumericVector gdal_dsn_read_fids_ij(CharacterVector dsn, IntegerVector layer,
                                CharacterVector sql, NumericVector ex, NumericVector ij) {
  return gdalgeometry::dsn_read_fids_ij(dsn, layer, sql, ex, ij);
}
// [[Rcpp::export]]
NumericVector gdal_dsn_read_fids_ia(CharacterVector dsn, IntegerVector layer,
                                CharacterVector sql, NumericVector ex, NumericVector ia) {
  return gdalgeometry::dsn_read_fids_ia(dsn, layer, sql, ex, ia);
}



// [[Rcpp::export]]
List read_fields_gdal_cpp(CharacterVector dsn,
                          IntegerVector layer,
                          CharacterVector sql,
                          IntegerVector limit_n,
                          IntegerVector skip_n,
                          NumericVector ex,
                          CharacterVector fid_column_name) {

 // validate_limit_n ensures it is 0 or greater
 // but, if greater we must ensure it won't exceed the feature count
 NumericVector ij;
    ij = gdalmiscutils::limit_skip_n_to_start_end_len(skip_n, limit_n, 
                                                                    feature_count_gdal_cpp(dsn, layer, sql, ex)); 
  return gdalgeometry::dsn_read_fields_ij(dsn, layer, sql, ex, fid_column_name, ij);
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
  // validate_limit_n ensures it is 0 or greater
  // but, if greater we must ensure it won't exceed the feature count
  // we get two warnings if the extent is poorly ordered, see 171 but I don't want to fix that atm
  // we might send in quiet messages ...
   NumericVector ij = gdalmiscutils::limit_skip_n_to_start_end_len(skip_n, limit_n, 
                                                      feature_count_gdal_cpp(dsn, layer, sql, ex)); 


    return gdalgeometry::dsn_read_geom_ij(dsn, layer, sql, ex, what, ij); 
}

// [[Rcpp::export]]
NumericVector read_fids_gdal_cpp(CharacterVector dsn,
                         IntegerVector layer,
                         CharacterVector sql,
                         IntegerVector limit_n,
                         IntegerVector skip_n,
                         NumericVector ex ) {
  // validate_limit_n ensures it is 0 or greater
  // but, if greater we must ensure it won't exceed the feature count
    NumericVector ij = gdalmiscutils::limit_skip_n_to_start_end_len(skip_n, limit_n, 
                                                      feature_count_gdal_cpp(dsn, layer, sql, ex)); 

  return gdal_dsn_read_fids_ij(dsn, layer, sql, ex, ij);
}

