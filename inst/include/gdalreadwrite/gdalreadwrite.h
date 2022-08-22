#ifndef GDALREADWRITE_H
#define GDALREADWRITE_H

#include <Rcpp.h>

#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()
#include "gdallibrary/gdallibrary.h"
#include "gdal_utils.h"  // for GDALWarpAppOptions

namespace gdalreadwrite{

using namespace Rcpp;

inline CharacterVector gdal_create(CharacterVector filename, CharacterVector driver,
                                   NumericVector extent, IntegerVector dimension,
                                   CharacterVector projection,
                                   IntegerVector n_bands) {
  
  // const char *pszFormat;
  // pszFormat = (const char *)driver[0];
  
  OGRSpatialReference* oTargetSRS = nullptr;
  oTargetSRS = new OGRSpatialReference;
  OGRErr target_chk =  oTargetSRS->SetFromUserInput(projection[0]);
  if (target_chk != OGRERR_NONE) {
    delete oTargetSRS;
    Rcpp::stop("cannot initialize target projection");
  }
  
  char **papszOptions = NULL;
  if (driver[0] == "GTiff") {
    papszOptions = CSLSetNameValue( papszOptions, "SPARSE_OK", "YES" );
    papszOptions = CSLSetNameValue( papszOptions, "TILED", "YES" );
    papszOptions = CSLSetNameValue( papszOptions, "BLOCKXSIZE", "512" );
    papszOptions = CSLSetNameValue( papszOptions, "BLOCKYSIZE", "512" );
    papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "DEFLATE" );
  }
 
 
    GDALDriver *poDriver;
  poDriver = GetGDALDriverManager()->GetDriverByName(driver[0]);
  if( poDriver == NULL ) {
    return Rcpp::CharacterVector::create(NA_STRING); 
  }
  char **papszMetadata;
  papszMetadata = poDriver->GetMetadata();
  if( CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATE, FALSE ) ) {
    Rprintf( "Driver %s supports Create() method.\n", (const char *)driver[0]);
  } else {
    Rprintf( "Driver %s does not support Create() method.\n", (const char *)driver[0]);
    return Rcpp::CharacterVector::create(NA_STRING); 
  }
  GDALDataset *poDstDS;
  poDstDS = poDriver->Create(filename[0], dimension[0], dimension[1], n_bands[0], GDT_Float32, papszOptions);
  if (poDstDS == NULL) {
    Rprintf( "Failed to Create %s. \n", (const char *)filename[0]);
    
    return Rcpp::CharacterVector::create(NA_STRING); 
  }
  
  double adfGeoTransform[6] = { extent[0], (extent[1] - extent[0])/ dimension[0], 0, 
                                extent[3], 0, (extent[2] - extent[3])/ dimension[1]};
  poDstDS->SetGeoTransform( adfGeoTransform );
  
  

  poDstDS->SetSpatialRef(oTargetSRS);
  
  GDALClose(poDstDS);
  
  delete oTargetSRS;
  return Rcpp::CharacterVector::create(filename[0]);
}
inline CharacterVector gdal_create_copy(CharacterVector dsource, CharacterVector dtarget, CharacterVector driver) {
  
  const char *pszFormat;
  pszFormat = (const char *)driver[0];
  GDALDriver *poDriver;
  //char **papszMetadata;
  poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
  
  
  GDALDataset *poSrcDS = (GDALDataset *) GDALOpen( dsource[0], GA_ReadOnly );
  if (poSrcDS == NULL ) stop("unable to open raster source for reading: %s", (char *)dsource[0]);
  
  // 
  GDALDataset *poDstDS;
  char **papszOptions = NULL;
  papszOptions = CSLSetNameValue( papszOptions, "SPARSE_OK", "YES" );
  poDstDS = poDriver->CreateCopy( dtarget[0], poSrcDS, FALSE,
                                  papszOptions, NULL, NULL );
  
  /* Once we're done, close properly the dataset */
  if( poDstDS == NULL ) {
    GDALClose( (GDALDatasetH) poSrcDS );
    Rprintf("unable to open raster source for CreateCopy: %s", (char *)dtarget[0]);
    return CharacterVector::create("");
  } else {
    GDALClose( (GDALDatasetH) poDstDS );
  }
  GDALClose( (GDALDatasetH) poSrcDS );
  
  
  return dtarget;
}
inline List gdal_read_block(CharacterVector dsn, IntegerVector offset,
                            IntegerVector dimension, IntegerVector band,
                            CharacterVector band_output_type) {
  IntegerVector window(6);
  window[0] = offset[0];
  window[1] = offset[1];
  window[2] = dimension[0];
  window[3] = dimension[1];
  window[4] = dimension[0];
  window[5] = dimension[1];
  return   gdalraster::gdal_raster_io(dsn, window, band, "nearestneighbour", band_output_type);
}

inline LogicalVector gdal_write_block(CharacterVector dsn, NumericVector data,
                                      IntegerVector offset, IntegerVector dimension, IntegerVector band) {
  GDALDataset  *poDataset;
  poDataset = (GDALDataset *) GDALOpen(dsn[0], GA_Update );
  if( poDataset == NULL )
  {
    Rcpp::stop("cannot open\n");
  }
  if (band[0] < 1) { GDALClose(poDataset);  Rcpp::stop("requested band %i should be 1 or greater", band[0]);  }
  int nbands = poDataset->GetRasterCount();
  if (band[0] > nbands) { GDALClose(poDataset);   Rcpp::stop("requested band %i should be equal to or less than number of bands: %i", band[0], nbands); }
  
  GDALRasterBand  *poBand;
  poBand = poDataset->GetRasterBand( band[0] );
  if (poBand == NULL) {
    Rprintf("cannot access band %i", band[0]);
    GDALClose(poDataset);
    Rcpp::stop("");
  }
  
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
  bool ok = true;
  
  if (err) {
    ok = false;
  }
  LogicalVector out(1);
  out[0] = ok;
  return out;
}

}// namespace gdalreadwrite
#endif
