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
  vals <- raster_io_gdal_cpp(dsn = datasourcename, window  = window, band = band, resample = resample[1L])
  if (set_na) vals[[1]][vals[[1]] == ri$nodata_value] <- NA   ## hardcode to 1 for now
  names(vals) <- sprintf("Band%i",band)
  vals
}


#' Raster warper (reprojection)
#'
#' Read a window of data from a GDAL raster source through a warp specification.
#' Only a single band may be read. The warp specification is provided by 'extent',
#' 'dimension', and 'wkt' properties of the transformed output.
#'
#' This function is not memory safe, the source is left on disk but the output
#' raster is all computed in memory so please be careful with very large values
#' for 'dimension'. `1000 * 1000 * 8` for 1000 columns, 1000 rows and floating point
#' double type will be 8Mb.
#'
#' There's no control over the output type (always double floating point).
#' #'
#' 'wkt' refers to the full Well-Known-Text specification of a coordinate reference
#' system. See [vapour_srs_wkt()] for conversion from PROJ.4 string to WKT.
#'
#' 'extent' is the four-figure xmin,xmax,ymin,ymax outer corners of corner pixels
#'
#' 'dimension' is the pixel dimensions of the output, x (ncol) then y (nrow).
#'
#' Values for missing data are not yet handled, just returned
#' as-is. Note that there may be regions of "zero data" in a warped output,
#' separate from propagated missing "NODATA" values in the source.
#'
#' Argument 'source_wkt' may be used to assign the proejction of the source, 'source_extent'
#' to assign the extent of the source. Sometimes both are required. Wild combinations of
#' 'source_extent' and/or 'extent' may be used for arbitrary flip orientations, scale and offset. For
#' expert usage only. Old versions allowed transform input for target and source but this is now disabled (maybe we'll write
#'  a new wrapper for that).
#'
#' @param x data source string (file name or URL or database connection string)
#' @param band index of band to read (1-based)
#' @param extent extent of the target warped raster 'c(xmin, xmax, ymin, ymax)'
#' @param source_extent extent of the source raster, used to override/augment incorrect source metadata
#' @param geotransform DEPRECATED use 'extent' the affine geotransform of the warped raster
#' @param dimension dimensions in pixels of the warped raster (x, y)
#' @param wkt projection of warped raster in Well-Known-Text
#' @param set_na NOT IMPLEMENTED logical, should 'NODATA' values be set to `NA`
#' @param source_geotransform DEPRECATED use 'source_extent' (override the native geotransform of the source)
#' @param resample resampling method used (see details in [vapour_read_raster])
#' @param source_wkt optional, override or augment the projection of the source with WKT
#'
#' @export
#' @return list of vectors (only 1 for 'band') of numeric values, in raster order
#' @examples
#' b <- 4e5
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' prj <- "+proj=aeqd +lon_0=147 +lat_0=-42"
#' vals <- vapour_warp_raster(f, extent = c(-b, b, -b, b),
#'                              dimension = c(186, 298),
#'                              wkt = vapour_srs_wkt(prj))
#' image(list(x = seq(-b, b, length.out = 187), y = seq(-b, b, length.out = 298),
#'     z = matrix(unlist(vals), 186)[,298:1]), asp = 1)
vapour_warp_raster <- function(x, band = 1L,
                               extent = NULL,
                               dimension = NULL,
                               wkt = "",
                               set_na = TRUE,
                               source_wkt = NULL,
                               source_extent = 0.0,
                               resample = "near",
                               silent = TRUE, ...,
                               source_geotransform = 0.0, geotransform = NULL) {
  if(!is.numeric(band)) stop("'band' must be numeric (integer), start at 1")
  if(!is.numeric(extent)) {
    if (isS4(extent)) {
      extent <- c(extent@xmin, extent@xmax, extent@ymin, extent@ymax)
    } else if (is.matrix(extent)) {
        extent <- extent[c(1, 3, 2, 4)]
    } else {
    stop("'extent' must be numeric 'c(xmin, xmax, ymin, ymax)'")
    }
  }
  if(!length(extent) == 4L) stop("'extent' must be of length 4")

  if (any(diff(extent)[c(1, 3)] == 0)) stop("'extent' expected to be 'c(xmin, xmax, ymin, ymax)', zero x or y range not permitted")
  if (length(source_extent) > 1 && any(diff(source_extent)[c(1, 3)] == 0)) stop("'extent' expected to be 'c(xmin, xmax, ymin, ymax)', zero x or y range not permitted")

  if (!all(diff(extent)[c(1, 3)] > 0)) message("'extent' expected to be 'c(xmin, xmax, ymin, ymax)', negative values detected (ok for expert use)")
  if (length(source_extent) > 1 && !all(diff(source_extent)[c(1, 3)] > 0)) message("'extent' expected to be 'c(xmin, xmax, ymin, ymax)', negative values detected (ok for expert use)")



  if(!is.numeric(dimension)) stop("'dimension' must be numeric")
  if(!length(dimension) == 2L) stop("'dimension must be of length 2'")
  if(!all(dimension > 0)) stop("'dimension' values must be greater than 0")
  if(!all(is.finite(dimension))) stop("'dimension' values must be finite and non-missing")
  if(!(length(source_geotransform) == 1 && source_geotransform == 0.0)) message("'source geotransform' is deprecated and now ignored, please use 'extent'")
  if (length(source_extent) > 1) {
    if (!is.numeric(source_extent)) {
      stop("'source_extent' must be numeric, of length 4 c(xmin, xmax, ymin, ymax)")
    }
    if (!all(is.finite(source_extent))) stop("'source_extent' values must be finite and non missing")
  }
  if (!is.null(geotransform)) message("'geotransform' is deprecated and now ignored, used 'extent'")
  if(!is.null(source_wkt)) {
    if (!is.character(source_wkt)) stop("source_wkt must be character")
    if(!nchar(source_wkt) > 10) message("short 'source_wkt' seems invalid")
  }
  if(!nchar(wkt) > 0) message("short 'wkt' seems invalid")
  ## TODO: validate geotransform, source_wkt, dimension

  if (is.null(source_wkt)) source_wkt <-  ""
  if (any(band < 1)) stop("band must be 1 or higher")

  resample <- tolower(resample[1L])
  if (resample == "gauss") {
    warning("Gauss resampling not available for warper, using NearestNeighbour")
    resample <- "near"
  }
  rso <- c("near", "bilinear", "cubic", "cubicspline", "lanczos", "average",
           "mode", "max", "min", "med", "q1", "q3") ## "sum", "rms")

  if (!resample %in% rso) {
    warning(sprintf("%s resampling not available for warper, using near", resample))
    resample <- "near"

  }


  vals <- warp_in_memory_gdal_cpp(x, source_WKT = source_wkt,
                                   target_WKT = wkt,
                                   target_extent = extent,
                                   target_dim = dimension,
                                  band = band,
                                  source_extent = source_extent,
                                  resample = resample,
                                  silent = silent)
  names(vals) <- sprintf("Band%i",band)
  vals
}




