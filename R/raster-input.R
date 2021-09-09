.r_to_gdal_datatype <- function(x) {
  if (nchar(x) == 0 || is.na(x)) return("")
  xout <- toupper(x[1L])
  xout <- c("RAW" = "Byte", "INTEGER" = "Int32", "DOUBLE" = "Float64", "NUMERIC" = "Float64", 
    "BYTE" = "Byte", 
    "UINT16" = "Int32", "INT16" = "Int32", 
    "UINT32" = "Int32", "INT32" = "Int32", 
    "FLOAT32"  = "Float32", "FLOAT64" = "Float64")[xout]
  if (is.na(xout)) {
    message(sprintf("unknown 'band_output_type = \'%s\'', ignoring", x))
    xout <- ""
  }
  xout
}

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
#' 
#' 'band_output_type' can be 'raw', 'integer', 'double', or case-insensitive versions of the GDAL types
#' 'Byte', 'UInt16', 'Int16', 'UInt32', 'Int32', 'Float32', or 'Float64'. These are mapped to one of the 
#' supported types 'Byte' ('== raw'), 'Int32' ('== integer'), or 'Float64' ('== double'). 
#'  
#' @param x data source
#' @param band index of which band to read (1-based)
#' @param window src_offset, src_dim, out_dim
#' @param resample resampling method used (see details)
#' @param ... reserved
#' @param native apply the full native window for read, `FALSE` by default
#' @param sds index of subdataset to read (usually 1)
#' @param set_na specify whether NA values should be set for the NODATA
#' @param band_output_type numeric type of band to apply (else the native type if ''), is mapped to one of 'Byte', 'Int32', or 'Float64'
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
vapour_read_raster <- function(x, band = 1, window, resample = "nearestneighbour", ..., sds = NULL, native = FALSE, set_na = TRUE, band_output_type = "") {
  
  band_output_type <- .r_to_gdal_datatype(band_output_type)
  datasourcename <- sds_boilerplate_checks(x, sds = sds)
  resample <- tolower(resample)  ## ensure check internally is lower case
  if (!resample %in% c("nearestneighbour", "average", "bilinear", "cubic", "cubicspline",
                       "gauss", "lanczos", "mode")) {
    warning(sprintf("resample mode '%s' is unknown", resample))
  }
  ri <- vapour_raster_info(x, sds = sds)
  if (native && !missing(window)) warning("'window' is specified, so 'native = TRUE' is ignored")
  if (native && missing(window)) window <- c(0, 0, ri$dimXY, ri$dimXY)
  
  if (!is.numeric(band) || band < 1 || length(band) < 1  || anyNA(band)) {
    stop("'band' must be an integer of length 1, and be greater than 0")
  }
  if (any(band > ri$bands)) stop(sprintf("specified 'band = %i', but maximum band number is %i", band, ri$bands))
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
  ##vals <- raster_io_gdal_cpp(dsn = datasourcename, window  = window, band = band, resample = resample[1L], band_output_type = band_output_type)
  vals <- lapply(band, function(iband) {
    raster_io_gdal_cpp(dsn = datasourcename, window  = window, band = iband, resample = resample[1L], band_output_type = band_output_type)[[1L]]
  })
  if (set_na && !is.raw(vals[[1L]][1L])) {
    for (i in seq_along(vals)) {
      vals[[i]][vals[[i]] == ri$nodata_value] <- NA   ## hardcode to 1 for now
    }
  }
  names(vals) <- sprintf("Band%i",band)
  vals
}

#' type safe(r) raster read
#'
#' These wrappers around [vapour_read_raster()] guarantee single vector output of the nominated type.
#'
#' _hex and _chr are aliases of each other.
#' @inheritParams vapour_read_raster
#' @aliases vapour_read_raster_raw vapour_read_raster_int vapour_read_raster_dbl vapour_read_raster_chr vapour_read_raster_hex
#' @export
#' @return atomic vector of the nominated type raw, int, dbl, or character (hex)
#' @examples
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' vapour_read_raster_int(f, window = c(0, 0, 5, 4))
#' vapour_read_raster_raw(f, window = c(0, 0, 5, 4))
#' vapour_read_raster_chr(f, window = c(0, 0, 5, 4))
#' plot(vapour_read_raster_dbl(f, native = TRUE), pch = ".", ylim = c(273, 300))
vapour_read_raster_raw <- function(x, band = 1,
                                   window,
                                   resample = "nearestneighbour", ...,
                                   sds = NULL, native = FALSE, set_na = TRUE) {
  
  
  if (length(band) > 1) message("_raw output implies one band, using only the first")
  vapour_read_raster(x, band = band, window = window, resample = resample, ..., sds = sds,
                     native = native, set_na = set_na, band_output_type = "Byte")[[1L]]
}

#' @name vapour_read_raster_raw
#' @export
vapour_read_raster_int <- function(x, band = 1,
                                   window,
                                   resample = "nearestneighbour", ...,
                                   sds = NULL, native = FALSE, set_na = TRUE) {
  
  if (length(band) > 1) message("_int output implies one band, using only the first")
  vapour_read_raster(x, band = band, window = window, resample = resample, ..., sds = sds,
                     native = native, set_na = set_na, band_output_type = "Int32")[[1L]]
}

#' @name vapour_read_raster_raw
#' @export
vapour_read_raster_dbl <- function(x, band = 1,
                                   window,
                                   resample = "nearestneighbour", ...,
                                   sds = NULL, native = FALSE, set_na = TRUE) {
  
  if (length(band) > 1) message("_dbl output implies one band, using only the first")
  vapour_read_raster(x, band = band, window = window, resample = resample, ..., sds = sds,
                     native = native, set_na = set_na, band_output_type = "Float64")[[1L]]
}


#' @name vapour_read_raster_raw
#' @export
vapour_read_raster_chr <- function(x, band = 1,
                                   window,
                                   resample = "nearestneighbour", ...,
                                   sds = NULL, native = FALSE, set_na = TRUE) {
  
  ## band must be length 1, 3 or 4
  if (length(band) == 2 || length(band) > 4) message("_chr output implies one, three or four bands ...")
  if (length(band) == 2L) band <- band[1L]
  if (length(band) > 4) band <- band[1:4]
  bytes <- vapour_read_raster(x, band = band, window = window, resample = resample, ..., sds = sds,
                              native = native, set_na = set_na, band_output_type = "Byte")
  
  ## pack into character with as.raster ...
  
  ## note that we replicate out *3 if we only have one band ... (annoying of as.raster)
  as.vector(grDevices::as.raster(array(unlist(bytes, use.names = FALSE), c(length(bytes[[1]]), 1, max(c(3, length(bytes)))))))
}

#' @name vapour_read_raster_raw
#' @export
vapour_read_raster_hex <- function(x, band = 1,
                                   window,
                                   resample = "nearestneighbour", ...,
                                   sds = NULL, native = FALSE, set_na = TRUE) {
  vapour_read_raster_chr(x, band = band, window = window, resample = resample, sds = sds,
                         native = native, set_na = set_na, ...)
}




#' Raster warper (reprojection)
#'
#' Read a window of data from a GDAL raster source through a warp specification.
#'  The warp specification is provided by 'extent',
#' 'dimension', and 'projection' properties of the transformed output.
#'
#' Any bands may be read, including repeats. 
#' 
#' This function is not memory safe, the source is left on disk but the output
#' raster is all computed in memory so please be careful with very large values
#' for 'dimension'. `1000 * 1000 * 8` for 1000 columns, 1000 rows and floating point
#' double type will be 8Mb.
#'
#' There's control over the output type, and is auto-detected from the source (raw/Byte, integer/Int32, numeric/Float64) or
#' can be set with 'band_output_type'. 
#'
#' 'projection' refers to the full Well-Known-Text specification of a coordinate reference
#' system. See [vapour_srs_wkt()] for conversion from PROJ.4 string to WKT. Any string
#' accepted by GDAL may be used for 'projection' or 'source_wkt', including EPSG strings, PROJ4 strings, and
#' file names. Note that this argument was named 'wkt' up until version 0.8.0. 
#'
#' 'extent' is the four-figure xmin,xmax,ymin,ymax outer corners of corner pixels
#'
#' 'dimension' is the pixel dimensions of the output, x (ncol) then y (nrow).
#'
#' Values for missing data are not yet handled, just returned
#' as-is. Note that there may be regions of "zero data" in a warped output,
#' separate from propagated missing "NODATA" values in the source.
#'
#' Argument 'source_wkt' may be used to assign the projection of the source, 'source_extent'
#' to assign the extent of the source. Sometimes both are required.
#'
#' If multiple sources are specified via 'x' and either 'source_wkt' or 'source_extent' are provided, these
#' are applied to every source even if they have valid values already. If this is not sensible please use VRT
#' to wrap the multiple sources first (see the gdalio package for some in-dev ideas).
#'
#' Wild combinations of
#' 'source_extent' and/or 'extent' may be used for arbitrary flip orientations, scale and offset. For
#' expert usage only. Old versions allowed transform input for target and source but this is now disabled (maybe we'll write
#'  a new wrapper for that).
#'
#' @section Options: 
#' 
#' The 'warp_options' arguments are for 'warp options -wo', 
#' 
#' 'transformation options -to', 'creation options -co', 
#' 'open options -oo', or 'dataset open options -doo', and other arguments that use named options in gdalwarp. 
#' 
#' To input use the appropriate argument 'warp_options' for '-wo', 'transformation_options' for '-to'. 
#' 
#' 'warp_options = c("SAMPLE_GRID=YES", "SAMPLE_STEPS=30") '
#' 
#'  Do not include the '-wo' or the '-to', and make sure each is a separate character element. These are added in turn
#'  with '-wo' or '-to' prepended to the string list in the implementation. 
#'  
#' There are no creation options '-co' available, because the MEM driver is used. This might changed, see for example 'vapour_write_raster_block'. 
#' We might add '-oo', '-doo' in future. 
#'  
#' 
#' See [GDALWarpOptions](https://gdal.org/api/gdalwarp_cpp.html#_CPPv4N15GDALWarpOptions16papszWarpOptionsE) for '-wo'. 
#' 
#' See [GDAL transformation options](https://gdal.org/api/gdal_alg.html#_CPPv432GDALCreateGenImgProjTransformer212GDALDatasetH12GDALDatasetHPPc) for '-to'. 
#' 
#' Note we already apply the following gdalwarp arguments based on input R arguments to this function. 
#' 
#' * **-of**      MEM is hardcoded, but may be extended in future
#' * **-t_srs**   set via 'projection'
#' * **-s_srs**   set via 'source_wkt'
#' * **-te**      set via 'extent'
#' * **-ts**      set via 'dimension'
#' * **-r**       set via 'resample'
#' 
#' note that 'source_extent' does nothing atm, there's no -se (you have to use VRT and we'll do that via new /vsivrt/ ...)
#' bundle any required options into 'options'. 
#' 
#' @param x vector of data source names (file name or URL or database connection string)
#' @param bands index of band/s to read (1-based), may be new order or replicated, or NULL (all bands used)
#' @param extent extent of the target warped raster 'c(xmin, xmax, ymin, ymax)'
#' @param source_extent extent of the source raster, used to override/augment incorrect source metadata
#' @param geotransform DEPRECATED use 'extent' the affine geotransform of the warped raster
#' @param dimension dimensions in pixels of the warped raster (x, y)
#' @param projection projection of warped raster (in Well-Known-Text, or any projection string accepted by GDAL)
#' @param set_na NOT IMPLEMENTED logical, should 'NODATA' values be set to `NA`
#' @param source_geotransform DEPRECATED use 'source_extent' (override the native geotransform of the source)
#' @param resample resampling method used (see details in [vapour_read_raster])
#' @param source_wkt optional, override or augment the projection of the source (in Well-Known-Text, or any projection string accepted by GDAL)
#' @param silent `TRUE` by default, set to `FALSE` to report messages
#' @param band_output_type numeric type of band to apply (else the native type if '') can be one of 'Byte', 'Int32', or 'Float64' but see details in [vapour_read_raster()]
#' @param ... unused
#' @param warp_options character vector of options, as in gdalwarp -wo - see Details
#' @param transformation_options character vector of options, as in gdalwarp -to
#' @export
#' @seealso vapour_read_raster vapour_read_raster_raw vapour_read_raster_int vapour_read_raster_dbl vapour_read_raster_chr vapour_read_raster_hex
#' @return list of vectors (only 1 for 'band') of numeric values, in raster order
#' @examples
#' b <- 4e5
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' prj <- "+proj=aeqd +lon_0=147 +lat_0=-42"
#' vals <- vapour_warp_raster(f, extent = c(-b, b, -b, b),
#'                              dimension = c(186, 298),
#'                              bands = 1, 
#'                              projection = vapour_srs_wkt(prj), 
#'                              warp_options = c("SAMPLE_GRID=YES"))
#'                              
#'                              
#' image(list(x = seq(-b, b, length.out = 187), y = seq(-b, b, length.out = 298),
#'     z = matrix(unlist(vals, use.names = FALSE), 186)[,298:1]), asp = 1)
vapour_warp_raster <- function(x, bands = 1L,
                               extent = NULL,
                               dimension = NULL,
                               projection = "",
                               set_na = TRUE,
                               source_wkt = NULL,
                               source_extent = 0.0,
                               resample = "near",
                               silent = TRUE, ...,
                               source_geotransform = 0.0, geotransform = NULL,
                               band_output_type = "", 
                               warp_options = "", 
                               transformation_options = "") {
  band_output_type <- .r_to_gdal_datatype(band_output_type)
  
  args <- list(...)
  
  if (projection == "" && "wkt" %in% names(args)) {
    projection <- args$wkt
    message("please use 'projection = ' rather than 'wkt = ', use of 'wkt' is deprecated and will be removed in a future version")
  }
  
  ## bands
  if (is.numeric(bands) && any(bands < 1)) stop("all 'bands' index must be >= 1")
  if (is.null(bands)) bands <- 0
  if(!is.numeric(bands)) stop("'bands' must be numeric (integer), start at 1")
  bands <- as.integer(bands)
  ##if ("projection" %in% names(list(...))) message("argument 'projection' input is ignored, warper functions use 'wkt = ' to specify target projection (any format is ok)")
  # dud_extent <- FALSE
  # if (is.null(extent)) {
  #   ## set a dummy and then pass in dud after following tests
  #   extent <- c(0, 1, 0, 1)
  #   dud_extent <- TRUE
  # }
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
  
  ## if (dud_extent) extent <- 0.0
  ## hmm, we can't rely on gdalwarp to give a sensibleish dimension if not specified, it goes for the native-res
  dud_dimension <- FALSE
  ## we dud it if no target projection is set, so you get native from the extent
  if (is.null(dimension) && nchar(projection) < 1) {
    ## NO. We can't heuristic dimension or extent because we don't have a format to return those values with
    ##  we make a simple raster, the image() thing and go with that
    
    ## FIXME: move this hardcode to C, and override with min(native_dimension, dimension)
    #dimension <- c(512, 512)
    
    ## we could
    ##  ## hardcode a default options(vapour.warp.default.dimension = c(512, 512))
    ##  ## modify hardcode based on extent aspect ratio (not lat corrected)
    ## ## modify harcode to not exceed the native (I think I like this the best, because it reduces logic churn and delays when
    ## ##  that has to be set in the C++, but we need to send down a message that the default is used (so do it all in C is the summ))
    ## set it to native with a max
    ## set it to native with a warn/override
    dud_dimension <- TRUE
    dimension <- c(2, 2)
  }
  if(!is.numeric(dimension)) stop("'dimension' must be numeric")
  if(!length(dimension) == 2L) stop("'dimension must be of length 2'")
  if(!all(dimension > 0)) stop("'dimension' values must be greater than 0")
  if(!all(is.finite(dimension))) stop("'dimension' values must be finite and non-missing")
  if (dud_dimension) dimension <- 0
  
  
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
    if(!silent) {
      if(!nchar(source_wkt) > 10) message("short 'source_wkt', possibly invalid?")
    }
  }
  
  if (!silent) {
    if(!nchar(projection) > 0) message("target 'projection' not provided, read will occur from from source in native projection")
  }

  if (is.null(source_wkt)) source_wkt <-  ""
  
  resample <- tolower(resample[1L])
  if (resample == "gauss") {
    warning("Gauss resampling not available for warper, using NearestNeighbour")
    resample <- "near"
  }
  rso <- c("near", "bilinear", "cubic", "cubicspline", "lanczos", "average",
           "mode", "max", "min", "med", "q1", "q3", "sum") #, "rms")
  
  if (!resample %in% rso) {
    warning(sprintf("%s resampling not available for warper, using near", resample))
    resample <- "near"
    
  }
  
  warp_options <- warp_options[!is.na(warp_options)]
  if (length(warp_options) < 1) warp_options <- ""
  transformation_options <- transformation_options[!is.na(transformation_options)]
  if (length(transformation_options) < 1) transformation_options <- ""
  
  vals <- warp_in_memory_gdal_cpp(x, source_WKT = source_wkt,
                                  target_WKT = projection,
                                  target_extent = as.numeric(extent),
                                  target_dim = as.integer(dimension),
                                  bands = as.integer(bands),
                                  source_extent = as.numeric(source_extent),
                                  resample = resample,
                                  silent = silent,
                                  band_output_type = band_output_type, 
                                  warp_options = warp_options, 
                                  transformation_options = transformation_options)
  if (length(bands) == 1 && bands == 0) {
    ## we got all bands by index
    bands <- seq_along(vals)
  }
  names(vals) <- make.names(sprintf("Band%i",bands), unique = TRUE)
  vals
}


#' type safe(r) raster warp
#'
#' These wrappers around [vapour_warp_raster()] guarantee single vector output of the nominated type.
#'
#' _hex and _chr are aliases of each other.
#' @inheritParams vapour_warp_raster
#' @aliases vapour_warp_raster_raw vapour_warp_raster_int vapour_warp_raster_dbl vapour_warp_raster_chr vapour_warp_raster_hex
#' @name vapour_warp_raster_raw
#' @export
#' @return atomic vector of the nominated type raw, int, dbl, or character (hex)
#' @examples
#' b <- 4e5
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' prj <- "+proj=aeqd +lon_0=147 +lat_0=-42"
#' bytes <- vapour_warp_raster_raw(f, extent = c(-b, b, -b, b),
#'                              dimension = c(18, 2),
#'                              bands = 1, 
#'                              projection = prj)
#' # not useful given source type floating point, but works
#' str(bytes)
vapour_warp_raster_raw <- function(x, bands = 1L,
                               extent = NULL,
                               dimension = NULL,
                               projection = "",
                               set_na = TRUE,
                               source_wkt = NULL,
                               source_extent = 0.0,
                               resample = "near",
                               silent = TRUE, ...,
                               warp_options = "", 
                               transformation_options = "") {
  if (length(bands) > 1 ) message("_raw output implies one band, ignoring all but the first")
  
  vapour_warp_raster(x, 
                     bands = bands, 
                     extent = extent, 
                     dimension = dimension, 
                     projection = projection, 
                     set_na = set_na, 
                     source_wkt = source_wkt, 
                     source_extent = source_extent, 
                     resample = resample, 
                     silent = silent, 
                     band_output_type = "Byte",
                     warp_options = warp_options, 
                     transformation_options = transformation_options, ...)[[1L]]
}


#' @name vapour_warp_raster_raw
#' @export
vapour_warp_raster_int <- function(x, bands = 1L,
                                   extent = NULL,
                                   dimension = NULL,
                                   projection = "",
                                   set_na = TRUE,
                                   source_wkt = NULL,
                                   source_extent = 0.0,
                                   resample = "near",
                                   silent = TRUE, ...,
                                   warp_options = "", 
                                   transformation_options = "")  {
  if (length(bands) > 1 ) message("_int output implies one band, ignoring all but the first")

  vapour_warp_raster(x, 
                     bands = bands, 
                     extent = extent, 
                     dimension = dimension, 
                     projection = projection, 
                     set_na = set_na, 
                     source_wkt = source_wkt, 
                     source_extent = source_extent, 
                     resample = resample, 
                     silent = silent, 
                     band_output_type = "Int32",
                     warp_options = warp_options, 
                     transformation_options = transformation_options, ...)[[1L]]
}

#' @name vapour_warp_raster_raw
#' @export
vapour_warp_raster_dbl <- function(x, bands = 1L,
                                   extent = NULL,
                                   dimension = NULL,
                                   projection = "",
                                   set_na = TRUE,
                                   source_wkt = NULL,
                                   source_extent = 0.0,
                                   resample = "near",
                                   silent = TRUE, ...,
                                   warp_options = "", 
                                   transformation_options = "") {
  if (length(bands) > 1 ) message("_dbl output implies one band, ignoring all but the first")
  
  vapour_warp_raster(x, 
                     bands = bands, 
                     extent = extent, 
                     dimension = dimension, 
                     projection = projection, 
                     set_na = set_na, 
                     source_wkt = source_wkt, 
                     source_extent = source_extent, 
                     resample = resample, 
                     silent = silent, 
                     band_output_type = "Float64",
                     warp_options = warp_options, 
                     transformation_options = transformation_options, ...)[[1L]]
}


#' @name vapour_warp_raster_raw
#' @export
vapour_warp_raster_chr <- function(x, bands = 1L,
                                   extent = NULL,
                                   dimension = NULL,
                                   projection = "",
                                   set_na = TRUE,
                                   source_wkt = NULL,
                                   source_extent = 0.0,
                                   resample = "near",
                                   silent = TRUE, ...,
                                   warp_options = "", 
                                   transformation_options = "") {
  ## band must be length 1, 3 or 4
  if (length(bands) == 2 || length(bands) > 4) message("_chr output implies one, three or four bands ...")
  if (length(bands) == 2L) bands <- bands[1L]
  if (length(bands) > 4) bands <- bands[1:4]
  bytes  <- vapour_warp_raster(x, 
                     bands = bands, 
                     extent = extent, 
                     dimension = dimension, 
                     projection = projection, 
                     set_na = set_na, 
                     source_wkt = source_wkt, 
                     source_extent = source_extent, 
                     resample = resample, 
                     silent = silent, 
                     band_output_type = "Byte",
                     warp_options = warp_options, 
                     transformation_options = transformation_options, ...)
  ## note that we replicate out *3 if we only have one band ... (annoying of as.raster)
  as.vector(grDevices::as.raster(array(unlist(bytes, use.names = FALSE), c(length(bytes[[1]]), 1, max(c(3, length(bytes)))))))
  
}


#' @name vapour_warp_raster_raw
#' @export
vapour_warp_raster_hex <- function(x, bands = 1L,
                                   extent = NULL,
                                   dimension = NULL,
                                   projection = "",
                                   set_na = TRUE,
                                   source_wkt = NULL,
                                   source_extent = 0.0,
                                   resample = "near",
                                   silent = TRUE, ...,
                                   warp_options = "", 
                                   transformation_options = "") {
  vapour_warp_raster_chr(x, 
                     bands = bands, 
                     extent = extent, 
                     dimension = dimension, 
                     projection = projection, 
                     set_na = set_na, 
                     source_wkt = source_wkt, 
                     source_extent = source_extent, 
                     resample = resample, 
                     silent = silent, 
                     warp_options = warp_options, 
                     transformation_options = transformation_options, ...)
}
