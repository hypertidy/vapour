#include <cpp11.hpp>
#include "gdallibrary/gdallibrary.h"

using namespace cpp11;

[[cpp11::register]]
integers set_gdal_config_cpp(strings option, strings value)
{
  gdallibrary::gdal_set_config_option(option, value);
  writable::integers out(1);
  out[0] = 1;
  return out;
}

[[cpp11::register]]
strings get_gdal_config_cpp(strings option){
  return gdallibrary::gdal_get_config_option(option);
}
