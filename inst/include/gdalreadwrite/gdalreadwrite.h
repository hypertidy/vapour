#ifndef GDALREADWRITE_H
#define GDALREADWRITE_H

#include <Rcpp.h>

#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()
#include "gdallibrary/gdallibrary.h"
#include "gdal_utils.h"  // for GDALWarpAppOptions

namespace gdalreadwrite{

using namespace Rcpp;



inline GDALDataType init_datatype(CharacterVector datatype) {
  // Byte/Int8/Int16/UInt16/
  //UInt32/Int32/UInt64/Int64/
  // Float32/Float64/CInt16/CInt32/
  // CFloat32/CFloat64
  // Unknown
  if (datatype[0] == "Byte") {
    return GDT_Byte;
  }
  
  if (datatype[0] == "Int16") {
    return GDT_Int16;
  }
  if (datatype[0] == "UInt16") {
    return GDT_UInt16;
  }
  if (datatype[0] == "UInt32") {
    return GDT_UInt32;
  }
  if (datatype[0] == "Int32") {
    return GDT_Int32;
  }

  if (datatype[0] == "Float32") {
    return GDT_Float32;
  }
  if (datatype[0] == "Float64") {
    return GDT_Float64;
  }
  if (datatype[0] == "CInt16") {
    return GDT_CInt16;
  }
  if (datatype[0] == "CInt32") {
    return GDT_CInt32;
  }
  if (datatype[0] == "CFLoat32") {
    return GDT_CFloat32;
  }
  if (datatype[0] == "CFloat64") {
    return GDT_CFloat64;
  }
#if (GDAL_VERSION_MAJOR >= 3 && GDAL_VERSION_MINOR >= 7)
  if (datatype[0] == "Int8") {
    return GDT_Int8;
  }
#endif
  
#if (GDAL_VERSION_MAJOR >= 3 && GDAL_VERSION_MINOR >= 5)
  
  if (datatype[0] == "UInt64") {
    return GDT_UInt64;
  }
  if (datatype[0] == "Int64") {
    return GDT_Int64;
  }
#endif
  
 // Rcpp::stop("datatype not suppported %s\n", datatype[0]);
  return GDT_Unknown;
}



inline CharacterVector gdal_create(CharacterVector filename, CharacterVector driver,
                                   NumericVector extent, IntegerVector dimension,
                                   CharacterVector projection,
                                   IntegerVector n_bands, 
                                   CharacterVector datatype,
                                   CharacterVector options) {
  
  // const char *pszFormat;
  // pszFormat = (const char *)driver[0];

  GDALDataType gdt_type = init_datatype(datatype); 
  //GDALDataType gdt_type = GDT_Float32; 
  OGRSpatialReference* oTargetSRS = nullptr;
  oTargetSRS = new OGRSpatialReference;
  OGRErr target_chk =  oTargetSRS->SetFromUserInput(projection[0]);
  if (target_chk != OGRERR_NONE) {
    if (oTargetSRS != nullptr) {
      delete oTargetSRS;
    }
    Rcpp::stop("cannot initialize target projection");
  }
  
  char **papszOptions = NULL;
  if (options.size() > 0) {
  for (int i = 0; i < options.size(); i++) {
// do this in R
    // if (EQUAL(options[i], "-co") || CSLPartialFindString(options[i], "=") > -1) {
    //   Rcpp::warning("create options should not include '-co' or '='")
    // }
    Rprintf("option: %s\n", (const char *)options[i]); 
    papszOptions = CSLAddString(papszOptions, options[i]); 
  }
  }
    // if (driver[0] == "GTiff") {
  //   papszOptions = CSLSetNameValue( papszOptions, "SPARSE_OK", "YES" );
  //   papszOptions = CSLSetNameValue( papszOptions, "TILED", "YES" );
  //   papszOptions = CSLSetNameValue( papszOptions, "BLOCKXSIZE", "512" );
  //   papszOptions = CSLSetNameValue( papszOptions, "BLOCKYSIZE", "512" );
  //   papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "DEFLATE" );
  // }
 
 
 
    GDALDriver *poDriver;
  poDriver = GetGDALDriverManager()->GetDriverByName("VRT");
  if( poDriver == NULL ) {
    return Rcpp::CharacterVector::create(NA_STRING); 
  }
  // char **papszMetadata;
  // papszMetadata = poDriver->GetMetadata();
  // if( CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATE, FALSE ) ) {
  //   Rprintf( "Driver %s supports Create() method.\n", (const char *)driver[0]);
  // } else {
  //   Rprintf( "Driver %s does not support Create() method.\n", (const char *)driver[0]);
  //   return Rcpp::CharacterVector::create(NA_STRING); 
  // }
  GDALDataset *poVrtDS;
  poVrtDS = poDriver->Create("", dimension[0], dimension[1], n_bands[0], gdt_type, NULL);
  if (poVrtDS == NULL) {
    Rprintf( "Failed to Create virtual datase\n");
    
    return Rcpp::CharacterVector::create(NA_STRING); 
  }
  
  double adfGeoTransform[6] = { extent[0], (extent[1] - extent[0])/ dimension[0], 0, 
                                extent[3], 0, (extent[2] - extent[3])/ dimension[1]};
  poVrtDS->SetGeoTransform( adfGeoTransform );
  poVrtDS->SetSpatialRef(oTargetSRS); 
  
  char **papszMetadata;
  GDALDriver *outDriver; 
  outDriver = GetGDALDriverManager()->GetDriverByName(driver[0]); 
  
  papszMetadata = outDriver->GetMetadata();
  if( !CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATECOPY, FALSE ) ) {
    if( poVrtDS != NULL )
      GDALClose( (GDALDatasetH) poVrtDS );
    Rcpp::stop("driver does not support CreateCopy: %s", driver); 
    
  }
  
  GDALDataset *poDS; 
  poDS = outDriver->CreateCopy(filename[0], poVrtDS,  false, papszOptions, NULL, NULL); 
  if( poDS != NULL )
    GDALClose( (GDALDatasetH) poDS );
  
  if( poVrtDS != NULL )
    GDALClose( (GDALDatasetH) poVrtDS );
  if (oTargetSRS != nullptr) {
    delete oTargetSRS;
  }
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
                            CharacterVector band_output_type, LogicalVector unscale) {
  IntegerVector window(6);
  window[0] = offset[0];
  window[1] = offset[1];
  window[2] = dimension[0];
  window[3] = dimension[1];
  window[4] = dimension[0];
  window[5] = dimension[1];
  return   gdalraster::gdal_raster_io(dsn, window, band, "nearestneighbour", band_output_type, unscale);
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
  
  double *padScanline;
  padScanline = (double *) CPLMalloc(sizeof(double) * static_cast<size_t>(dimension[0] * dimension[1]));
  for (int i = 0; i < data.length(); i++) {
    padScanline[i] = data[i];
  }
  CPLErr err;
  err = poBand->RasterIO( GF_Write, offset[0], offset[1], dimension[0], dimension[1],
                          padScanline, dimension[0], dimension[1], GDT_Float64,
                          0, 0);
  GDALClose(poDataset);
  CPLFree(padScanline);
  LogicalVector out(1);
  out[0] = err == CE_None;
  return out;
}

}// namespace gdalreadwrite
#endif
