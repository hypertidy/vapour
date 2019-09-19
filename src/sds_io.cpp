#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_priv.h"
#include "gdalwarper.h"
#include "cpl_conv.h" // for CPLMalloc()


// [[Rcpp::export]]
CharacterVector sds_info_cpp(const char* pszFilename)
{
  GDALDataset  *poDataset;
  GDALAllRegister();
  poDataset = (GDALDataset *) GDALOpen( pszFilename, GA_ReadOnly );
  if( poDataset == NULL )
  {
    Rcpp::stop("cannot open dataset");
  }

  char **MDdomain = GDALGetMetadataDomainList(poDataset);

  int mdi = 0; // iterate though MetadataDomainList
  bool has_sds = false;
  while (MDdomain && MDdomain[mdi] != NULL) {
    if (strcmp(MDdomain[mdi], "SUBDATASETS") == 0) {
      has_sds = true;
    }
    mdi++;
  }
  //cleanup
  CSLDestroy(MDdomain);

  int dscount = 1;
  if (has_sds) {
    char **SDS = GDALGetMetadata(poDataset, "SUBDATASETS");
    int sdi = 0;
    while (SDS && SDS[sdi] != NULL) {
      sdi++; // count
    }
    //this seems to be the wrong context in which to do this?
    //CSLDestroy(SDS);
    dscount = sdi;
  }
  Rcpp::CharacterVector ret(dscount);
  if (has_sds) {
    // we have subdatasets, so list them all
    char **SDS2 = GDALGetMetadata(poDataset, "SUBDATASETS");
    for (int ii = 0; ii < dscount; ii++) {
      ret(ii) = SDS2[ii];
    }
    //this seems to be the wrong context in which to do this?
    //CSLDestroy(SDS2);
  } else {
    ret[0] = pszFilename;
  }
  GDALClose( (GDALDatasetH) poDataset );
  return ret;
}

