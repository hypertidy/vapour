#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_priv.h"
#include "gdalwarper.h"
#include "cpl_conv.h" // for CPLMalloc()
#include "ogr_spatialref.h" // for OGRSpatialReference

#include "gdal.h"  // for GCPs

// [[Rcpp::export]]
List raster_info_cpp (CharacterVector filename, LogicalVector min_max)
{
  GDALDatasetH hDataset;
  GDALAllRegister();
  hDataset = GDALOpenEx(filename[0], GA_ReadOnly, nullptr, NULL, nullptr);

  if( hDataset == nullptr )
  {
    Rcpp::stop("cannot open dataset");
  }
  int nXSize = GDALGetRasterXSize(hDataset);
  int nYSize = GDALGetRasterYSize(hDataset);


  double        adfGeoTransform[6];

  //poDataset->GetGeoTransform( adfGeoTransform );
  GDALGetGeoTransform(hDataset, adfGeoTransform );

  // bail out NOW (we have no SDS and/or no rasters)
  // #f <- system.file("h5ex_t_enum.h5", package = "h5")
  if (GDALGetRasterCount(hDataset) < 1) {
    Rcpp::stop("no rasters found in dataset");
  }
  Rcpp::DoubleVector trans(6);
  for (int ii = 0; ii < 6; ii++) trans[ii] = adfGeoTransform[ii];



  GDALRasterBandH  hBand;
  int             nBlockXSize, nBlockYSize;
  //int             bGotMin, bGotMax;
  double          adfMinMax[2];

  hBand = GDALGetRasterBand(hDataset, 1);
  // if we don't bail out above with no rasters things go bad here
  GDALGetBlockSize(hBand, &nBlockXSize, &nBlockYSize);
  if (min_max[0]) {
    GDALComputeRasterMinMax(hBand, TRUE, adfMinMax);
  }

  int nn = 9;
  Rcpp::List out(nn);
  Rcpp::CharacterVector names(nn);
  out[0] = trans;
  names[0] = "geotransform";
  out[1] = Rcpp::IntegerVector::create(nXSize, nYSize);
  names[1] = "dimXY";

  DoubleVector vmmx(2);
  if (min_max[0]) {
    vmmx[0] = adfMinMax[0];
    vmmx[1] = adfMinMax[1];
  } else {
    vmmx[0] = NA_REAL;
    vmmx[1] = NA_REAL;
  }
  out[2] = vmmx;
  names[2] = "minmax";

  out[3] = Rcpp::IntegerVector::create(nBlockXSize, nBlockYSize);
  names[3] = "tilesXY";

  const char *proj;
  proj = GDALGetProjectionRef(hDataset);
  //https://gis.stackexchange.com/questions/164279/how-do-i-create-ogrspatialreference-from-raster-files-georeference-c
  //char *proj_tmp = (char *) proj;
  out[4] = Rcpp::CharacterVector::create(proj);
  names[4] = "projection";

  // get band number
  int nBands = GDALGetRasterCount(hDataset);

  out[5] = nBands;
  names[5] = "bands";

  //char *stri;
  //OGRSpatialReference oSRS;
  //oSRS.importFromWkt(&proj_tmp);
  //oSRS.exportToProj4(&stri);
  out[6] =  Rcpp::CharacterVector::create(""); //Rcpp::CharacterVector::create(stri);
  names[6] = "proj4";

  int succ;
  out[7] = GDALGetRasterNoDataValue(hBand, &succ);
  names[7] = "nodata_value";

  int ocount = GDALGetOverviewCount(hBand);
// f <- system.file("extdata/volcano_overview.tif", package = "vapour", mustWork = TRUE)
  IntegerVector oviews = IntegerVector(ocount * 2 + 1);
  oviews[0] = ocount;
  if (ocount > 0) {
    GDALRasterBandH  oBand;
    for (int ii = 0; ii < ocount; ii++) {
      oBand = GDALGetOverview(hBand, ii);


      int xsize = GDALGetRasterBandXSize(oBand);
      int ysize = GDALGetRasterBandYSize(oBand);

      oviews[((ii + 1) * 2) - 1 ] = xsize;
      oviews[((ii + 1) * 2) + 0 ] = ysize;



    }
  }
  out[8] = oviews;
  names[8] = "overviews";

  // if (GDALHasArbitraryOverviews(hDataset) > 0) {
  //   //HasArbitraryOverViews
  //   //GetOverviewCount
  //   //GetOverview
  //
  //   out[8] = ocount;
  //
  // }



  out.attr("names") = names;

  //CPLFree(stri);
  // close up
  GDALClose( hDataset );
  return out;

}

// [[Rcpp::export]]
List raster_gcp_cpp(CharacterVector filename) {
  // get GCPs if any
  GDALDatasetH hDataset;
  //GDALDataset  *poDataset;
  GDALAllRegister();
 hDataset = GDALOpenEx( filename[0], GA_ReadOnly, nullptr, NULL, nullptr);
  if( hDataset == nullptr )
  {
    Rcpp::stop("cannot open dataset");
  }

 int gcp_count;
  gcp_count = GDALGetGCPCount(hDataset);
  const char *srcWKT = GDALGetGCPProjection(hDataset);
  Rcpp::List gcpout(6);
  Rcpp::CharacterVector gcpnames(6);
  Rcpp::CharacterVector gcpCRS(1);
  gcpCRS[0] = srcWKT;
  gcpnames[0] = "Pixel";
  gcpnames[1] = "Line";
  gcpnames[2] = "X";
  gcpnames[3] = "Y";
  gcpnames[4] = "Z";
  gcpnames[5] = "CRS";
  gcpout.attr("names") = gcpnames;
  if (gcp_count > 0) {
    Rcpp::NumericVector GCPPixel(gcp_count);
    Rcpp::NumericVector GCPLine(gcp_count);
    Rcpp::NumericVector GCPX(gcp_count);
    Rcpp::NumericVector GCPY(gcp_count);
    Rcpp::NumericVector GCPZ(gcp_count);
    for (int igcp = 0; igcp < gcp_count; ++igcp) {
      const GDAL_GCP *gcp = GDALGetGCPs( hDataset ) + igcp;
      //const GDAL_GCP *gcp = poDataset->GetGCPs() + igcp;
      GCPPixel[igcp] = gcp->dfGCPPixel;
      GCPLine[igcp] = gcp->dfGCPLine;
      GCPX[igcp] = gcp->dfGCPX;
      GCPY[igcp] = gcp->dfGCPY;
      GCPZ[igcp] = gcp->dfGCPZ;
    }
    gcpout[0] = GCPPixel;
    gcpout[1] = GCPLine;
    gcpout[2] = GCPX;
    gcpout[3] = GCPY;
    gcpout[4] = GCPZ;
    gcpout[5] = gcpCRS;
    //gcp_proj = poDataset->GetGCPProjection();
  } else {
    Rprintf("No GCP (ground control points) found.\n");
  }
  GDALClose( hDataset );
  return gcpout;
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




