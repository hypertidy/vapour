#ifndef GDALREADWRITE_H
#define GDALREADWRITE_H

#include <cpp11.hpp>
#include "common/common_vapour.h"
#include "common/gdal_handles.h"

#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()
#include "gdallibrary/gdallibrary.h"
#include "gdal_utils.h"  // for GDALWarpAppOptions

namespace gdalreadwrite{

using namespace cpp11;
namespace writable = cpp11::writable;

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
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
  if (dt == "Int8")    return GDT_Int8;
#endif
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,5,0)
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

  if (extent.size() != 4) cpp11::stop("'extent' must have 4 values: xmin, xmax, ymin, ymax");
  if (dimension.size() != 2) cpp11::stop("'dimension' must have 2 values: ncol, nrow");
  if (dimension[0] == NA_INTEGER || dimension[1] == NA_INTEGER || dimension[0] < 1 || dimension[1] < 1) {
    cpp11::stop("'dimension' values must be 1 or greater");
  }

  GDALDataType gdt_type = init_datatype(datatype);
  if (gdt_type == GDT_Unknown) cpp11::stop("unknown 'datatype': '%s'", as_cstr(datatype[0]));

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
  gdalcommon::CPLStringPtr wkt_owner(pszWKT);

  if (err != OGRERR_NONE) {
    gdalcommon::stop_gdal("failed to export CRS to WKT");
  }
  GDALDriverH hDriver;
  hDriver = GDALGetDriverByName(as_cstr(driver[0]));
  if( hDriver == nullptr ) {
    cpp11::stop("failed to get nominated 'driver'");
  }

  CPLStringList aosOptions;
  for (int i = 0; i < options_list_pairs.size(); i++) {
    strings options2(options_list_pairs[i]);
    if (options2.size() == 2) {
      aosOptions.SetNameValue(as_cstr(options2[0]), as_cstr(options2[1]));
    }
  }

  gdalcommon::DatasetPtr hDS((GDALDataset *) GDALCreate(hDriver, as_cstr(filename[0]),
                   dimension[0], dimension[1], n_bands[0], gdt_type,
                   aosOptions.List()));

  if (!hDS) {
    // not an error, as before: the result is NA and GDAL's reason is a warning
    writable::strings naout(1);
    naout[0] = NA_STRING;
    return naout;
  }

  double adfGeoTransform[6] = { extent[0], (extent[1] - extent[0])/ dimension[0], 0,
                                extent[3], 0, (extent[2] - extent[3])/ dimension[1]};
  hDS->SetGeoTransform(adfGeoTransform);
  if (pszWKT != nullptr) hDS->SetProjection(pszWKT);

  return filename;
}





inline strings gdal_create_copy(strings dsource, strings dtarget, strings driver) {

  GDALDriver *poDriver;
  poDriver = GetGDALDriverManager()->GetDriverByName(as_cstr(driver[0]));
  if (poDriver == nullptr) cpp11::stop("failed to get nominated 'driver'");

  writable::integers sds0 = {0};
  writable::strings no_options((R_xlen_t)0);
  gdalcommon::DatasetPtr poSrcDS = gdalraster::open_raster(as_cstr(dsource[0]), sds0, no_options);
  if (!poSrcDS) {
    gdalcommon::stop_gdal(std::string("unable to open raster source for reading: '") + as_cstr(dsource[0]) + "'");
  }

  CPLStringList aosOptions;
  aosOptions.SetNameValue("SPARSE_OK", "YES");
  gdalcommon::DatasetPtr poDstDS(poDriver->CreateCopy( as_cstr(dtarget[0]), poSrcDS.get(), FALSE,
                                  aosOptions.List(), NULL, NULL ));

  if (!poDstDS) {
    // not an error, as before: the result is "" and GDAL's reason is a warning
    writable::strings empty(1);
    empty[0] = "";
    return empty;
  }
  return dtarget;
}
inline list gdal_read_block(strings dsn, integers offset,
                            integers dimension, integers band,
                            strings band_output_type, logicals unscale, logicals nara,
                            strings open_options) {
  if (offset.size() != 2 || dimension.size() != 2) cpp11::stop("'offset' and 'dimension' must each have 2 values");
  writable::integers window(6);
  window[0] = offset[0];
  window[1] = offset[1];
  window[2] = dimension[0];
  window[3] = dimension[1];
  window[4] = dimension[0];
  window[5] = dimension[1];
  writable::strings rs(1);
  rs[0] = "nearestneighbour";
  return gdalraster::gdal_raster_io(dsn, window, band, rs, band_output_type, unscale, nara, open_options);
}

inline logicals gdal_write_block(strings dsn, doubles data,
                                 integers offset, integers dimension, integers band,
                                 strings open_options) {
  if (offset.size() != 2 || dimension.size() != 2) cpp11::stop("'offset' and 'dimension' must each have 2 values");
  if (dimension[0] == NA_INTEGER || dimension[1] == NA_INTEGER || dimension[0] < 1 || dimension[1] < 1) {
    cpp11::stop("'dimension' values must be 1 or greater");
  }
  // GDAL reads exactly this many values from 'data'
  R_xlen_t ncell = static_cast<R_xlen_t>(dimension[0]) * static_cast<R_xlen_t>(dimension[1]);
  if (data.size() != ncell) {
    cpp11::stop("'data' has %.0f values but 'dimension' needs %.0f", (double)data.size(), (double)ncell);
  }

  writable::integers sds0 = {0};
  gdalcommon::DatasetPtr poDataset = gdalraster::open_raster(as_cstr(dsn[0]), sds0, open_options,
                                                             GDAL_OF_RASTER | GDAL_OF_UPDATE);
  if (!poDataset) {
    gdalcommon::stop_gdal(std::string("cannot open '") + as_cstr(dsn[0]) + "' for update");
  }
  if (band.size() < 1 || band[0] == NA_INTEGER || band[0] < 1) cpp11::stop("requested band %i should be 1 or greater", band.size() < 1 ? 0 : (int)band[0]);
  int nbands = poDataset->GetRasterCount();
  if (band[0] > nbands) cpp11::stop("requested band %i should be equal to or less than number of bands: %i", (int)band[0], nbands);

  GDALRasterBand  *poBand;
  poBand = poDataset->GetRasterBand( band[0] );
  if (poBand == NULL) {
    gdalcommon::stop_gdal("cannot access band " + std::to_string((int)band[0]));
  }

  std::vector<double> padScanline(static_cast<size_t>(ncell));
  for (R_xlen_t i = 0; i < ncell; i++) padScanline[static_cast<size_t>(i)] = data[i];
  CPLErr err;
  err = poBand->RasterIO( GF_Write, offset[0], offset[1], dimension[0], dimension[1],
                          padScanline.data(), dimension[0], dimension[1], GDT_Float64,
                          0, 0);
  writable::logicals out(1);
  out[0] = (Rboolean)(err == CE_None);
  return out;
}

}// namespace gdalreadwrite
#endif
