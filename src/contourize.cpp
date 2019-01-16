// #include <Rcpp.h>
// using namespace Rcpp;
//
// #include "gdal_priv.h"
// #include "gdal_alg.h"
//
// #define MEMORY_FILENAME "/vsimem/example.dat"
//
// // [[Rcpp::export]]
// List contourize(CharacterVector filename, NumericVector levels)
// {
//   GDALDatasetH hDataset;
//   GDALAllRegister();
//   // hDataset = GDALOpenEx(filename[0], GA_ReadOnly, nullptr, NULL, nullptr);
//   if( hDataset == nullptr )
//   {
//     Rcpp::stop("cannot open dataset");
//   }
//
//   // vector output
//   const char *pszDriverName = "Memory";
//   GDALDriver *memDriver;
//   memDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName );
//   if( memDriver == NULL )
//   {
//     printf( "%s driver not available.\n", pszDriverName );
//     exit( 1 );
//   }
//
//   GDALDataset *memDS = memDriver->Create(MEMORY_FILENAME);
//
//   GDALRasterBandH *band =   GDALGetRasterBand(hDataset,   1);
//   GDALContourGenerate(&band, 10, 0,
//                       0, NULL,
//                       false, 0.0,
//                       &memDS,
//                       -1, -1, NULL, NULL);
//
//   // GDALRasterBandH 	hBand,
//   // double 	dfContourInterval,
//   // double 	dfContourBase,
//   // int 	nFixedLevelCount,
//   // double * 	padfFixedLevels,
//   // int 	bUseNoData,
//   // double 	dfNoDataValue,
//   // void * 	hLayer,
//   // int 	iIDField,
//   // int 	iElevField,
//   // GDALProgressFunc 	pfnProgress,
//   // void * 	pProgressArg
//   //
//   // close up
//   GDALClose( hDataset );
//
//
//  Rcpp::List out;
//  return out;
// }
