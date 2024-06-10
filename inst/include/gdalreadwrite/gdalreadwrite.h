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



inline CharacterVector gdal_create(CharacterVector filename, 
                                   CharacterVector driver,
                                   NumericVector extent, 
                                   IntegerVector dimension,
                                   CharacterVector projection,
                                   IntegerVector n_bands, 
                                   CharacterVector datatype,
                                   List options_list_pairs) {
  
  
  GDALDataType gdt_type = init_datatype(datatype); 
  
  OGRSpatialReference oSRS;
  oSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
  
  if (oSRS.SetFromUserInput(projection[0]) != OGRERR_NONE)
  {
    Rcpp::warning(
      "Failed to process 'projection' definition");
    
  }
  
  char *pszWKT = nullptr;
#if GDAL_VERSION_MAJOR >= 3
  const char *optionsWKT[3] = { "MULTILINE=YES", "FORMAT=WKT2", NULL };
  OGRErr err = oSRS.exportToWkt(&pszWKT, optionsWKT);
#else
  OGRErr err = oSRS.exportToWkt(&pszWKT);
#endif
  
  
  GDALDriverH hDriver;
  hDriver = GDALGetDriverByName(driver[0]);
  if( hDriver == nullptr ) {
    //delete(poTargetSRS); 
    Rcpp::stop("failed to get nominated 'driver'"); 
  }
  
  
  
  
  char **papszOptions = nullptr;
  if (options_list_pairs.size() > 0) {
    for (int i = 0; i < options_list_pairs.size(); i++) {
      CharacterVector options2 = options_list_pairs[i]; 
      if (options2.size() == 2) {
        //Rprintf("options: %s %s\n", (char *)options2[0], (char *)options2[1]); 
        papszOptions = CSLSetNameValue(papszOptions, (char *)options2[0], (char *)options2[1]);
      }
    }
  }
  
  
  
  GDALDatasetH hDS = nullptr;
  hDS = GDALCreate(hDriver, filename[0],
                   dimension[0], dimension[1], n_bands[0], gdt_type,
                   papszOptions);
  
  
  if (hDS == nullptr) {
    Rprintf( "Failed to create dataset\n");
    if (pszWKT != nullptr)  CPLFree(pszWKT); 
     CSLDestroy(papszOptions); 
    
    return Rcpp::CharacterVector::create(NA_STRING);
  }
  
  double adfGeoTransform[6] = { extent[0], (extent[1] - extent[0])/ dimension[0], 0,
                                extent[3], 0, (extent[2] - extent[3])/ dimension[1]};
  GDALSetGeoTransform(hDS, adfGeoTransform );
  GDALSetProjection(hDS, pszWKT); 
  
  if (pszWKT != nullptr) CPLFree(pszWKT); 
  CSLDestroy(papszOptions); 
  
  
  if( hDS != nullptr ) GDALClose( hDS );
  return filename; 
}





inline CharacterVector gdal_create_copy(CharacterVector dsource, CharacterVector dtarget, CharacterVector driver) {
  
  GDALDriver *poDriver;
  //char **papszMetadata;
  poDriver = GetGDALDriverManager()->GetDriverByName(driver[0]);
  
  GDALDataset *poSrcDS = (GDALDataset *) GDALOpen( dsource[0], GA_ReadOnly );
  if (poSrcDS == NULL ) stop("unable to open raster source for reading: %s", (char *)dsource[0]);
  
  // 
  GDALDataset *poDstDS;
  char **papszOptions = nullptr;
  papszOptions = CSLSetNameValue( papszOptions, "SPARSE_OK", "YES" );
  poDstDS = poDriver->CreateCopy( dtarget[0], poSrcDS, FALSE,
                                  papszOptions, NULL, NULL );
  
  /* Once we're done, close properly the dataset */
  if( poDstDS == NULL ) {
    GDALClose( (GDALDatasetH) poSrcDS );
    Rprintf("unable to open raster source for CreateCopy: %s", (char *)dtarget[0]);
    CSLDestroy(papszOptions); 
    return CharacterVector::create("");
  } 
  CSLDestroy(papszOptions); 
  
  GDALClose( (GDALDatasetH) poDstDS );
  
  
  
  return dtarget;
}
inline List gdal_read_block(CharacterVector dsn, IntegerVector offset,
                            IntegerVector dimension, IntegerVector band,
                            CharacterVector band_output_type, LogicalVector unscale, LogicalVector nara) {
  IntegerVector window(6);
  window[0] = offset[0];
  window[1] = offset[1];
  window[2] = dimension[0];
  window[3] = dimension[1];
  window[4] = dimension[0];
  window[5] = dimension[1];
  return   gdalraster::gdal_raster_io(dsn, window, band, "nearestneighbour", band_output_type, unscale, nara);
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
