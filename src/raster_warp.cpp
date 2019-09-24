
#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_priv.h"
#include "gdalwarper.h"
#include "cpl_conv.h" // for CPLMalloc()

// [[Rcpp::export]]
List warp_memory_cpp(CharacterVector source_filename,
                           CharacterVector source_WKT,
                           CharacterVector target_WKT,
                           NumericVector target_geotransform,
                           IntegerVector target_dim,
                           IntegerVector band) {
  GDALDatasetH hSrcDS;
  GDALDatasetH hDstDS;
  GDALRasterBandH poBand, dstBand;
  GDALDriverH hDriver;
  GDALDataType eDT;

  // Open input and output files.
  GDALAllRegister();
  hSrcDS = GDALOpen( source_filename[0], GA_ReadOnly );
  CPLAssert( hSrcDS != NULL );

  if (source_WKT[0].empty()) {
    // do nothing
  } else {

    Rprintf("setting projection");
    GDALSetProjection( hSrcDS, source_WKT[0] );
  }
  poBand = GDALGetRasterBand(hSrcDS, 1);

  //TODO need type handling for nodata
  //int serr;
  //double no_data = GDALGetRasterNoDataValue(poBand, &serr);
  // Create output with same datatype as first input band.
  eDT = GDALGetRasterDataType(poBand);

  // Get output driver
  hDriver = GDALGetDriverByName( "MEM" );

  // Create the output file.
  hDstDS = GDALCreate( hDriver, "", target_dim[0], target_dim[1],
                       GDALGetRasterCount(hSrcDS), eDT, NULL );
  CPLAssert( hDstDS != NULL );

  // Write out the projection definition.
  GDALSetProjection( hDstDS, target_WKT[0] );
  double GeoTransform[6];
  for (int i = 0; i < 6; i++) GeoTransform[i] = target_geotransform[i];
  GDALSetGeoTransform( hDstDS, GeoTransform );



  // Setup warp options.
  GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
  psWarpOptions->hSrcDS = hSrcDS;
  psWarpOptions->hDstDS = hDstDS;
  psWarpOptions->nBandCount = 1;
  psWarpOptions->panSrcBands =
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
  psWarpOptions->panSrcBands[0] = 1;
  psWarpOptions->panDstBands =
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
  psWarpOptions->panDstBands[0] = 1;
  psWarpOptions->pfnProgress = GDALTermProgress;

  // Establish reprojection transformer.
  psWarpOptions->pTransformerArg =
    GDALCreateGenImgProjTransformer( hSrcDS,
                                     GDALGetProjectionRef(hSrcDS),
                                     hDstDS,
                                     GDALGetProjectionRef(hDstDS),
                                     FALSE, 0.0, 1 );
  psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

  // Initialize and execute the warp operation.
  GDALWarpOperation oOperation;
  oOperation.Initialize( psWarpOptions );
  oOperation.ChunkAndWarpImage( 0, 0,
                                GDALGetRasterXSize( hDstDS ),
                                GDALGetRasterYSize( hDstDS ) );
  GDALDestroyGenImgProjTransformer( psWarpOptions->pTransformerArg );
  GDALDestroyWarpOptions( psWarpOptions );
  double *double_scanline;
  double_scanline = (double *) CPLMalloc(sizeof(double)*
    static_cast<unsigned long>(target_dim[0])*
    static_cast<unsigned long>(target_dim[1]));
  //GDALRasterIOExtraArg psExtraArg;
  //INIT_RASTERIO_EXTRA_ARG(psExtraArg);

  CPLErr err;
  dstBand = GDALGetRasterBand(hDstDS, band[0]);

  err = GDALRasterIO(dstBand,  GF_Read, 0, 0, target_dim[0], target_dim[1],
                          double_scanline, target_dim[0], target_dim[1], GDT_Float64,
                          0, 0);
  NumericVector res(target_dim[0] * target_dim[1]);
  for (int i = 0; i < (target_dim[0] * target_dim[1]); i++) {
  //  if (((float)double_scanline[i] == (float)no_data) || ISNAN(double_scanline[i])) {
  //    res[i] = NA_REAL;
  //  } else {
      res[i] = double_scanline[i];
  //  }
  }

  GDALClose( hDstDS );
  GDALClose( hSrcDS );
  Rcpp::List outlist(1);  //hardcode to 1 for now
  outlist[0] = res;

  return outlist;
}
