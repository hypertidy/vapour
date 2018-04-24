#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_priv.h"
//#include "gdalwarper.h"
//#include "cpl_conv.h" // for CPLMalloc()

List gdal_info (const char* pszFilename)
{
  GDALDataset  *poDataset;
  GDALAllRegister();
  poDataset = (GDALDataset *) GDALOpen( pszFilename, GA_ReadOnly );
  if( poDataset == NULL )
  {
    Rcpp::stop("cannot open dataset");
  }
  char **papszSubdatasets = GDALGetMetadata( poDataset, "SUBDATASETS" );
  int nSubdatasets = CSLCount( papszSubdatasets );


  Rcpp::IntegerVector nsubds(1);
  nsubds[0] = nSubdatasets;
  Rcpp::List out(1);
  Rcpp::CharacterVector outnames(1);
  out[0] = nsubds;
  outnames[0] = "sds";
  out.attr("names") = outnames;
  return(out);
}
