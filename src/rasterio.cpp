#include <Rcpp.h>
using namespace Rcpp;


#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()


//' Raster IO
//'
//' @noRd
//' @examples
//' f <- system.file("extdata", "volcano.tif", package = "vapour")
//' fact <- 2
//' m <- matrix(rasterio(f, fact), ncol(volcano)/fact)
//' image(m)
//' contour(t(volcano), add = TRUE)
NumericVector rasterio (const char* pszFilename, NumericVector subfact = 5)
{

  // need a check for subfact >= 0 :)
  GDALDataset  *poDataset;
  GDALAllRegister();
  poDataset = (GDALDataset *) GDALOpen( pszFilename, GA_ReadOnly );
  if( poDataset == NULL )
  {
    return(0);
  }



  double        adfGeoTransform[6];
  printf( "Driver: %s/%s\n",
          poDataset->GetDriver()->GetDescription(),
          poDataset->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );
  printf( "Size is %dx%dx%d\n",
          poDataset->GetRasterXSize(), poDataset->GetRasterYSize(),
          poDataset->GetRasterCount() );
  if( poDataset->GetProjectionRef()  != NULL )
    printf( "Projection is `%s'\n", poDataset->GetProjectionRef() );
  if( poDataset->GetGeoTransform( adfGeoTransform ) == CE_None )
  {
    printf( "Origin = (%.6f,%.6f)\n",
            adfGeoTransform[0], adfGeoTransform[3] );
    printf( "Pixel Size = (%.6f,%.6f)\n",
            adfGeoTransform[1], adfGeoTransform[5] );

  }


  GDALRasterBand  *poBand;
  int             nBlockXSize, nBlockYSize;
  int             bGotMin, bGotMax;
  double          adfMinMax[2];
  poBand = poDataset->GetRasterBand( 1 );
  poBand->GetBlockSize( &nBlockXSize, &nBlockYSize );
  printf( "Block=%dx%d Type=%s, ColorInterp=%s\n",
          nBlockXSize, nBlockYSize,
          GDALGetDataTypeName(poBand->GetRasterDataType()),
          GDALGetColorInterpretationName(
            poBand->GetColorInterpretation()) );
  adfMinMax[0] = poBand->GetMinimum( &bGotMin );
  adfMinMax[1] = poBand->GetMaximum( &bGotMax );
  if( ! (bGotMin && bGotMax) )
    GDALComputeRasterMinMax((GDALRasterBandH)poBand, TRUE, adfMinMax);
  printf( "Min=%.3fd, Max=%.3f\n", adfMinMax[0], adfMinMax[1] );
  if( poBand->GetOverviewCount() > 0 )
    printf( "Band has %d overviews.\n", poBand->GetOverviewCount() );
  if( poBand->GetColorTable() != NULL )
    printf( "Band has a color table with %d entries.\n",
            poBand->GetColorTable()->GetColorEntryCount() );






   float *pafScanline;
   int nXSize = poBand->GetXSize();
   int nYSize = poBand->GetYSize();
   int sub = subfact[0];

   float f_outXSize = nXSize/sub;
   float f_outYSize = nYSize/sub;

   int outXSize = f_outXSize;
   int outYSize = f_outYSize;

   if (outXSize < 1) outXSize = 1;
   if (outYSize < 1) outYSize = 1;



   pafScanline = (float *) CPLMalloc(sizeof(float)*outXSize*outYSize);
   poBand->RasterIO( GF_Read, 0, 0, nXSize, nYSize,
                     pafScanline, outXSize, outYSize, GDT_Float32,
                     0, 0 );

   NumericVector out(outXSize*outYSize);
   for (int i = 0; i < (outXSize*outYSize); i++) out[i] = pafScanline[i];
  return out;
}
