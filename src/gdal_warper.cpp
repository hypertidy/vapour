// #include <Rcpp.h>
// using namespace Rcpp;
//
//
// #include "gdalwarper.h"
// #include "ogr_spatialref.h"
// #include "cpl_conv.h" // for CPLMalloc()
// #include "cpl_string.h"
//
//
//
// #include <vector>
// #include <string>
//
//
// // [[Rcpp::export]]
// CharacterVector raster_warp_gcp (CharacterVector src_filename, CharacterVector dst_filename,
//                              CharacterVector dst_SRSproj, NumericVector dst_dimXY,
//                              NumericVector dst_resolution)
// {
//   GDALDatasetH  hSrcDS, hDstDS;
//   GDALDriverH hDriver;
//   GDALDataType eDT;
//   // Open input and output files.
//   GDALAllRegister();
//
//
//   // Open the source file.
//   hSrcDS = GDALOpen(src_filename[0], GA_ReadOnly );
//   CPLAssert( hSrcDS != NULL );
//
//   // Create output with same datatype as first input band.
//   eDT = GDALGetRasterDataType(GDALGetRasterBand(hSrcDS,1));
//
//   // Get output driver (GeoTIFF format)
//   hDriver = GDALGetDriverByName( "GTiff" );
//   CPLAssert( hDriver != NULL );
//
//   // Get Source coordinate system.
//   const char *pszSrcWKT = NULL;
//   char *pszDstWKT = NULL; //different to GDAL warp API (no const)
//   pszSrcWKT = GDALGetGCPProjection(hSrcDS);
//   CPLAssert( pszSrcWKT != NULL && strlen(pszSrcWKT) > 0 );
//
//   OGRSpatialReference oSRS;
//   oSRS.importFromProj4(dst_SRSproj[0]);
//   oSRS.exportToWkt( &pszDstWKT );
//
//   CPLAssert( pszDstWKT != NULL && strlen(pszDstWKT) > 0 );
//   // More checks here, e.g. https://www.gdal.org/classOGRSpatialReference.html#a16c78a4cb401cccaa82a9e65f1b5d5c4
//   //  oSRS.Validate();
//
//   char **papszMD;
//   papszMD = GDALGetMetadata( hSrcDS, "GEOLOCATION" );
//
//   Rprintf("%s\n", CSLFetchNameValue(papszMD,"PIXEL_STEP"));
//
//   Rprintf("%s\n", CSLFetchNameValue(papszMD,"X_BAND"));
//
//   double adfDstGeoTransform[6];
//   int nPixels=0, nLines=0;
//   CPLErr eErr;
//   nPixels = 2316;
//   nLines = 2554;
//   adfDstGeoTransform[0] = -1157398;
//   adfDstGeoTransform[1] = 1000 ;
//   adfDstGeoTransform[2] = 0;
//   adfDstGeoTransform[3] = 1277541 ;
//   adfDstGeoTransform[4] = 0;
//   adfDstGeoTransform[5] = -1000;
//   // Create the output file.
//   hDstDS = GDALCreate( hDriver, dst_filename[0], nPixels, nLines,
//                        GDALGetRasterCount(hSrcDS), eDT, NULL );
//   CPLAssert( hDstDS != NULL );
//
//   // // Establish reprojection transformer
//   //Write out the projection definition.
//   GDALSetProjection( hDstDS, pszDstWKT );
//   GDALSetGeoTransform( hDstDS, adfDstGeoTransform );
//
//
//   // Setup warp options.
//
//   GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
//   psWarpOptions->hSrcDS = hSrcDS;
//   psWarpOptions->hDstDS = hDstDS;
//   psWarpOptions->nBandCount = 1;
//   psWarpOptions->panSrcBands =
//     (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
//   psWarpOptions->panSrcBands[0] = 1;
//   psWarpOptions->panDstBands =
//     (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
//   psWarpOptions->panDstBands[0] = 1;
//   psWarpOptions->pfnProgress = GDALTermProgress;
//   psWarpOptions->pTransformerArg = GDALCreateGeoLocTransformer(hSrcDS, papszMD, false);
//   psWarpOptions->pfnTransformer = GDALGeoLocTransform;
//
//   // // Copy the color table, if required.
//   GDALColorTableH hCT;
//   hCT = GDALGetRasterColorTable( GDALGetRasterBand(hSrcDS,1) );
//   if( hCT != NULL )
//     GDALSetRasterColorTable( GDALGetRasterBand(hDstDS,1), hCT );
//
//   // Initialize and execute the warp operation.
//   GDALWarpOperation oOperation;
//   oOperation.Initialize( psWarpOptions );
//
//   oOperation.ChunkAndWarpImage( 0, 0,
//                                  GDALGetRasterXSize( hDstDS ),
//                                  GDALGetRasterYSize( hDstDS ) );
//   GDALDestroyGeoLocTransformer( psWarpOptions->pTransformerArg );
//   GDALDestroyWarpOptions( psWarpOptions );
//
//   GDALClose( hDstDS );
//   GDALClose( hSrcDS );
//   return "";
// }
//
//
//
// // [[Rcpp::export]]
// CharacterVector raster_warp (CharacterVector src_filename, CharacterVector dst_filename,
//                              CharacterVector dst_SRSproj, NumericVector dst_dimXY,
//                              NumericVector dst_resolution)
// {
//   GDALDatasetH  hSrcDS, hDstDS;
//   GDALDriverH hDriver;
//   GDALDataType eDT;
//   // Open input and output files.
//   GDALAllRegister();
//
//
//   // Open the source file.
//   hSrcDS = GDALOpen(src_filename[0], GA_ReadOnly );
//   CPLAssert( hSrcDS != NULL );
//   // Create output with same datatype as first input band.
//   eDT = GDALGetRasterDataType(GDALGetRasterBand(hSrcDS,1));
//   // Get output driver (GeoTIFF format)
//   hDriver = GDALGetDriverByName( "GTiff" );
//   CPLAssert( hDriver != NULL );
//   // Get Source coordinate system.
//   const char *pszSrcWKT = NULL;
//   char *pszDstWKT = NULL; //different to GDAL warp API (no const)
//
//   pszSrcWKT = GDALGetProjectionRef( hSrcDS );
// Rprintf("%s\n", pszSrcWKT);
//   CPLAssert( pszSrcWKT != NULL && strlen(pszSrcWKT) > 0 );
//
//   OGRSpatialReference oSRS;
//   //oSRS.SetProjCS(dst_SRSproj[0]);
//   oSRS.importFromProj4(dst_SRSproj[0]);
//   //oSRS.SetUTM( 11, TRUE );
//   //oSRS.SetWellKnownGeogCS( "WGS84" );
//   oSRS.exportToWkt( &pszDstWKT );
//   CPLAssert( pszDstWKT != NULL && strlen(pszDstWKT) > 0 );
//   // More checks here, e.g. https://www.gdal.org/classOGRSpatialReference.html#a16c78a4cb401cccaa82a9e65f1b5d5c4
// //  oSRS.Validate();
//
//
//   // Create a transformer that maps from source pixel/line coordinates
//   // to destination georeferenced coordinates (not destination
//   // pixel line).  We do that by omitting the destination dataset
//   // handle (setting it to NULL).
//   void *hTransformArg;
//   hTransformArg =
//     GDALCreateGenImgProjTransformer( hSrcDS, pszSrcWKT, NULL, pszDstWKT,
//                                      FALSE, 0, 1 );
//
//   CPLAssert( hTransformArg != NULL );
//   // Get approximate output georeferenced bounds and resolution for file.
//   double adfDstGeoTransform[6];
//   int nPixels=0, nLines=0;
//   CPLErr eErr;
//   eErr = GDALSuggestedWarpOutput( hSrcDS,
//                                   GDALGenImgProjTransform, hTransformArg,
//                                   adfDstGeoTransform, &nPixels, &nLines );
//   CPLAssert( eErr == CE_None );
//   GDALDestroyGenImgProjTransformer( hTransformArg );
//
//
//   // Create the output file.
//   hDstDS = GDALCreate( hDriver, dst_filename[0], nPixels, nLines,
//                        GDALGetRasterCount(hSrcDS), eDT, NULL );
//   CPLAssert( hDstDS != NULL );
//
//
//   //Write out the projection definition.
//   GDALSetProjection( hDstDS, pszDstWKT );
//   GDALSetGeoTransform( hDstDS, adfDstGeoTransform );
//   // // Copy the color table, if required.
//   // GDALColorTableH hCT;
//   // hCT = GDALGetRasterColorTable( GDALGetRasterBand(hSrcDS,1) );
//   // if( hCT != NULL )
//   //   GDALSetRasterColorTable( GDALGetRasterBand(hDstDS,1), hCT );
//   //
//   //
//   //
//
//
//
//
//   // Setup warp options.
//   GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
//   psWarpOptions->hSrcDS = hSrcDS;
//   psWarpOptions->hDstDS = hDstDS;
//   psWarpOptions->nBandCount = 1;
//   psWarpOptions->panSrcBands =
//     (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
//   psWarpOptions->panSrcBands[0] = 1;
//   psWarpOptions->panDstBands =
//     (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
//   psWarpOptions->panDstBands[0] = 1;
//   psWarpOptions->pfnProgress = GDALTermProgress;
//   // Establish reprojection transformer.
//   psWarpOptions->pTransformerArg =
//     GDALCreateGenImgProjTransformer( hSrcDS,
//                                      GDALGetProjectionRef(hSrcDS),
//                                      hDstDS,
//                                      GDALGetProjectionRef(hDstDS),
//                                      FALSE, 0.0, 1 );
//   psWarpOptions->pfnTransformer = GDALGenImgProjTransform;
//   // Initialize and execute the warp operation.
//   // GDALWarpOperation oOperation;
//   // oOperation.Initialize( psWarpOptions );
//   // oOperation.ChunkAndWarpImage( 0, 0,
//   //                               GDALGetRasterXSize( hDstDS ),
//   //                               GDALGetRasterYSize( hDstDS ) );
//   // GDALDestroyGenImgProjTransformer( psWarpOptions->pTransformerArg );
//   GDALDestroyWarpOptions( psWarpOptions );
//
//
//
//   GDALClose( hDstDS );
//   GDALClose( hSrcDS );
//   return "";
// }
