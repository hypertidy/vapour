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
#' @param unscale default is `TRUE` so native values will be converted by offset and scale to floating point
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
vapour_read_raster <- function(x, band = 1, window, resample = "nearestneighbour", ..., sds = NULL, native = FALSE, set_na = TRUE, band_output_type = "", unscale = TRUE) {
  x <- .check_dsn_single(x)
  if (!length(native) == 1L || is.na(native[1]) || !is.logical(native)) {
    stop("'native' must be a single 'TRUE' or 'FALSE'")
  }
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
    raster_io_gdal_cpp(dsn = datasourcename, window  = window, band = iband, resample = resample[1L], band_output_type = band_output_type, unscale = unscale)[[1L]]
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
#' `*_hex` and `*_chr` are aliases of each other.
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
#' The warp specification is provided by 'extent', 'dimension', and 'projection'
#' properties of the transformed output.
#'
#' Any bands may be read, including repeats. 
#' 
#' This function is not memory safe, the source is left on disk but the output
#' raster is all computed in memory so please be careful with very large values
#' for 'dimension'. `1000 * 1000 * 8` for 1000 columns, 1000 rows and floating
#' point double type will be 8Mb. 
#'
#' There's control over the output type, and is auto-detected from the source
#' (raw/Byte, integer/Int32, numeric/Float64) or can be set with
#' 'band_output_type'.
#'
#' 'projection' refers to any projection string for a CRS understood by GDAL.
#' This includes the full Well-Known-Text specification of a coordinate
#' reference system, PROJ strings, "AUTH:CODE" types, and others. See
#' [vapour_srs_wkt()] for conversion from PROJ.4 string to WKT, and
#' [vapour_raster_info()] and [vapour_layer_info()] for various formats
#' available from a data source. Any string accepted by GDAL may be used for
#' 'projection' or 'source_projection', including EPSG strings, PROJ4 strings,
#' and file names. Note that this argument was named 'wkt' up until version
#' 0.8.0.
#'
#' 'extent' is the four-figure xmin,xmax,ymin,ymax outer corners of corner pixels
#'
#' 'dimension' is the pixel dimensions of the output, x (ncol) then y (nrow).
#'
#' Options for missing data are not yet handled, just returned as-is. Note that
#' there may be regions of "zero data" in a warped output, separate from
#' propagated missing "NODATA" values in the source.
#'
#' Argument 'source_projection' may be used to assign the projection of the
#' source, 'source_extent' to assign the extent of the source. Sometimes both
#' are required. Note, this is now better done by creating 'VRT', see [vapour_vrt()]
#' for assigning the source projection, extent, and some other options. 
#'
#' If multiple sources are specified via 'x' and either 'source_projection' or
#' 'source_extent' are provided, these are applied to every source even if they
#' have valid values already. If this is not sensible please use VRT to wrap the
#' multiple sources first.
#'
#' Wild combinations of 'source_extent' and/or 'extent' may be used for
#' arbitrary flip orientations, scale and offset. For expert usage only. Old
#' versions allowed transform input for target and source but this is now
#' disabled (maybe we'll write a new wrapper for that).
#' 
#' @section Options: 
#' 
#' The various options are convenience arguments  for 'warp options -wo',
#' transformation options -to', 'open options -oo', and 'options' for any other
#' arguments in gdalwarp. There are no 'creation options -co' or 'dataset output
#' options -doo', because these are not supported by the MEM driver.
#'
#' All 'warp_options' are paired with a '-wo' declaration and similarly for '-to', and '-oo', 
#' this is purely a convenience, since 'options' itself can be used for these as well but we recommend using
#' the individual arguments. 
#' An example for warp options is  `warp_options = c("SAMPLE_GRID=YES", "SAMPLE_STEPS=30")` and one for
#' general arguments might be 
#' 'options = c("-ovr", "AUTO", "-nomd", "-cutline", "/path/to/cut.gpkg", "-crop_to_cutline")'.  If they would 
#' be separated by spaces on the command line then include as separate elements in the options character vector. 
#'
#' 
#' See [GDALWarpOptions](https://gdal.org/api/gdalwarp_cpp.html#_CPPv4N15GDALWarpOptions16papszWarpOptionsE) for '-wo'. 
#' 
#' See [GDAL transformation options](https://gdal.org/api/gdal_alg.html#_CPPv432GDALCreateGenImgProjTransformer212GDALDatasetH12GDALDatasetHPPc) for '-to'. 
#' 
#' See [GDALWARP command line app](https://gdal.org/programs/gdalwarp.html) for further details. 
#' 
#' Note we already apply the following gdalwarp arguments based on input R
#' arguments to this function.
#' 
#' \describe{
#' \item{-of}{MEM is hardcoded, but may be extended in future}
#' \item{-t_srs}{set via 'projection'}
#' \item{-s_srs}{set via 'source_projection'}
#' \item{-te}{set via 'extent'}
#' \item{-ts}{set via 'dimension'}
#' \item{-r}{set via 'resample'}
#' \item{-ot}{set via 'band_output_type'}
#' \item{-te_srs}{ not supported}
#' \item{-a_ullr}{(not a gdalwarp argument, but we do analog) set via 'source_extent' use [vapour_vrt()] instead} 
#' }
#' 
#' In future all 'source_*' arguments may be deprecated in favour of
#' augmentation by 'vapour_vrt()'.
#'
#' Common inputs for `projection` are WKT variants, 'AUTH:CODE's e.g.
#' 'EPSG:3031', the 'OGC:CRS84' for lon,lat WGS84, 'ESRI:code' and other
#' authority variants, and datum names such as 'WGS84','NAD27' recognized by
#' PROJ itself.
#'
#' See help for 'SetFromUserInput' in 'OGRSpatialReference', and
#' 'proj_create_crs_to_crs'.
#' 
#' [c.proj_create_crs_to_crs](https://proj.org/development/reference/functions.html#c.proj_create_crs_to_crs) 
#' 
#' [c.proj_create](https://proj.org/development/reference/functions.html#c.proj_create)
#' 
#' [SetFromUserInput](https://gdal.org/doxygen/classOGRSpatialReference.html#aec3c6a49533fe457ddc763d699ff8796)
#' 
#' @param x vector of data source names (file name or URL or database connection string)
#' @param bands index of band/s to read (1-based), may be new order or replicated, or NULL (all bands used, the default)
#' @param extent extent of the target warped raster 'c(xmin, xmax, ymin, ymax)'
#' @param source_extent extent of the source raster, used to override/augment incorrect source metadata
#' @param dimension dimensions in pixels of the warped raster (x, y)
#' @param projection projection of warped raster (in Well-Known-Text, or any projection string accepted by GDAL)
#' @param set_na NOT IMPLEMENTED logical, should 'NODATA' values be set to `NA`
#' @param resample resampling method used (see details in [vapour_read_raster])
#' @param source_projection optional, override or augment the projection of the source (in Well-Known-Text, or any projection string accepted by GDAL)
#' @param silent `TRUE` by default, set to `FALSE` to report messages
#' @param band_output_type numeric type of band to apply (else the native type if '') can be one of 'Byte', 'Int32', or 'Float64' but see details in [vapour_read_raster()]
#' @param ... unused
#' @param warp_options character vector of options, as in gdalwarp -wo - see Details
#' @param transformation_options character vector of options, as in gdalwarp -to see Details
#' @param open_options character vector of options, as in gdalwarp -oo - see Details
#' @param options character vectors of options as per the gdalwarp command line 
#' @param nomd if `TRUE` the Metadata tag is removed from the resulting VRT (it can be quite substantial)
#' @param overview pick an integer overview from the source (0L is highest resolution, default -1L does nothing)
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
vapour_warp_raster <- function(x, bands = NULL,
                               extent = NULL,
                               dimension = NULL,
                               projection = "",
                               set_na = TRUE,
                               source_projection = NULL,
                               source_extent = 0.0,
                               resample = "near",
                               silent = TRUE, ...,
                               band_output_type = "", 
                               warp_options = "", 
                               transformation_options = "", 
                               open_options = "",
                               options = "", 
                               nomd = FALSE, overview = -1L) {
  x <- .check_dsn_multiple(x)
  if (!is.null(bands) && (anyNA(bands) || length(bands) < 1 || !is.numeric(bands))) {
    stop("'bands' must be a valid set of band integers (1-based)")
  }
  if (is.null(projection) || is.na(projection[1]) || length(projection) != 1L || !is.character(projection)) {
    stop("'projection' must be a valid character string for a GDAL/PROJ coordinate reference system (map projection)")
  }
  
  ## deprecated arguments
  if (!is.null(list(...)$source_wkt)) stop("'source_wkt' is defunct, please use 'source_projection'")
  if(!is.null(list(...)$source_geotransform)) stop("'source_geotransform' is defunct and now ignored, please use 'source_extent'")
  if (!is.null(list(...)$geotransform)) stop("'geotransform' is defunct and now ignored, used 'extent'")
  
  if (band_output_type == "vrt") {
    ## special case, do nothing
  } else {
    band_output_type <- .r_to_gdal_datatype(band_output_type)
  }
  
  args <- list(...)
  
  if (projection == "" && "wkt" %in% names(args)) {
    projection <- args$wkt
    message("please use 'projection = ' rather than 'wkt = ', use of 'wkt' is deprecated and will be removed in a future version")
  }
  
  ## bands
  if (is.numeric(bands) && any(bands < 1)) stop("all 'bands' index must be >= 1")
  if (is.null(bands)) bands <- 0
  if(length(bands) < 1 || anyNA(bands) || !is.numeric(bands)) stop("'bands' must be numeric (integer), start at 1")
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
  
  
  if (length(source_extent) > 1) {
    if (!is.numeric(source_extent)) {
      stop("'source_extent' must be numeric, of length 4 c(xmin, xmax, ymin, ymax)")
    }
    if (!all(is.finite(source_extent))) stop("'source_extent' values must be finite and non missing")
  }
  if(!is.null(source_projection)) {
    if (!is.character(source_projection)) stop("source_projection must be character")
    if(!silent) {
      if(!nchar(source_projection) > 10) message("short 'source_projection', possibly invalid?")
    }
  }
  
  if (!silent) {
    if(!nchar(projection) > 0) message("target 'projection' not provided, read will occur from from source in native projection")
  }
  
  if (is.null(source_projection)) source_projection <-  ""
  
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
  #if (length(warp_options) < 1) warp_options <- ""
  transformation_options <- transformation_options[!is.na(transformation_options)]
  #if (length(transformation_options) < 1) transformation_options <- ""
  open_options <- open_options[!is.na(open_options)]
 # if (length(open_options) < 1) open_options <- ""
 # dataset_output_options <- dataset_output_options[!is.na(dataset_output_options)]
  
  
  ## process all options into one big string list
  
  if (any(grepl("-wo", options) |
          grepl("-to", options) | 
          grepl("-oo", options))) {
    ##message("manually setting -wo, -to, -oo options detected, prefer use of 'warp_options', 'transformation_options', 'open_options'")
  } 
  
  if (nchar(warp_options)[1L] > 0) options <- c(options, rbind("-wo", warp_options))
  if (nchar(transformation_options)[1L] > 0)  options <- c(rbind("-to", transformation_options))
  if (nchar(open_options)[1L] > 0) options <- c(options, rbind("-oo", open_options))
  #if (nchar(dataset_output_options)[1L] > 0)  options <- c(rbind("-doo", dataset_output_options))

  options <- options[!is.na(options)]
  options <- options[nchar(options) > 0]
  if (length(options) < 1) options <- ""
  
  ## no -r, -te, -t_srs, -ts, -of, -s_srs, -ot, -te_srs we set them manually
  if (any(grepl("-r", options) |
      grepl("-te", options) | 
      grepl("-t_srs", options) | 
      grepl("-ts", options) |
      grepl("-of", options) |
      grepl("-s_srs", options) | 
      grepl("-ot", options))) {
    stop("manually setting -r, -te, -t_srs, -of, -s_srs, -ot options not allowed \n ( these controlled by arguments 'resample', 'target_extent', 'target_projection', '<MEM>', 'source_projection', 'band_output_type')")
  } 
  if (any(grepl("-te_srs", options))) stop("setting '-te_srs' projection of target extent is not supported") 
  vals <- warp_in_memory_gdal_cpp(x, source_WKT = source_projection,
                                  target_WKT = projection,
                                  target_extent = as.numeric(extent),
                                  target_dim = as.integer(dimension),
                                  bands = as.integer(bands),
                                  source_extent = as.numeric(source_extent),
                                  resample = resample,
                                  silent = silent,
                                  band_output_type = band_output_type, 
                                  options = options, nomd = nomd, overview)
  # ##// if we Dataset->RasterIO we don't have separated bands'
  # nbands <- length(vals[[1L]]) / prod(as.integer(dimension))
  # if (nbands > 1) vals <- split(vals[[1L]], rep(seq_len(nbands), each = prod(as.integer(dimension))))
  # 
  # names(vals) <- make.names(sprintf("Band%i",seq_len(nbands)), unique = TRUE)
  # 
  if (band_output_type == "vrt") return(vals[[1L]])
  
  bands <- seq_along(vals)
 
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
vapour_warp_raster_raw <- function(x, bands = NULL,
                                   extent = NULL,
                                   dimension = NULL,
                                   projection = "",
                                   set_na = TRUE,
                                   source_projection = NULL,
                                   source_extent = 0.0,
                                   resample = "near",
                                   silent = TRUE, ...,
                                   warp_options = "", 
                                   transformation_options = "", 
                                   open_options = "",
                                   options = "") {
  if (length(bands) > 1 ) message("_raw output implies one band, ignoring all but the first")
  
  vapour_warp_raster(x, 
                     bands = bands, 
                     extent = extent, 
                     dimension = dimension, 
                     projection = projection, 
                     set_na = set_na, 
                     source_projection = source_projection, 
                     source_extent = source_extent, 
                     resample = resample, 
                     silent = silent, 
                     band_output_type = "Byte", 
                     warp_options = warp_options, 
                     transformation_options = transformation_options,
                     open_options = open_options, 
                     options = options,...)[[1L]]
}

vapour_warp_raster_vrt <- function(x, bands = NULL,
                                   extent = NULL,
                                   dimension = NULL,
                                   projection = "",
                                   set_na = TRUE,
                                   source_projection = NULL,
                                   source_extent = 0.0,
                                   resample = "near",
                                   silent = TRUE, ..., 
                                   warp_options = "", 
                                   transformation_options = "", 
                                   open_options = "",
                                   options = "") {
  
  vapour_warp_raster(x, 
                     bands = bands, 
                     extent = extent, 
                     dimension = dimension, 
                     projection = projection, 
                     set_na = set_na, 
                     source_projection = source_projection, 
                     source_extent = source_extent, 
                     resample = resample, 
                     silent = silent, 
                     band_output_type = "vrt",
                     warp_options = warp_options, 
                     transformation_options = transformation_options,
                     open_options = open_options, 
                     options = options,...)[[1L]]
}

#' @name vapour_warp_raster_raw
#' @export
vapour_warp_raster_int <- function(x, bands = NULL,
                                   extent = NULL,
                                   dimension = NULL,
                                   projection = "",
                                   set_na = TRUE,
                                   source_projection = NULL,
                                   source_extent = 0.0,
                                   resample = "near",
                                   silent = TRUE, ..., 
                                   warp_options = "", 
                                   transformation_options = "", 
                                   open_options = "",
                                   options = "")  {
  if (length(bands) > 1 ) message("_int output implies one band, ignoring all but the first")
  
  vapour_warp_raster(x, 
                     bands = bands, 
                     extent = extent, 
                     dimension = dimension, 
                     projection = projection, 
                     set_na = set_na, 
                     source_projection = source_projection, 
                     source_extent = source_extent, 
                     resample = resample, 
                     silent = silent, 
                     band_output_type = "Int32",
                     warp_options = warp_options, 
                     transformation_options = transformation_options,
                     open_options = open_options, 
                     options = options,...)[[1L]]
}

#' @name vapour_warp_raster_raw
#' @export
vapour_warp_raster_dbl <- function(x, bands = NULL,
                                   extent = NULL,
                                   dimension = NULL,
                                   projection = "",
                                   set_na = TRUE,
                                   source_projection = NULL,
                                   source_extent = 0.0,
                                   resample = "near",
                                   silent = TRUE, ..., 
                                   warp_options = "", 
                                   transformation_options = "", 
                                   open_options = "",
                                   options = "") {
  if (length(bands) > 1 ) message("_dbl output implies one band, ignoring all but the first")
  
  vapour_warp_raster(x, 
                     bands = bands, 
                     extent = extent, 
                     dimension = dimension, 
                     projection = projection, 
                     set_na = set_na, 
                     source_projection = source_projection, 
                     source_extent = source_extent, 
                     resample = resample, 
                     silent = silent, 
                     band_output_type = "Float64",
                     warp_options = warp_options, 
                     transformation_options = transformation_options,
                     open_options = open_options, 
                     options = options,...)[[1L]]
}


#' @name vapour_warp_raster_raw
#' @export
vapour_warp_raster_chr <- function(x, bands = NULL,
                                   extent = NULL,
                                   dimension = NULL,
                                   projection = "",
                                   set_na = TRUE,
                                   source_projection = NULL,
                                   source_extent = 0.0,
                                   resample = "near",
                                   silent = TRUE, ..., 
                                   warp_options = "", 
                                   transformation_options = "", 
                                   open_options = "",
                                   options = "") {
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
                               source_projection = source_projection, 
                               source_extent = source_extent, 
                               resample = resample, 
                               silent = silent, 
                               band_output_type = "Byte",
                               warp_options = warp_options, 
                               transformation_options = transformation_options,
                               open_options = open_options, 
                               options = options,...)
  ## note that we replicate out *3 if we only have one band ... (annoying of as.raster)
  as.vector(grDevices::as.raster(array(unlist(bytes, use.names = FALSE), c(length(bytes[[1]]), 1, max(c(3, length(bytes)))))))
  
}


#' @name vapour_warp_raster_raw
#' @export
vapour_warp_raster_hex <- function(x, bands = NULL,
                                   extent = NULL,
                                   dimension = NULL,
                                   projection = "",
                                   set_na = TRUE,
                                   source_projection = NULL,
                                   source_extent = 0.0,
                                   resample = "near",
                                   silent = TRUE, ..., 
                                   warp_options = "", 
                                   transformation_options = "", 
                                   open_options = "",
                                   options = "") {
  vapour_warp_raster_chr(x, 
                         bands = bands, 
                         extent = extent, 
                         dimension = dimension, 
                         projection = projection, 
                         set_na = set_na, 
                         source_projection = source_projection, 
                         source_extent = source_extent, 
                         resample = resample, 
                         silent = silent, 
                         warp_options = warp_options, 
                         transformation_options = transformation_options,
                         open_options = open_options, 
                         options = options,...)
}
