#include "gdal_priv.h"
#include "gdal.h"
#include <Rcpp.h>

using namespace Rcpp;


// [[Rcpp::export]]
Rcpp::CharacterVector vapour_gdal_version_cpp()
{
  Rcpp::CharacterVector out(1);

  out[0] = GDALVersionInfo("--version");
  //out[0] = version;
 return out;
}

// [[Rcpp::export]]
Rcpp::List vapour_all_drivers_cpp()
{
  GDALAllRegister();

  int n = GetGDALDriverManager()->GetDriverCount();

  Rcpp::CharacterVector sname(n);
  Rcpp::CharacterVector lname(n);
  Rcpp::LogicalVector isvector(n);
  Rcpp::LogicalVector israster(n);
  Rcpp::LogicalVector iscopy(n);
  Rcpp::LogicalVector iscreate(n);
  Rcpp::LogicalVector isvirt(n);
  for (int idriver = 0; idriver < n; idriver++) {
     GDALDriver *dr = GetGDALDriverManager()->GetDriver(idriver);
     sname(idriver) = GDALGetDriverShortName(dr);
     lname(idriver) = GDALGetDriverLongName(dr);
     isvector(idriver) = (dr->GetMetadataItem(GDAL_DCAP_VECTOR) != NULL);
     israster(idriver) = (dr->GetMetadataItem(GDAL_DCAP_RASTER) != NULL);
     iscopy(idriver) = (dr->GetMetadataItem(GDAL_DCAP_CREATECOPY) != NULL);
     iscreate(idriver) = (dr->GetMetadataItem(GDAL_DCAP_CREATE) != NULL);
     isvirt(idriver) = (dr->GetMetadataItem(GDAL_DCAP_VIRTUALIO) != NULL);


  }
  Rcpp::List out = Rcpp::List::create(Rcpp::Named("driver") = sname,
                                      Rcpp::Named("name") = lname,
                                      Rcpp::Named("vector") = isvector,
                                      Rcpp::Named("raster") = israster,
                                      Rcpp::Named("create") = iscreate,
                                      Rcpp::Named("copy") = iscopy,
                                      Rcpp::Named("virtual") = isvirt);
  return out;
}
