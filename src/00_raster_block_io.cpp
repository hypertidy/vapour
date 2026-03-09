#include <cpp11.hpp>
#include "gdalreadwrite/gdalreadwrite.h"
#include "gdalraster/gdalraster.h"

using namespace cpp11;
namespace writable = cpp11::writable;

[[cpp11::register]]
list vapour_read_raster_block_cpp(strings dsource,
                                  integers offset, integers dimension, integers band,
                                  strings band_output_type,
                                  logicals unscale, logicals nara) {
  return gdalreadwrite::gdal_read_block(dsource, offset, dimension, band, band_output_type, unscale, nara);
}

[[cpp11::register]]
logicals vapour_write_raster_block_cpp(strings dsource, doubles data,
                                       integers offset, integers dimension, integers band,
                                       strings open_options) {
  return gdalreadwrite::gdal_write_block(dsource, data, offset, dimension, band, open_options);
}

[[cpp11::register]]
strings vapour_create_copy_cpp(strings dsource, strings dtarget, strings driver) {
  return gdalreadwrite::gdal_create_copy(dsource, dtarget, driver);
}

[[cpp11::register]]
strings vapour_create_cpp(strings filename, strings driver,
                          doubles extent, integers dimension,
                          strings projection,
                          integers n_bands,
                          strings datatype,
                          list options_list_pairs) {
  return gdalreadwrite::gdal_create(filename, driver, extent, dimension, projection, n_bands, datatype, options_list_pairs);
}

[[cpp11::register]]
doubles vapour_read_raster_value_cpp(strings dsource,
                                     integers col, integers row, integers band,
                                     strings band_output_type) {
  writable::integers sds0 = {0};
  GDALDatasetH ds = gdalraster::gdalH_open_dsn(std::string(dsource[0]).c_str(), sds0);

  writable::doubles vals(col.size());
  writable::strings resample(1);
  resample[0] = "near";

  if (band[0] < 1) cpp11::stop("invalid band number");
  if (band[0] > ((GDALDataset *)ds)->GetRasterCount()) cpp11::stop("invalid band number");
  GDALRasterBand * poBand = ((GDALDataset*) ds)->GetRasterBand(band[0]);
  GDALRasterIOExtraArg psExtraArg;
  psExtraArg = gdalraster::init_resample_alg(resample);
  CPLErr err;

  for (int i = 0; i < col.size(); i++) {
    double v;
    err = poBand->RasterIO(GF_Read, col[i], row[i], 1, 1,
                           &v, 1, 1, GDT_Float64,
                           0, 0, &psExtraArg);
    if (err != OGRERR_NONE) {
      cpp11::stop("failed to read band values");
    }
    vals[i] = v;
  }
  GDALClose(ds);
  return vals;
}

[[cpp11::register]]
list blocks_cpp1(strings dsource, integers iblock, logicals read) {
  writable::integers sds0 = {0};
  GDALDatasetH ds = gdalraster::gdalH_open_dsn(std::string(dsource[0]).c_str(), sds0);
  GDALRasterBand * poBand = ((GDALDataset*) ds)->GetRasterBand(1);

  if (! (poBand->GetRasterDataType() == GDT_Float32 )) cpp11::stop("");

  int nXBlockSize, nYBlockSize;
  poBand->GetBlockSize( &nXBlockSize, &nYBlockSize );

  writable::list out(1);
  int cnt = 0;
  float *faBlockData = (float *) CPLMalloc(nXBlockSize * nYBlockSize);
  int iXBlock = 0;
  int iYBlock = 0;
  int nXValid, nYValid;
  poBand->GetActualBlockSize(iXBlock, iYBlock, &nXValid, &nYValid);
  writable::doubles float_data(nXValid * nYValid);
  CPLErr err = poBand->ReadBlock( iXBlock, iYBlock, faBlockData );
  if (!(err == OGRERR_NONE)) {
    GDALClose(ds);
    CPLFree(faBlockData);
    cpp11::stop("could not read block");
  }
  for( int iY = 0; iY < nYValid; iY++ )
  {
    for( int iX = 0; iX < nXValid; iX++ )
    {
      float_data[cnt] = (double)faBlockData[iX + iY * nXBlockSize];
      cnt++;
  }
  }
  writable::integers abs = {nXValid, nYValid};
  float_data.attr("actual_block_size") = abs;
  out[0] = float_data;
  GDALClose(ds);
  CPLFree(faBlockData);
  return out;
}

[[cpp11::register]]
list blocks_cpp(strings dsource, integers iblock, logicals read) {
  writable::integers sds0 = {0};
  GDALDatasetH ds = gdalraster::gdalH_open_dsn(std::string(dsource[0]).c_str(), sds0);
  GDALRasterBand * poBand = ((GDALDataset*) ds)->GetRasterBand(1);

  int nXBlockSize, nYBlockSize;
  poBand->GetBlockSize( &nXBlockSize, &nYBlockSize );

  int nXBlocks = (poBand->GetXSize() + nXBlockSize - 1) / nXBlockSize;
  int nYBlocks = (poBand->GetYSize() + nYBlockSize - 1) / nYBlockSize;

  writable::list out(nXBlocks * nYBlocks);
  int cnt = 0;

  for( int iYBlock = 0; iYBlock < nYBlocks; iYBlock++ )
  {
    for( int iXBlock = 0; iXBlock < nXBlocks; iXBlock++ )
    {
      int nXValid, nYValid;
      poBand->GetActualBlockSize(iXBlock, iYBlock, &nXValid, &nYValid);
      if ((bool)read[0]) {
        writable::raws raw_bytes(nXValid * nYValid);
        Rprintf("%i\n", iYBlock);
        writable::integers abs = {nXValid, nYValid};
        raw_bytes.attr("actual_block_size") = abs;
        out[cnt] = raw_bytes;
      } else {
        writable::logicals dummy_vec((R_xlen_t)0);
        writable::integers abs = {nXValid, nYValid};
        dummy_vec.attr("actual_block_size") = abs;
        out[cnt] = dummy_vec;
      }
      cnt++;
    }
  }
  GDALClose(ds);
  return out;
}
