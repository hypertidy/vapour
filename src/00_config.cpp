#include <Rcpp.h>
#include "gdallibrary/gdallibrary.h"
using namespace Rcpp;
using namespace gdallibrary;

// [[Rcpp::export]]
IntegerVector set_gdal_config_cpp(CharacterVector option, CharacterVector value) 
{
  gdal_set_config_option( option, value );
  return 1;
}

// [[Rcpp::export]]
CharacterVector get_gdal_config_cpp(CharacterVector option){
  return gdal_get_config_option(option);
}
