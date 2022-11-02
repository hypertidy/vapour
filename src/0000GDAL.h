#include "gdal_priv.h"
#include "Rcpp.h"

RCPP_MODULE(GDALDataset) {
  using namespace Rcpp;
  class_<GDALDataset>("GDALDataset")
    .constructor<Rcpp::CharacterVector>()
    .method("Open", &GDALDataset::Open)
    .method("GetRasterXSize", &GDALDataset::GetRasterXSize)
    .method("GetRasterYSize", &GDALDataset::GetRasterYSize)
  
}
