#ifndef GDALREADWRITE_H
#define GDALREADWRITE_H


#include <Rcpp.h>

#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()

namespace gdalreadwrite{

using namespace Rcpp;

inline List gdal_read_block(CharacterVector dsn, IntegerVector offset, IntegerVector dimension, IntegerVector band) {
  GDALDataset  *poDataset;
  GDALAllRegister();
  poDataset = (GDALDataset *) GDALOpen(dsn[0], GA_ReadOnly );
  if( poDataset == NULL )
  {
    Rcpp::stop("cannot open\n");
  }

  // band
  GDALRasterBand  *poBand;
  poBand = poDataset->GetRasterBand( band[0] );

  //rasterio
  float *pafScanline;
  pafScanline = (float *) CPLMalloc(sizeof(float)*dimension[0] * dimension[1]);
  CPLErr err;
  err = poBand->RasterIO( GF_Read, offset[0], offset[1], dimension[0], dimension[1],
                   pafScanline, dimension[0], dimension[1], GDT_Float32,
                   0, 0);

  // finish up
  GDALClose(poDataset);
  List out = Rcpp::List(1);
  NumericVector res(dimension[0] * dimension[1]);
  for (int i = 0; i < res.length(); i++) {
    res[i] = pafScanline[i];
  }
  CPLFree(pafScanline);
  out[0] = res;

  return out;
}


inline LogicalVector gdal_write_block(CharacterVector dsn, NumericVector data,
                                      IntegerVector offset, IntegerVector dimension, IntegerVector band) {
  GDALDataset  *poDataset;
  GDALAllRegister();
  poDataset = (GDALDataset *) GDALOpen(dsn[0], GA_Update );
  if( poDataset == NULL )
  {
    Rcpp::stop("cannot open\n");
  }

  GDALRasterBand  *poBand;
  poBand = poDataset->GetRasterBand( band[0] );

  float *pafScanline;
  pafScanline = (float *) CPLMalloc(sizeof(float)*dimension[0] * dimension[1]);
  for (int i = 0; i < data.length(); i++) {
    pafScanline[i] = data[i];
  }

  CPLErr err;
  err = poBand->RasterIO( GF_Write, offset[0], offset[1], dimension[0], dimension[1],
                          pafScanline, dimension[0], dimension[1], GDT_Float32,
                          0, 0);

  GDALClose(poDataset);
  CPLFree(pafScanline);

  bool ok = false;
  LogicalVector out(1);
  out[0] = ok;
  return out;
}

}// namespace gdalreadwrite
#endif
