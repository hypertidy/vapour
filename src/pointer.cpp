#include <Rcpp.h>
using namespace Rcpp;
#include "gdal_priv.h"
#include "ogrsf_frmts.h"

// [[Rcpp::export]]
SEXP gh_GDALOpenEx(CharacterVector dsn) {
  
  GDALAllRegister();
  // create pointer to an GDAL object and
  // wrap it as an external pointer
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_READONLY, NULL, NULL, NULL);
  //poDS = (GDALDataset*) GDALOpen(dsn[0], GDAL_OF_READONLY);
  
  if( poDS == NULL )
  {
    Rprintf("Problem with 'dsn' input: %s\n", dsn[0]);
    Rcpp::stop("Open failed.\n");
  }
  Rcpp::XPtr<GDALDataset> ptr(poDS);
  
  return ptr;
}


/// invoke the close method
// [[Rcpp::export]]
SEXP gh_GDALClose(SEXP xp) {
  
  // grab the object as a XPtr (smart pointer)
  // to GDALDataset
  Rcpp::XPtr<GDALDataset> ptr(xp);
  NumericVector res(1);
  res[0] = 0;
  GDALClose(ptr);
  res[0] = 1;
  // return the result to R
  return res;
}
// ExecuteSQL
// [[Rcpp::export]]
SEXP gh_ExecuteSQL(SEXP xp, CharacterVector sql, SEXP extent) {
  Rcpp::XPtr<GDALDataset> ptr(xp);
  OGRLayer  *poLayer;
  poLayer = ptr->ExecuteSQL(sql[0], NULL, NULL);
  
  Rcpp::XPtr<OGRLayer> out_ptr(poLayer);
  
  return out_ptr;
}

// [[Rcpp::export]]
SEXP gh_GetLayer(SEXP xp, IntegerVector layer) {
  // this, or harmless emssage from XPtr below?
  // if (R_ExternalPtrAddr(xp) == NULL)  {
  //   Rcpp::stop("dataset pointer is nil");
  // }
  
  Rcpp::XPtr<GDALDataset> ptr(xp);
  
  OGRLayer  *poLayer;
  poLayer = ptr->GetLayer(layer[0]);
  poLayer->ResetReading();
  Rcpp::XPtr<OGRLayer> out_ptr(poLayer);
  return out_ptr;
}
// [[Rcpp::export]]
SEXP gh_GetNextFeature(SEXP xp) {
  XPtr<OGRLayer> lyr(xp);
  OGRFeature *poFeature;
  
  //lyr->ResetReading();
  poFeature = lyr->GetNextFeature();
  // double* pdfProgressPct; 
  // GDALProgressFunc pfnProgress; 
  // void* pProgressData; 
  // poFeature = GDALDataset::GetNextFeature(lyr, pdfProgressPct, pfnProgress, pProgressData); 
  // //double nFeature = (double)
  
  //  poFeature = ptr->GetNextFeature();
  XPtr<OGRFeature> out_ptr(poFeature);
  return out_ptr;
}
// [[Rcpp::export]]
SEXP gh_DestroyFeature(SEXP xp) {
  XPtr<OGRFeature> feature(xp);
  
  OGRFeature::DestroyFeature(feature);
  return Rcpp::wrap(1);
}
// [[Rcpp::export]]
SEXP gh_getGeometryRef(SEXP xp) {
  XPtr<OGRFeature> feature(xp);

  OGRGeometry *poGeometry;
  poGeometry = feature->GetGeometryRef();
  XPtr<OGRGeometry> out_ptr(poGeometry);
  return out_ptr;
}
// [[Rcpp::export]]
SEXP gh_getPoints_preview(SEXP xp) {
  XPtr<OGRGeometry> poGeometry(xp); 
  OGRGeometryH hR  = OGR_G_GetGeometryRef(poGeometry, 0); 
  
  int nn = 4; 
  int stride = sizeof(double), n = OGR_G_GetPointCount(hR);
  if (n < nn) nn = n; 
  SEXP x, y, z;
  x = PROTECT(Rf_allocVector(REALSXP, n));
  y = PROTECT(Rf_allocVector(REALSXP, n));
  z = PROTECT(Rf_allocVector(REALSXP, n));
  OGR_G_GetPoints(hR, REAL(x), stride, REAL(y), stride, REAL(z), stride);
  
  return Rcpp::wrap(REAL(x)[0]); 
}
// [[Rcpp::export]]
SEXP gh_exportToWkb(SEXP xp) {
  XPtr<OGRGeometry> geom(xp);
  Rcpp::RawVector raw(geom->WkbSize());
  geom->exportToWkb(wkbNDR, &(raw[0]), wkbVariantIso);
  return Rcpp::wrap(raw);
}
// surely no point to a hook like this, we just want a src/ wrap to do what gdal_list_drivers() gets
// [[Rcpp::export]]
SEXP gh_GetGDALDriverManager()  {
  GDALDriverManager *gdm  = GetGDALDriverManager();
  Rcpp::XPtr<GDALDriverManager> ptr(gdm);
  return ptr;
}

// but I'm practicing the art ...
// [[Rcpp::export]]
SEXP gh_GetDriverCount(SEXP xp)  {
  
  Rcpp::XPtr<GDALDriverManager> ptr(xp);
  int n = ptr->GetDriverCount();
  return Rcpp::wrap(n);
}


