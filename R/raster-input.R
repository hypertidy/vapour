#' Raster IO (read)
#'
#' Read a window of data from a GDAL raster source. The first argument is the source
#' name and the second is a 6-element `window` of offset, source dimension, and output dimension.
#'
#' The value of `window` may be input as only 4 elements, in which case the source dimension
#' Will be used as the output dimension.
#'
#' This is analogous to the `rgdal` function `readGDAL` with its arguments `offset`,  `region.dim`
#' and `output.dim`.  There's no semantic wrapper for this in vapour, but see
#' `https://github.com/hypertidy/lazyraster` for one approach.
#'
#' Resampling options will depend on GDAL version,  but currently  'NearestNeighbour' (default),
#' 'Average', 'Bilinear', 'Cubic', 'CubicSpline',  'Gauss', 'Lanczos', 'Mode' are potentially
#' available. These are compared internally by converting to lower-case. Detailed use of this is barely tried or tested with vapour, but is
#' a standard facility used in GDAL. Easiest way to compare results is with gdal_translate.
#'
#' There is no write support in vapour.
#'
#' Currently the `window` argument is required. If this argument unspecified and `native = TRUE` then
#' the default window specification will be used, the entire extent at native resolution. If 'window'
#' is specified and `native = TRUE` then the window is used as-is, with a warning (native is ignored).
#' @param x data source
#' @param band index of which band to read
#' @param window src_offset, src_dim, out_dim
#' @param resample resampling method used (see details)
#' @param ... reserved
#' @param native apply the full native window for read, `FALSE` by default
#' @param sds index of subdataset to read (usually 1)
#'
#' @export
#' @examples
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' ## a 5*5 window from a 10*10 region
#' vapour_read_raster(f, window = c(0, 0, 10, 10, 5, 5))
#' vapour_read_raster(f, window = c(0, 0, 10, 10, 5, 5), resample = "Lanczos")
#' ## find the information first
#' ri <- vapour_raster_info(f)
#' str(matrix(vapour_read_raster(f, window = c(0, 0, ri$dimXY, ri$dimXY)), ri$dimXY[1]))
#' ## the method can be used to up-sample as well
#' str(matrix(vapour_read_raster(f, window = c(0, 0, 10, 10, 15, 25)), 15))
#'
vapour_read_raster <- function(x, band = 1, window, resample = "nearestneighbour", ..., sds = NULL, native = FALSE) {
  datasourcename <- sds_boilerplate_checks(x, sds = sds)
  resample <- tolower(resample)  ## ensure check internally is lower case
  if (!resample %in% c("nearestneighbour", "average", "bilinear", "cubic", "cubicspline",
                       "gauss", "lanczos", "mode")) {
    warning(sprintf("resample mode '%s' is unknown", resample))
  }
  ri <- vapour_raster_info(x, sds = sds)
  if (native && !missing(window)) warning("'window' is specified, so 'native = TRUE' is ignored")
  if (native && missing(window)) window <- c(0, 0, ri$dimXY, ri$dimXY)

  if (!is.numeric(band) || band < 1 || length(band) < 1 || length(band) > 1 || is.na(band)) {
    stop("'band' must be an integer of length 1, and be greater than 0")
  }
  if (band > ri$bands) stop(sprintf("specified 'band = %i', but maximum band number is %i", band, ri$bands))
  ## turn these warning cases into errors here, + tests
  ## rationale is that dev can still call the internal R wrapper function to
  ## get these errors, but not the R user



  stopifnot(length(window) %in% c(4L, 6L))
  ## use src dim as out dim by default
 if (length(window) == 4L) window <- c(window, window[3:4])
  ## these error at the GDAL level
  if (any(window[1:2] < 0)) stop("window cannot index lower than 0")
  if (any(window[1:2] > (ri$dimXY-1))) stop("window offset cannot index higher than grid dimension")
  ## this does not error in GDAL, gives an out of bounds value
  if (any(window[3:4] < 1)) stop("window size cannot be less than 1")

  ## GDAL error
  if (any((window[1:2] + window[3:4]) > ri$dimXY)) stop("window size cannot exceed grid dimension")
  ## GDAL error
  if (any(window[5:6] < 1)) stop("requested output dimension cannot be less than 1")
  ## pull a swifty here with [[  to return numeric or integer
  raster_io_cpp(filename = datasourcename, window  = window, band = band, resample = resample[1L])[[1L]]
}
