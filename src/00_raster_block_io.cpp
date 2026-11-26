#include <cpp11.hpp>
#include "gdalreadwrite/gdalreadwrite.h"
#include "gdalraster/gdalraster.h"

using namespace cpp11;
namespace writable = cpp11::writable;
using gdalcommon::gdal_call;

[[cpp11::register]]
list vapour_read_raster_block_cpp(strings dsource,
                                  integers offset, integers dimension, integers band,
                                  strings band_output_type,
                                  logicals unscale, logicals nara,
                                  strings open_options) {
  return gdal_call([&]() {
    return gdalreadwrite::gdal_read_block(dsource, offset, dimension, band, band_output_type, unscale, nara, open_options);
  });
}

[[cpp11::register]]
logicals vapour_write_raster_block_cpp(strings dsource, doubles data,
                                       integers offset, integers dimension, integers band,
                                       strings open_options) {
  return gdal_call([&]() {
    return gdalreadwrite::gdal_write_block(dsource, data, offset, dimension, band, open_options);
  });
}

[[cpp11::register]]
strings vapour_create_copy_cpp(strings dsource, strings dtarget, strings driver) {
  return gdal_call([&]() { return gdalreadwrite::gdal_create_copy(dsource, dtarget, driver); });
}

[[cpp11::register]]
strings vapour_create_cpp(strings filename, strings driver,
                          doubles extent, integers dimension,
                          strings projection,
                          integers n_bands,
                          strings datatype,
                          list options_list_pairs) {
  return gdal_call([&]() {
    return gdalreadwrite::gdal_create(filename, driver, extent, dimension, projection, n_bands, datatype, options_list_pairs);
  });
}

[[cpp11::register]]
doubles vapour_read_raster_value_cpp(strings dsource,
                                     integers col, integers row, integers band,
                                     strings band_output_type) {
  return gdal_call([&]() {
    writable::integers sds0 = {0};
    writable::strings no_options((R_xlen_t)0);
    gdalcommon::DatasetPtr ds = gdalraster::open_raster_or_stop(as_cstr(dsource[0]), sds0, no_options);

    if (col.size() != row.size()) cpp11::stop("'col' and 'row' must be the same length");
    if (band.size() < 1 || band[0] == NA_INTEGER || band[0] < 1) cpp11::stop("invalid band number");
    if (band[0] > ds->GetRasterCount()) cpp11::stop("invalid band number");
    GDALRasterBand * poBand = ds->GetRasterBand(band[0]);
    if (poBand == nullptr) gdalcommon::stop_gdal("cannot access band");

    writable::doubles vals(col.size());
    writable::strings resample(1);
    resample[0] = "near";
    GDALRasterIOExtraArg psExtraArg;
    psExtraArg = gdalraster::init_resample_alg(resample);

    for (int i = 0; i < col.size(); i++) {
      double v;
      CPLErr err = poBand->RasterIO(GF_Read, col[i], row[i], 1, 1,
                             &v, 1, 1, GDT_Float64,
                             0, 0, &psExtraArg);
      if (err != CE_None) {
        gdalcommon::stop_gdal("failed to read band values");
      }
      vals[i] = v;
    }
    return vals;
  });
}

[[cpp11::register]]
list blocks_cpp1(strings dsource, integers iblock, logicals read) {
  return gdal_call([&]() {
    writable::integers sds0 = {0};
    writable::strings no_options((R_xlen_t)0);
    gdalcommon::DatasetPtr ds = gdalraster::open_raster_or_stop(as_cstr(dsource[0]), sds0, no_options);
    if (ds->GetRasterCount() < 1) cpp11::stop("no bands in dataset");
    GDALRasterBand * poBand = ds->GetRasterBand(1);

    if (! (poBand->GetRasterDataType() == GDT_Float32 )) cpp11::stop("only Float32 bands are supported");

    int nXBlockSize, nYBlockSize;
    poBand->GetBlockSize( &nXBlockSize, &nYBlockSize );

    writable::list out(1);
    R_xlen_t cnt = 0;
    // one whole block of Float32 values
    std::vector<float> faBlockData(static_cast<size_t>(nXBlockSize) * static_cast<size_t>(nYBlockSize));
    int iXBlock = 0;
    int iYBlock = 0;
    int nXValid, nYValid;
    poBand->GetActualBlockSize(iXBlock, iYBlock, &nXValid, &nYValid);
    writable::doubles float_data(static_cast<R_xlen_t>(nXValid) * static_cast<R_xlen_t>(nYValid));
    CPLErr err = poBand->ReadBlock( iXBlock, iYBlock, faBlockData.data() );
    if (err != CE_None) {
      gdalcommon::stop_gdal("could not read block");
    }
    for( int iY = 0; iY < nYValid; iY++ )
    {
      for( int iX = 0; iX < nXValid; iX++ )
      {
        float_data[cnt] = (double)faBlockData[static_cast<size_t>(iX) + static_cast<size_t>(iY) * static_cast<size_t>(nXBlockSize)];
        cnt++;
      }
    }
    writable::integers abs = {nXValid, nYValid};
    float_data.attr("actual_block_size") = abs;
    out[0] = float_data;
    return out;
  });
}

[[cpp11::register]]
list blocks_cpp(strings dsource, integers iblock, logicals read) {
  return gdal_call([&]() {
    writable::integers sds0 = {0};
    writable::strings no_options((R_xlen_t)0);
    gdalcommon::DatasetPtr ds = gdalraster::open_raster_or_stop(as_cstr(dsource[0]), sds0, no_options);
    if (ds->GetRasterCount() < 1) cpp11::stop("no bands in dataset");
    GDALRasterBand * poBand = ds->GetRasterBand(1);

    int nXBlockSize, nYBlockSize;
    poBand->GetBlockSize( &nXBlockSize, &nYBlockSize );

    int nXBlocks = (poBand->GetXSize() + nXBlockSize - 1) / nXBlockSize;
    int nYBlocks = (poBand->GetYSize() + nYBlockSize - 1) / nYBlockSize;

    writable::list out(static_cast<R_xlen_t>(nXBlocks) * static_cast<R_xlen_t>(nYBlocks));
    R_xlen_t cnt = 0;

    for( int iYBlock = 0; iYBlock < nYBlocks; iYBlock++ )
    {
      for( int iXBlock = 0; iXBlock < nXBlocks; iXBlock++ )
      {
        int nXValid, nYValid;
        poBand->GetActualBlockSize(iXBlock, iYBlock, &nXValid, &nYValid);
        if ((bool)read[0]) {
          writable::raws raw_bytes(static_cast<R_xlen_t>(nXValid) * static_cast<R_xlen_t>(nYValid));
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
    return out;
  });
}
