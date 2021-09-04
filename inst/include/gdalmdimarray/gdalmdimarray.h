#ifndef GDALMDIMARRAY_H
#define GDALMDIMARRAY_H
#include <Rcpp.h>
#include "gdal_priv.h"

namespace gdalmdimarray {
using namespace Rcpp;

inline IntegerVector gdalmdimarray(CharacterVector dsn, CharacterVector array) 
  {

  // //GDALAllRegister();
  auto poDataset = std::unique_ptr<GDALDataset>(
     GDALDataset::Open( dsn[0], GDAL_OF_MULTIDIM_RASTER ));
   if( !poDataset )
   {
     Rcpp::stop("no data open\n"); 
  }
   auto poRootGroup = poDataset->GetRootGroup();
  if( !poRootGroup )
   {
     Rcpp::stop("no root group getted\n"); 
  }
  const char* arrayname[12] = {"analysed_sst"}; 
  auto poVar = poRootGroup->OpenMDArray("analysed_sst");
  if( !poVar )
  {
      //Rprintf("no open array %s", array[0]);
      Rcpp::stop(""); 
  }
  size_t nValues = 1;
  std::vector<size_t> anCount;
  for( const auto poDim: poVar->GetDimensions() )
  {
     anCount.push_back(static_cast<size_t>(poDim->GetSize()));
     nValues *= anCount.back();
  }
  int dim = anCount.max_size(); 
  // std::vector<double> values(nValues);
  // poVar->Read(std::vector<GUInt64>{0,0,0}.data(),
  //              anCount.data(),
  //              nullptr, /* step: defaults to 1,1,1 */
  //              nullptr, /* stride: default to row-major convention */
  //              GDALExtendedDataType::Create(GDT_Float64),
  //              &values[0]);
  IntegerVector dt(2);
  dt[0] = nValues;
  dt[1] = dim; 
  return dt;
}

} // namespace gdalmdimarray
#endif
