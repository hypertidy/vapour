#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_priv.h"

#include "gdal.h"

// [[Rcpp::export]]
CharacterVector  VSI_list(CharacterVector urlpath)
{
  char **VSI_paths = VSIReadDirEx(urlpath[0], 0);
  int ipath = 0; // iterate though MetadataDomainList
  while (VSI_paths && VSI_paths[ipath] != NULL) {
    ipath++;
  }

  Rcpp::CharacterVector names(ipath);
  for (int i = 0; i < ipath; i++) {
    names[i] = VSI_paths[i];
  }
  CSLDestroy(VSI_paths);
  return names;

}
