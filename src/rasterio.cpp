#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_priv.h"
#include "gdalwarper.h"
#include "cpl_conv.h" // for CPLMalloc()

// [[Rcpp::export]]
List raster_info_cpp (const char* pszFilename)
{
  GDALDataset  *poDataset;
  GDALAllRegister();
  poDataset = (GDALDataset *) GDALOpen( pszFilename, GA_ReadOnly );
  if( poDataset == NULL )
  {
    Rcpp::stop("cannot open dataset");
  }


  //poDataset->GetMetadata();

  double        adfGeoTransform[6];

  poDataset->GetGeoTransform( adfGeoTransform );

  // bail out NOW (we have no SDS and/or no rasters)
  // #f <- system.file("h5ex_t_enum.h5", package = "h5")

  // #raster_sds_info(f)
  // #raster_info(f)
  if (poDataset->GetRasterCount() < 1) {
    Rcpp::stop("no rasters found in dataset");
  }
  // Rprintf( "Size is %dx%dx%d\n",
  //         poDataset->GetRasterXSize(), poDataset->GetRasterYSize(),
  //         poDataset->GetRasterCount() );


  Rcpp::DoubleVector trans(6);
  for (int ii = 0; ii < 6; ii++) trans[ii] = adfGeoTransform[ii];



  GDALRasterBand  *poBand;
  int             nBlockXSize, nBlockYSize;
  int             bGotMin, bGotMax;
  double          adfMinMax[2];

  poBand = poDataset->GetRasterBand( 1 );

  // if we don't bail out above with no rasters things go bad here
  poBand->GetBlockSize( &nBlockXSize, &nBlockYSize );

  adfMinMax[0] = poBand->GetMinimum( &bGotMin );
  adfMinMax[1] = poBand->GetMaximum( &bGotMax );
  if( ! (bGotMin && bGotMax) )
    GDALComputeRasterMinMax((GDALRasterBandH)poBand, TRUE, adfMinMax);

  int nXSize = poBand->GetXSize();
  int nYSize = poBand->GetYSize();
  int nn = 6;
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

  out[4] = Rcpp::CharacterVector::create(proj);
  names[4] = "projection";
  out.attr("names") = names;

  // get band number
  int nBands = poDataset->GetRasterCount();
  out[5] = nBands;
  names[5] = "bands";
  // close up
  GDALClose( (GDALDatasetH) poDataset );

  return out;

}


// [[Rcpp::export]]
List raster_io_cpp(CharacterVector filename,
                            IntegerVector window,
                            IntegerVector band = 1,
                            CharacterVector resample = "nearestneighbour")
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
  GDALDataType band_type =  poBand->GetRasterDataType();

  if( poBand == NULL )
  {
    Rcpp::stop("cannot get band");
  }

  // how to do this is here:
  // https://stackoverflow.com/questions/45978178/how-to-pass-in-a-gdalresamplealg-to-gdals-rasterio
  GDALRasterIOExtraArg psExtraArg;
  INIT_RASTERIO_EXTRA_ARG(psExtraArg);

  if (resample[0] == "average") {
    psExtraArg.eResampleAlg = GRIORA_Average;
  }
  if (resample[0] == "bilinear") {
    psExtraArg.eResampleAlg = GRIORA_Bilinear;
  }
  if (resample[0] == "cubic") {
    psExtraArg.eResampleAlg = GRIORA_Cubic;
  }

  if (resample[0] == "cubicspline") {
    psExtraArg.eResampleAlg = GRIORA_CubicSpline;
  }
  if (resample[0] == "gauss") {
    psExtraArg.eResampleAlg = GRIORA_Gauss;
  }
  if (resample[0] == "lanczos") {
    psExtraArg.eResampleAlg = GRIORA_Lanczos;
  }
  if (resample[0] == "mode") {
    psExtraArg.eResampleAlg = GRIORA_Mode;
  }
  if (resample[0] == "nearestneighbour") {
    psExtraArg.eResampleAlg = GRIORA_NearestNeighbour;
  }


  double *double_scanline;
  int    *integer_scanline;

  List out(1);
  CPLErr err;

  bool band_type_not_supported = true;
  // here we catch byte, int* as R's 32-bit integer
  // or Float32/64 as R's 64-bit numeric
  if ((band_type == GDT_Byte) |
      (band_type == GDT_Int16) |
      (band_type == GDT_Int32) |
      (band_type == GDT_UInt16) |
      (band_type == GDT_UInt32)) {
    integer_scanline = (int *) CPLMalloc(sizeof(int)*
            static_cast<unsigned long>(outXSize)*
            static_cast<unsigned long>(outYSize));
    err = poBand->RasterIO( GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                            integer_scanline, outXSize, outYSize, GDT_Int32,
                            0, 0, &psExtraArg);
    IntegerVector res(outXSize*outYSize);
    for (int i = 0; i < (outXSize*outYSize); i++) res[i] = integer_scanline[i];
    out[0] = res;
    band_type_not_supported = false;
  }
  if ((band_type == GDT_Float64) | (band_type == GDT_Float32)) {
    double_scanline = (double *) CPLMalloc(sizeof(double)*
            static_cast<unsigned long>(outXSize)*
            static_cast<unsigned long>(outYSize));
    err = poBand->RasterIO( GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                            double_scanline, outXSize, outYSize, GDT_Float64,
                            0, 0, &psExtraArg);
    NumericVector res(outXSize*outYSize);
    for (int i = 0; i < (outXSize*outYSize); i++) res[i] = double_scanline[i];
    out[0] = res;

    band_type_not_supported = false;
  }


  // safe but lazy way of not supporting Complex, TypeCount or Unknown types
  // (see GDT_ checks above)
  if (band_type_not_supported) {
    Rcpp::stop("band type not supported (is it Complex? report at hypertidy/vapour/issues)");
  }
  if(err != CE_None) {
    // Report failure somehow.
    Rcpp::stop("raster read failed");
  }
  // close up
  GDALClose( (GDALDatasetH) poDataset );

  return out;
}




