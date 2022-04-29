#include "cpl_conv.h"
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
IntegerVector gdal_set_config_option(CharacterVector option, CharacterVector value) 
{
 CPLSetConfigOption( option[0], value[0] );
  

 return 1;
}

// [[Rcpp::export]]
CharacterVector gdal_get_config_option(CharacterVector option){
  CharacterVector out(1);
  const char *str = CPLGetConfigOption(option[0], nullptr);
  if (str) 
    {
    out[0] = str;
    }
  return out;
}
