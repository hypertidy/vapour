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


namespace gdalwarpmem{


using namespace Rcpp;


inline List gdal_warp_in_memory(CharacterVector source_filename,
                     CharacterVector source_WKT,
                     CharacterVector target_WKT,
                     NumericVector target_geotransform,
                     IntegerVector target_dim,
                     IntegerVector band) {


  // TODO
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
  po_SrcDS = static_cast<GDALDatasetH *>(CPLRealloc(po_SrcDS, sizeof(GDALDatasetH) * source_filename.size()));

  GDALDatasetH hDstDS;
  GDALRasterBandH poBand, dstBand;
  GDALDriverH hDriver;
  GDALDataType eDT;

  // Open input and output files.
  GDALAllRegister();
  Rcpp::CharacterVector oo;
  std::vector <char *> oo_char; // = create_options(oo, true); // open options

  for (int i = 0; i < source_filename.size(); i++) {
    po_SrcDS[i] = GDALOpenEx((const char *) source_filename[i], GA_ReadOnly, NULL, oo_char.data(), NULL);
    CPLAssert( po_SrcDS[i] != NULL );
    if (source_WKT[0].empty()) {
      // do nothing
    } else {
     if (i == 0) {
       Rprintf("setting projection");
     }

    GDALSetProjection( po_SrcDS[i], source_WKT[0] );
   }
    //Rprintf("%i\n", i);
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
  GDALWarpAppOptions* psOptions = GDALWarpAppOptionsNew(options_char.data(), NULL);

  int err_0 = 0;

  GDALDatasetH hOutDS = GDALWarp(NULL, hDstDS, 1, po_SrcDS, psOptions, &err_0);


   double *double_scanline;
   double_scanline = (double *) CPLMalloc(sizeof(double)*
     static_cast<unsigned long>(target_dim[0])*
     static_cast<unsigned long>(target_dim[1]));

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
  for (int i = 0; i < source_filename.size(); i++) {
   GDALClose( po_SrcDS[i] );
  }
  CPLFree(double_scanline);

  return outlist;
}

} // namespace gdalwarpmem
#endif
