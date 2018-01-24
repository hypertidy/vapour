#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_priv.h"
#include "gdalwarper.h"
#include "cpl_conv.h" // for CPLMalloc()

//' Raster IO
//'
//' @param pszFilename data source
//' @export
//' @examples
//' f <- system.file("extdata", "sst.tif", package = "vapour")
//' raster_info(f)
// [[Rcpp::export]]
List raster_info (const char* pszFilename)
{
  GDALDataset  *poDataset;
  GDALAllRegister();
  poDataset = (GDALDataset *) GDALOpen( pszFilename, GA_ReadOnly );
  if( poDataset == NULL )
  {
    Rcpp::stop("cannot open dataset");
  }
poDataset->GetMetadata();
  double        adfGeoTransform[6];
  poDataset->GetGeoTransform( adfGeoTransform );

  Rcpp::DoubleVector trans(6);
  for (int ii = 0; ii < 6; ii++) trans[ii] = adfGeoTransform[ii];

  GDALRasterBand  *poBand;
  int             nBlockXSize, nBlockYSize;
  int             bGotMin, bGotMax;
  double          adfMinMax[2];
  poBand = poDataset->GetRasterBand( 1 );
  poBand->GetBlockSize( &nBlockXSize, &nBlockYSize );
  adfMinMax[0] = poBand->GetMinimum( &bGotMin );
  adfMinMax[1] = poBand->GetMaximum( &bGotMax );
  if( ! (bGotMin && bGotMax) )
    GDALComputeRasterMinMax((GDALRasterBandH)poBand, TRUE, adfMinMax);

  float *pafScanline;
  int nXSize = poBand->GetXSize();
  int nYSize = poBand->GetYSize();
  int nn = 5;
  Rcpp::List out(nn);
  Rcpp::CharacterVector names(nn);
  out[0] = trans;
  names[0] = "geotransform";
  out[1] = Rcpp::IntegerVector::create(nXSize, nYSize);
  names[1] = "dimXY";

  out[2] = Rcpp::DoubleVector::create(adfMinMax[0], adfMinMax[1]);
  names[2] = "minmax";

  out[3] = Rcpp::IntegerVector::create(nBlockXSize, nBlockYSize);
  names[3] = "tilesXY";

  const char *proj;
  proj = poDataset->GetProjectionRef();
  // close up
  GDALClose( (GDALDatasetH) poDataset );

  out[4] = Rcpp::CharacterVector::create(proj);
  names[4] = "projection";
  out.attr("names") = names;
  return out;

}


//' Raster IO (read)
//'
//' Read a window of data from a GDAL raster source. The first argument is the source
//' name and the second is a 6-element `window` of offset, source dimension, and output dimension.
//'
//' This is analgous to the `rgdal` function `readGDAL` with its arguments `offset`,  `region.dim`
//' and `output.dim`.
//' @param filename data source
//' @param window src_offset, src_dim, out_dim
//' @export
//' @examples
//' f <- system.file("extdata", "sst.tif", package = "vapour")
//' ## a 5*5 window from a 10*10 region
//' raster_io(f, window = c(0, 0, 10, 10, 5, 5))
//' ## find the information first
//' #ri <- raster_info(f)
//' #str(matrix(raster_io(f, c(0, 0, ri$dimXY, ri$dimXY)), ri$dimXY[1]))
//' ## the method can be used to up-sample as well
//' #str(matrix(raster_io(f, window = c(0, 0, 10, 10, 15, 25)), 15))
//' ## a future version will provide access to different methods
// [[Rcpp::export]]
NumericVector raster_io(CharacterVector filename, IntegerVector window, IntegerVector band = 1)
{

  int Xoffset = window[0];
  int Yoffset = window[1];
  int nXSize = window[2];
  int nYSize = window[3];

  int outXSize = window[4];
  int outYSize = window[5];

  GDALDataset  *poDataset;
  GDALAllRegister();
  poDataset = (GDALDataset *) GDALOpen( filename[0], GA_ReadOnly );
  if( poDataset == NULL )
  {
    Rcpp::stop("cannot open dataset");
  }

  GDALRasterBand  *poBand;
  poBand = poDataset->GetRasterBand( band[0] );
  if( poBand == NULL )
  {
    Rcpp::stop("cannot get band");
  }
  float *pafScanline;

  //GDALRasterIOExtraArg psExtraArg;
  //INIT_RASTERIO_EXTRA_ARG(psExtraArg);
  // TODO expose the resampling options to user
  //psExtraArg.eResampleAlg = GRIORA_NearestNeighbour;
  pafScanline = (float *) CPLMalloc(sizeof(float)*outXSize*outYSize);
  CPLErr err = poBand->RasterIO( GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                  pafScanline, outXSize, outYSize, GDT_Float32,
                  0, 0);

  if(err != CE_None) {
    // Report failure somehow.
    Rcpp::stop("raster read failed");
  }
  // close up
  GDALClose( (GDALDatasetH) poDataset );
  NumericVector out(outXSize*outYSize);
  for (int i = 0; i < (outXSize*outYSize); i++) out[i] = pafScanline[i];
  return out;
}


