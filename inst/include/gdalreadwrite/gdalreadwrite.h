#ifndef GDALREADWRITE_H
#define GDALREADWRITE_H

#include <cpp11.hpp>
#include "common/common_vapour.h"

#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()
#include "gdallibrary/gdallibrary.h"
#include "gdal_utils.h"  // for GDALWarpAppOptions

namespace gdalreadwrite{

using namespace cpp11;
namespace writable = cpp11::writable;

// Helper to convert strings to char** for GDAL open options
inline std::vector<const char*> charv_to_charptr(strings cv) {
  std::vector<const char*> out(cv.size() + 1);
  for (int i = 0; i < cv.size(); i++) {
    out[i] = CHAR(STRING_ELT(cv, i));
  }
  out[cv.size()] = nullptr;
  return out;
}



inline GDALDataType init_datatype(strings datatype) {
  std::string dt = std::string(datatype[0]);
  if (dt == "Byte")    return GDT_Byte;
  if (dt == "Int16")   return GDT_Int16;
  if (dt == "UInt16")  return GDT_UInt16;
  if (dt == "UInt32")  return GDT_UInt32;
  if (dt == "Int32")   return GDT_Int32;
  if (dt == "Float32") return GDT_Float32;
  if (dt == "Float64") return GDT_Float64;
  if (dt == "CInt16")  return GDT_CInt16;
  if (dt == "CInt32")  return GDT_CInt32;
  if (dt == "CFloat32") return GDT_CFloat32;
  if (dt == "CFloat64") return GDT_CFloat64;
#if (GDAL_VERSION_MAJOR >= 3 && GDAL_VERSION_MINOR >= 7)
  if (dt == "Int8")    return GDT_Int8;
#endif
#if (GDAL_VERSION_MAJOR >= 3 && GDAL_VERSION_MINOR >= 5)
  if (dt == "UInt64")  return GDT_UInt64;
  if (dt == "Int64")   return GDT_Int64;
#endif
  return GDT_Unknown;
}



inline strings gdal_create(strings filename,
                           strings driver,
                           doubles extent,
                           integers dimension,
                           strings projection,
                           integers n_bands,
                           strings datatype,
                           list options_list_pairs) {
  
  
  GDALDataType gdt_type = init_datatype(datatype);
  
  OGRSpatialReference oSRS;
  oSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
  
  if (oSRS.SetFromUserInput(as_cstr(projection[0])) != OGRERR_NONE)
  {
    cpp11::warning(
      "Failed to process 'projection' definition");
    
  }
  
  char *pszWKT = nullptr;
  OGRErr err;
  
#if GDAL_VERSION_MAJOR >= 3
  const char *optionsWKT[3] = { "MULTILINE=YES", "FORMAT=WKT2", NULL };
  err = oSRS.exportToWkt(&pszWKT, optionsWKT);
#else
  err = oSRS.exportToWkt(&pszWKT);
#endif
  
  if (err != OGRERR_NONE) {
    cpp11::stop("failed to export CRS to WKT");
  }
  GDALDriverH hDriver;
  hDriver = GDALGetDriverByName(as_cstr(driver[0]));
  if( hDriver == nullptr ) {
    cpp11::stop("failed to get nominated 'driver'");
  }
  
  
  
  
  char **papszOptions = nullptr;
  if (options_list_pairs.size() > 0) {
    for (int i = 0; i < options_list_pairs.size(); i++) {
      strings options2(options_list_pairs[i]);
      if (options2.size() == 2) {
        papszOptions = CSLSetNameValue(papszOptions,
                                       as_cstr(options2[0]),
                                       as_cstr(options2[1]));
      }
    }
  }
  
  
  
  GDALDatasetH hDS = nullptr;
  hDS = GDALCreate(hDriver, as_cstr(filename[0]),
                   dimension[0], dimension[1], n_bands[0], gdt_type,
                   papszOptions);
  
  
  if (hDS == nullptr) {
    Rprintf( "Failed to create dataset\n");
    if (pszWKT != nullptr)  CPLFree(pszWKT);
    CSLDestroy(papszOptions);
    
    writable::strings naout(1);
    naout[0] = NA_STRING;
    return naout;
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





inline strings gdal_create_copy(strings dsource, strings dtarget, strings driver) {
  
  GDALDriver *poDriver;
  poDriver = GetGDALDriverManager()->GetDriverByName(as_cstr(driver[0]));
  
  GDALDataset *poSrcDS = (GDALDataset *) GDALOpen( as_cstr(dsource[0]), GA_ReadOnly );
  if (poSrcDS == NULL ) cpp11::stop("unable to open raster source for reading: %s", as_cstr(dsource[0]));
  
  //
  GDALDataset *poDstDS;
  char **papszOptions = nullptr;
  papszOptions = CSLSetNameValue( papszOptions, "SPARSE_OK", "YES" );
  poDstDS = poDriver->CreateCopy( as_cstr(dtarget[0]), poSrcDS, FALSE,
                                  papszOptions, NULL, NULL );
  
  /* Once we're done, close properly the dataset */
  if( poDstDS == NULL ) {
    GDALClose( (GDALDatasetH) poSrcDS );
    Rprintf("unable to open raster source for CreateCopy: %s", as_cstr(dtarget[0]));
    CSLDestroy(papszOptions);
    writable::strings empty(1);
    empty[0] = "";
    return empty;
  }
  CSLDestroy(papszOptions);
  
  GDALClose( (GDALDatasetH) poDstDS );
  
  
  
  return dtarget;
}
inline list gdal_read_block(strings dsn, integers offset,
                            integers dimension, integers band,
                            strings band_output_type, logicals unscale, logicals nara) {
  writable::integers window(6);
  window[0] = offset[0];
  window[1] = offset[1];
  window[2] = dimension[0];
  window[3] = dimension[1];
  window[4] = dimension[0];
  window[5] = dimension[1];
  writable::strings rs(1);
  rs[0] = "nearestneighbour";
  return gdalraster::gdal_raster_io(dsn, window, band, rs, band_output_type, unscale, nara);
}

inline logicals gdal_write_block(strings dsn, doubles data,
                                 integers offset, integers dimension, integers band,
                                 strings open_options) {
  GDALDataset  *poDataset;
  
  // Use GDALOpenEx with open_options if provided
  if (open_options.size() > 0 && std::string(open_options[0]) != "") {
    std::vector<const char*> oo = charv_to_charptr(open_options);
    poDataset = (GDALDataset *) GDALOpenEx(as_cstr(dsn[0]), GDAL_OF_RASTER | GDAL_OF_UPDATE,
                 NULL, oo.data(), NULL);
  } else {
    poDataset = (GDALDataset *) GDALOpen(as_cstr(dsn[0]), GA_Update);
  }
  if( poDataset == NULL )
  {
    cpp11::stop("cannot open");
  }
  if (band[0] < 1) { GDALClose(poDataset);  cpp11::stop("requested band %i should be 1 or greater", band[0]);  }
  int nbands = poDataset->GetRasterCount();
  if (band[0] > nbands) { GDALClose(poDataset);   cpp11::stop("requested band %i should be equal to or less than number of bands: %i", band[0], nbands); }
  
  GDALRasterBand  *poBand;
  poBand = poDataset->GetRasterBand( band[0] );
  if (poBand == NULL) {
    Rprintf("cannot access band %i", band[0]);
    GDALClose(poDataset);
    cpp11::stop("");
  }
  
  double *padScanline;
  padScanline = (double *) CPLMalloc(sizeof(double) * static_cast<size_t>(dimension[0] * dimension[1]));
  for (int i = 0; i < data.size(); i++) {
    padScanline[i] = data[i];
  }
  CPLErr err;
  err = poBand->RasterIO( GF_Write, offset[0], offset[1], dimension[0], dimension[1],
                          padScanline, dimension[0], dimension[1], GDT_Float64,
                          0, 0);
  GDALClose(poDataset);
  CPLFree(padScanline);
  writable::logicals out(1);
  out[0] = (Rboolean)(err == CE_None);
  return out;
}

}// namespace gdalreadwrite
#endif
