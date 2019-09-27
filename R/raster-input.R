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
#' @param band index of which band to read (1-based)
#' @param window src_offset, src_dim, out_dim
#' @param resample resampling method used (see details)
#' @param ... reserved
#' @param native apply the full native window for read, `FALSE` by default
#' @param sds index of subdataset to read (usually 1)
#' @param set_na specify whether NA values should be set for the NODATA
#' @export
#' @return list of numeric vectors (only one for 'band')
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
vapour_read_raster <- function(x, band = 1, window, resample = "nearestneighbour", ..., sds = NULL, native = FALSE, set_na = TRUE) {
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
  vals <- raster_io_cpp(filename = datasourcename, window  = window, band = band, resample = resample[1L])
  if (set_na) vals[[1]][vals[[1]] == ri$nodata_value] <- NA   ## hardcode to 1 for now
  names(vals) <- sprintf("Band%i",band)
  vals
}


#' IN-DEV,NOT EXPORTED Raster input (warp)
#'
#' Read a window of data from a GDAL raster source through a warp specification.
#' Only a single band may be read. The warp specification is provided by 'geotransform',
#' 'dimension', and 'wkt' properties of the transformed output.
#'
#' This function is not memory safe, the source is left on disk but the output
#' raster is all computed in memory so please be careful with very large values
#' for 'dimension'. `1000 * 1000 * 8` for 1000 columns, 1000 rows and floating point
#' double type will be 8Mb.
#'
#' There's no control over the output type (always double floating point) and no
#' control for the resampling algorithm (uses 'nearest neighbour').
#'
#' 'wkt' refers to the full Well-Known-Text specification of a coordinate reference
#' system. See [vapour_srs_wkt()] for conversion from PROJ.4 string to WKT.
#'
#' 'geotransform' is the six-figure affine transform 'xmin, xres, yskew, ymax, xskew, yres' see [vapour_raster_info()] for more detail.
#'
#' 'dimension' is the pixel dimensions of the output, x (ncol) then y (nrow).
#'
#' Values for missing data are not yet handled, just returned
#' as-is. Note that there may be regions of "zero data" in a warped output,
#' separate from propagated missing "NODATA" values in the source.
#'
#' Argument 'source_wkt' may be used to assign the proejction of the source, this
#' is sometimes required especially for NetCDF files (no checking is done). If the
#' source has no projection and this is not set GDAL will error.
#'
#' @param x data source string (file name or URL or database connection string)
#' @param band index of band to read (1-based)
#' @param geotransform the affine geotransform of the warped raster
#' @param dimension dimensions in pixels of the warped raster (x, y)
#' @param wkt projection of warped raster in Well-Known-Text
#' @param set_na NOT IMPLEMENTED logical, should 'NODATA' values be set to `NA`
#' @param source_wkt optional, override the projection of the source with WKT
#' @noRd
#' @return list of vectors (only 1 for 'band') of numeric values, in raster order
#' @examples
#' #gt <- c(-637239.4, 5030.0, 0.0, 261208.7, 0.0, -7760.0)
#'
#' #f <- system.file("extdata", "sst.tif", package = "vapour")
#' #vals <- vapour_warp_raster(f, geotransform = gt,
#' #                             dimension = c(186, 298),
#' #                             wkt = tas_wkt)
#' ## wkt, dimension, geotransform above created via
#' ##p <- raster::projectRaster(f,
#' ## crs = "+proj=laea +lon_0=147 +lat_0=-42 +datum=WGS84")
#' ##writeRaster(p, "a.tif")
#' ## vapour::vapour_raster_info("a.tif")
vapour_warp_raster <- function(x, band = 1L,
                               geotransform = NULL,
                               dimension = NULL,
                               wkt = "",
                               set_na = TRUE,
                               source_wkt = NULL) {
  stopifnot(is.numeric(band))
  stopifnot(is.numeric(geotransform))
  stopifnot(length(geotransform) == 6L)
  stopifnot(is.numeric(dimension))
  stopifnot(length(dimension) == 2L)
  stopifnot(all(dimension > 0))
  if (!is.null(source_wkt)) stopifnot(is.character(source_wkt))
  stopifnot(nchar(source_wkt) > 10)
  stopifnot(nchar(wkt)> 0)
  ## TODO: validate geotransform, source_wkt, dimension
  if (length(band) != 1) {
    warning("more than one band requested, using first only")
  }
  if (is.null(source_wkt)) source_wkt <-  ""
  if (band < 1) stop("band must be 1 or higher")

  vals <- warp_memory_cpp(x, source_WKT = source_wkt,
                                   target_WKT = wkt,
                                   target_geotransform = geotransform,
                                   target_dim = dimension, band = band)
  #ri <- vapour_raster_info(x)
  #if (set_na) {
   # vals[vals == ri$nodata_value] <- NA
   # x[abs(x - ri$nodata_value) < sqrt(.Machine$double.eps)] <- NA
  #}
  names(vals) <- sprintf("Band%i",band)
  vals
}




