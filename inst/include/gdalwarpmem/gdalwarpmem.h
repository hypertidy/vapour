#ifndef GDALWARPMEM_H
#define GDALWARPMEM_H
#include <Rcpp.h>
#include "gdal.h"
#include "gdal_alg.h"
#include "gdal_alg_priv.h"
#include "gdal_priv.h"
#include "gdalwarper.h"
#include "gdal_utils.h"
//#include "gdal_version.h"

#include "cpl_conv.h" // for CPLMalloc()
#include "cpl_string.h" // for CSLAddString

namespace gdalwarpmem{


using namespace Rcpp;

inline List gdal_warp_in_memory(CharacterVector source_filename,
                                CharacterVector source_WKT,
                                CharacterVector target_WKT,
                                NumericVector target_geotransform,
                                IntegerVector target_dim,
                                IntegerVector band,
                                NumericVector source_geotransform,
                                CharacterVector resample) {


  // TODO
  // need input override for source geotransform
  // resample algorithm (options)
  // data type (see paleolimbot/pkd ?)
  // options options options
  // correct treatment of multiple input source_filename
  // set NODATA
  // allow choice of driver, output file
  // DONE multiple bands
  // DONE band selection
  //std::vector<GDALDatasetH> po_SrcDS(source_filename.size());
  GDALDatasetH *po_SrcDS;
  po_SrcDS = static_cast<GDALDatasetH *>(CPLRealloc(po_SrcDS, sizeof(GDALDatasetH) * 1)); //source_filename.size()));

  GDALDatasetH hDstDS;
  GDALRasterBandH poBand, dstBand;
  GDALDriverH hDriver;
  GDALDataType eDT;

  // Open input and output files.
  GDALAllRegister();
  Rcpp::CharacterVector oo;
  std::vector <char *> oo_char; // = create_options(oo, true); // open options

  //  for (int i = 0; i < source_filename.size(); i++) {

  po_SrcDS[0] = GDALOpenEx((const char *) source_filename[0], GA_ReadOnly, NULL, oo_char.data(), NULL);
  CPLAssert( po_SrcDS[0] != NULL );
  if (source_WKT[0].empty()) {
    //      When a projection definition is not available an empty (but not NULL) string is returned.
    const char* srcproj = nullptr;
    srcproj = GDALGetProjectionRef(po_SrcDS[0]);
    if ((srcproj != NULL) && (srcproj[0] == '\0')) {
      // close up any we opened
      //for (int ii = 0; ii <= i; ii++) {
      GDALClose(po_SrcDS[0]);
      //  }
      Rcpp::stop("no valid source projection in source\n");
    }
  } else {
    //     if (i == 0) {
    Rprintf("setting projection");
    //     }
    GDALSetProjection( po_SrcDS[0], source_WKT[0] );
  }

  if (source_geotransform.length() == 1) {
  } else {
    double SourceGeoTransform[6];
    for (int ii = 0; ii < 6; ii++) SourceGeoTransform[ii] = source_geotransform[ii];
    GDALSetGeoTransform( po_SrcDS[0], SourceGeoTransform );
  }

  //TODO need type handling for nodata
  //int serr;
  //double no_data = GDALGetRasterNoDataValue(poBand, &serr);
  // Create output with same datatype as first input band.
  poBand = GDALGetRasterBand(po_SrcDS[0], 1);

  eDT = GDALGetRasterDataType(poBand);
  // Get output driver
  hDriver = GDALGetDriverByName( "MEM" );

  // Create the output data set.
  hDstDS = GDALCreate( hDriver, "", target_dim[0], target_dim[1],
                       GDALGetRasterCount(po_SrcDS[0]), eDT, NULL );

  CPLAssert( hDstDS != NULL );
  // Write out the projection definition.
  GDALSetProjection( hDstDS, target_WKT[0] );
  // and the extent
  double GeoTransform[6];
  for (int i = 0; i < 6; i++) GeoTransform[i] = target_geotransform[i];
  GDALSetGeoTransform( hDstDS, GeoTransform );

  Rcpp::CharacterVector options;
  std::vector <char *> options_char; //create_options(options, true);

  char** papszArg = nullptr;
  //c("nearestneighbour", "bilinear", "cubic", "cubicspline", "lanczos", "average", "mode",
  //"max", "min", "med", "q1", "q3", "sum", "rms")
  papszArg = CSLAddString(papszArg, "-r");
  papszArg = CSLAddString(papszArg, resample[0]);

  GDALWarpAppOptions* psOptions = GDALWarpAppOptionsNew(papszArg, NULL);
  int err_0 = 0;
  GDALDatasetH hOutDS = GDALWarp(NULL, hDstDS, 1, po_SrcDS, psOptions, &err_0);
  double *double_scanline;
  double_scanline = (double *) CPLMalloc(sizeof(double)*static_cast<unsigned long>(target_dim[0]) * static_cast<unsigned long>(target_dim[1]));

  CPLErr err;
  Rcpp::List outlist(band.size());
  for (int iband = 0; iband < band.size(); iband++) {
    dstBand = GDALGetRasterBand(hOutDS, band[iband]);
    err = GDALRasterIO(dstBand,  GF_Read, 0, 0, target_dim[0], target_dim[1],
                       double_scanline, target_dim[0], target_dim[1], GDT_Float64,
                       0, 0);
    NumericVector res(target_dim[0] * target_dim[1]);
    for (int i = 0; i < (target_dim[0] * target_dim[1]); i++) {
      res[i] = double_scanline[i];
    }
    outlist[iband] = res;
  }
  GDALClose( hDstDS );
  //  for (int i = 0; i < source_filename.size(); i++) {
  GDALClose( po_SrcDS[0] );
  //  }
  CPLFree(double_scanline);
  GDALWarpAppOptionsFree(psOptions);
  return outlist;
}

} // namespace gdalwarpmem
#endif
