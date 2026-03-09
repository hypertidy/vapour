#include <cpp11.hpp>
#include "gdalapplib/gdalapplib.h"

using namespace cpp11;

[[cpp11::register]]
list raster_warp_file_cpp(strings source_filename,
                          strings target_crs,
                          doubles target_extent,
                          integers target_dim,
                          strings target_filename,
                          integers bands,
                          strings resample,
                          logicals silent,
                          strings band_output_type,
                          strings warp_options,
                          strings transformation_options) {
  return gdalapplib::gdalwarp_applib(source_filename,
                                     target_crs,
                                     target_extent,
                                     target_dim,
                                     target_filename,
                                     bands,
                                     resample,
                                     silent,
                                     band_output_type,
                                     warp_options,
                                     transformation_options);
}
