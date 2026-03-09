#include <cpp11.hpp>
#include "gdallibrary/gdallibrary.h"
#include "gdalwarpgeneral/gdalwarpgeneral.h"

using namespace cpp11;

[[cpp11::register]]
list warp_general_cpp(strings dsn,
                      strings target_crs,
                      doubles target_extent,
                      integers target_dim,
                      doubles target_res,
                      integers bands,
                      strings resample,
                      logicals silent,
                      strings band_output_type,
                      strings options,
                      strings dsn_outname,
                      logicals include_meta,
                      logicals nara) {
  return gdalwarpgeneral::gdal_warp_general(dsn,
                                          target_crs,
                                          target_extent,
                                          target_dim,
                                          target_res,
                                          bands,
                                          resample,
                                          silent,
                                          band_output_type,
                                          options,
                                          dsn_outname,
                                          include_meta,
                                          nara);
}

[[cpp11::register]]
list warp_suggest_cpp(strings dsn, strings target_crs) {
  return gdalwarpgeneral::gdal_suggest_warp(dsn, target_crs);
}
