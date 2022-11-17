#include "proj.h"

#include <Rcpp.h>
using namespace Rcpp;

// this returns a list of 2 or 4, takes in a list of 2 or 4
// 2 == x,y
// 4 =- x,y,z,t
// If NULL returned,
//    input list not len 2 or 4 || crs_to_crs is invalid || gis-order is invalid || PROJ>=6 is not available
// [[Rcpp::export]]
List proj_reproj_list(List x, CharacterVector source, CharacterVector target)
{
  PJ_CONTEXT *C;
  PJ *P;
  PJ* P_for_GIS;
  PJ_COORD a, b;
  C = PJ_DEFAULT_CTX;
  
  List nilist; 
  int ncolumns = x.size();
  if (ncolumns != 2 && ncolumns != 4) {
    Rprintf("bad columns %i\n", ncolumns);
    return(nilist);
  }

  NumericVector x_copy = x[0];
  
  NumericVector y_copy = x[1];
  
  NumericVector  z_copy, t_copy;
  if (ncolumns == 4) {
    z_copy =  z_copy = x[2];
    t_copy =  t_copy = x[3];
  }
  /* FIXME: could in principle be a long vector */
  int N = x_copy.size();
  int r;
  
  const char* crs_in =  (const char *)source[0];
  const char* crs_out = (const char *)target[0];
  
  
  P = proj_create_crs_to_crs(C, crs_in, crs_out, NULL);
  if (0 == P) {
    Rprintf("crs to crs problem\n");
    return(nilist);
  }
  
  P_for_GIS = proj_normalize_for_visualization(C, P);
  if( 0 == P_for_GIS )  {
    Rprintf("bad longlat order\n");
    return(nilist);
  }
  proj_destroy(P);
  P = P_for_GIS;
  
  for (int i = 0; i < N; i++) {
    if (ncolumns == 2) {
      a = proj_coord(x_copy[i], y_copy[i], 0, 0);
    } else {
      a = proj_coord(x_copy[i], y_copy[i], z_copy[i], t_copy[i]);
    }
    b = proj_trans(P, PJ_FWD, a);
    x_copy[i] = b.xyzt.x;
    y_copy[i] = b.xyzt.y;
    if (ncolumns == 4) {
      z_copy[i] = b.xyzt.z;
      t_copy[i] = b.xyzt.t;
    }
  }
  r = proj_errno(P);
  proj_destroy(P);
  if (r) {
    Rprintf("Error detected, some values Inf (error code: %i)\n\n", r);
    Rprintf("' %s\n\n '", proj_errno_string(r));
  }
  
  
  List vec(ncolumns); 
  vec[0] = x_copy;
  vec[1] = y_copy;
 

  if (ncolumns == 2) {
  
  } else {
    vec[2] = z_copy;
    vec[3] = t_copy;
    
  }
  
  return vec;
}





// [[Rcpp::export]]
List proj_reproj_xy(NumericVector x, NumericVector y, CharacterVector source, CharacterVector target) {
  List out(2); 
 
 PJ_CONTEXT *C;
 PJ *P;
 PJ* P_for_GIS;
 PJ_COORD a, b;
 C = PJ_DEFAULT_CTX;
 /* FIXME: could in principle be a long vector */
 int N = x.size();
 int r;
 

 NumericVector xout(N);
 NumericVector yout(N);
 
 const char* crs_in =  (const char *)source[0];
 const char* crs_out = (const char *)target[0];
 
 List nilist; 
 P = proj_create_crs_to_crs(C, crs_in, crs_out, NULL);
 if (0 == P) {
   return(nilist);
 }
 
 P_for_GIS = proj_normalize_for_visualization(C, P);
 if( 0 == P_for_GIS )  {
   return(nilist);
 }
 proj_destroy(P);
 P = P_for_GIS;
 
 for (int i = 0; i < N; i++) {
   a = proj_coord(x[i], y[i], 0, 0);
   b = proj_trans(P, PJ_FWD, a);
   xout[i] = b.xyzt.x;
   yout[i] = b.xyzt.y;
 }
 r = proj_errno(P);
 proj_destroy(P);
 if (r) {
   Rprintf("Error detected, some values Inf (error code: %i)\n\n", r);
   Rprintf("' %s\n\n '", proj_errno_string(r));
 }
 
 List vec(2);
 vec[0] = xout;
 vec[1] = yout; 
 
 return vec;
}




