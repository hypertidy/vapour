#include <Rcpp.h>
using namespace Rcpp;

#include "ogr_spatialref.h" // for OGRSpatialReference
#include "cpl_conv.h" // for CPLFree()

// [[Rcpp::export]]
CharacterVector proj_to_wkt_cpp(CharacterVector proj_str) {
  OGRSpatialReference oSRS;
  char *pszWKT = NULL;
  oSRS.importFromProj4(proj_str[0]);
  oSRS.exportToWkt(&pszWKT);
  //printf( "%s\n", pszWKT );
  CharacterVector out =  Rcpp::CharacterVector::create(pszWKT);
  CPLFree(pszWKT);

  return out;
}
