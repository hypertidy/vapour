#include <limits>


#include "ogr_api.h"
#include <Rcpp.h>


using namespace Rcpp;

constexpr int MAX_INT =  std::numeric_limits<int>::max ();



// thk686
static void attachPoints(SEXP res, OGRGeometryH hG)
{
  SEXP x, y, z;
  int stride = sizeof(double),
    n = OGR_G_GetPointCount(hG);
  if ( n == 0 ) return;
  x = PROTECT(Rf_allocVector(REALSXP, n));
  y = PROTECT(Rf_allocVector(REALSXP, n));
  z = PROTECT(Rf_allocVector(REALSXP, n));
  OGR_G_GetPoints(hG, REAL(x), stride, REAL(y), stride, REAL(z), stride);
  switch ( Rf_length(res) )
  {
  case 3:;
    SET_VECTOR_ELT(res, 0, x);
    SET_VECTOR_ELT(res, 1, y);
    SET_VECTOR_ELT(res, 2, z);
    break;
  case 2:;
    SET_VECTOR_ELT(res, 0, x);
    SET_VECTOR_ELT(res, 1, y);
    break;
  default:;
  Rf_warning("Empty geometry\n");
  break;
  }
  UNPROTECT(3);
  return;
}
// thk686
static void attachNames(SEXP x)
{
  int dim = Rf_length(x);
  SEXP names = PROTECT(Rf_allocVector(STRSXP, dim));
  switch ( dim )
  {
  case 3:;
    SET_STRING_ELT(names, 2, Rf_mkChar("z"));
  case 2:;
    SET_STRING_ELT(names, 1, Rf_mkChar("y"));
    SET_STRING_ELT(names, 0, Rf_mkChar("x"));
    break;
  default:;
  Rf_warning("Empty geometry\n");
  break;
  }
  Rf_setAttrib(x, R_NamesSymbol, names);
  UNPROTECT(1);
  return;
}
// thk686
static SEXP GetPointsInternal(OGRGeometryH hG, int nested)
{
  SEXP res;
  int n = OGR_G_GetGeometryCount(hG);
  switch ( n )
  {
  case 0:
  {
    int dim = OGR_G_GetCoordinateDimension(hG);
    res = PROTECT(Rf_allocVector(VECSXP, dim));
    attachPoints(res, hG);
    attachNames(res);
    UNPROTECT(1);
  }
    break;
  case 1:
    if (nested == 0) {
      {
        OGRGeometryH hR = OGR_G_GetGeometryRef(hG, 0);
        res = GetPointsInternal(hR, nested);
      }
      break;
    } // else: fall through
  default:
    {
      res = PROTECT(Rf_allocVector(VECSXP, n));
      for ( int i = 0; i != n; ++i )
      {
        OGRGeometryH hR = OGR_G_GetGeometryRef(hG, i);
        SET_VECTOR_ELT(res, i, GetPointsInternal(hR, nested));
      }
      UNPROTECT(1);
    }
    break;
  }
  return res;
}












