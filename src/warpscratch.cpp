// #include <Rcpp.h>
// using namespace Rcpp;
// 
// 
// #include "gdal_priv.h"
// #include "gdalwarper.h"
// #include "gdal_utils.h"  // for GDALWarpAppOptions
// #include "gdalraster/gdalraster.h"
// #include "ogr_spatialref.h"  // for OGRCreateCoordinateTransformation
// 
// #include "gdalarrowstream/gdalvectorstream.h"
// 
// 
// 
// // [[Rcpp::export]]
// List gdal_ptrs_cpp(CharacterVector dsn, CharacterVector layer) {
//   return gdalvectorstream::ogr_ptrs(dsn, layer); 
// }
// 
// // [[Rcpp::export]]
// NumericVector timesTwo(std::vector<std::string> x) {
// 
//   std::vector <char *> vops = string_to_charptr( x);
//  auto psOptions = GDALWarpAppOptionsNew(vops.data(), nullptr);
// 
//   GDALWarpAppOptionsFree(psOptions);
// 
//   NumericVector num = NumericVector(1); 
//   num[0] = 2.0;
//   return num;
// }
// 
// 
// // [[Rcpp::export]]
// IntegerVector n_layers(SEXP xp) {
//   GDALDataset *poDS = (GDALDataset*)R_ExternalPtrAddr(xp);
//   IntegerVector res(1); 
//   res[0] = poDS->GetLayerCount(); 
//   return res;
// }
// 
// // [[Rcpp::export]]
// CharacterVector ptr_query(SEXP xp) {
//   OGRLayer *poLayer = (OGRLayer*)R_ExternalPtrAddr(xp);
//   CharacterVector res(1); 
//    res[0] = poLayer->GetDescription(); 
//   return res;
// }
// 
