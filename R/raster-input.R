#' Raster IO (read)
#'
#' Read a window of data from a GDAL raster source. The first argument is the source
#' name and the second is a 6-element `window` of offset, source dimension, and output dimension.
#'
#' This is analgous to the `rgdal` function `readGDAL` with its arguments `offset`,  `region.dim`
#' and `output.dim`.
#'
#' Resampling options will depend on GDAL version,  but currently 'NearestNeighbour' (default),
#' 'Average', 'Bilinear', 'Cubic', 'CubicSpline',  'Gauss', 'Lanczos', 'Mode' are potentially
#' available. Detailed use of this is barely tried or tested with vapour, but is
#' a standard facility used in GDAL. Easiest way to compare results is with gdal_translate.
#' @param x data source
#' @param band index of which band to read
#' @param window src_offset, src_dim, out_dim
#' @param resample resampling method used (see details)
#' @export
#' @examples
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' ## a 5*5 window from a 10*10 region
#' raster_io(f, window = c(0, 0, 10, 10, 5, 5))
#' raster_io(f, window = c(0, 0, 10, 10, 5, 5), resample = "Lanczos")
#' ## find the information first
#' #ri <- raster_info(f)
#' #str(matrix(raster_io(f, c(0, 0, ri$dimXY, ri$dimXY)), ri$dimXY[1]))
#' ## the method can be used to up-sample as well
#' #str(matrix(raster_io(f, window = c(0, 0, 10, 10, 15, 25)), 15))
#' ## a future version will provide access to different methods
raster_io <- function(x, band = 1, window, resample = "NearestNeighbour") {
  raster_io_cpp(filename = x, window  = window, band = band, resample = resample[1L])
}
