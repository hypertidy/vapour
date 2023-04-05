

#' General raster read and convert
#' 
#' The warper is used to convert source/s to an output file or to data in memory. 
#'
#' Two functions 'gdal_raster_data' and 'gdal_raster_dsn' act like the gdalwarp command line
#' tool, a convenience third function 'gdal_raster_image()' works especially for image data. 
#' 
#' @param dsn data sources, files, urls, db strings, vrt, etc
#' @param target_crs projection of the target grid
#' @param target_dim dimension of the target grid
#' @param target_ext extent of the target grid
#' @param target_res resolution of the target grid
#' @param resample resampling algorithm used
#' @param bands band or bands to include, default is first band only (use NULL or a value less that one to obtain all bands)
#' @param band_output_type specify the band type, see [vapour_read_raster]
#' @param options general options passed to gdal warper
#' @param out_dsn use with [gdal_raster_dsn] optionally set the output file name (or one will be generated)
#' @export
#' @returns pixel values in a list vector per band, or a list of file paths
#'
#' @examples
#' dsn <- system.file("extdata/sst.tif", package = "vapour")
#' par(mfrow = c(2, 2))
#' ## do nothing, get native
#' X <- gdal_raster_data(dsn)
#' 
#' ## set resolution (or dimension, extent, crs, or combination thereof - GDAL 
#' ## will report/resolve incompatible opts)
#' X1 <- gdal_raster_data(dsn,  target_res = 1)
#' 
#' ## add a cutline, and cut to it using gdal warp args
#' cutline <- system.file("extdata/cutline_sst.gpkg", package = "vapour")
#' X1c <- gdal_raster_data(dsn,  target_res = .1, options = c("-cutline",cutline, "-crop_to_cutline" ))
#' 
#' ## warp whole grid to give res
#' X2 <- gdal_raster_data(dsn,  target_res = 25000, target_crs = "EPSG:32755")
#' 
#' ## specify exactly (as per vapour originally)
#' X3 <- gdal_raster_data(dsn,  target_ext = c(-1, 1, -1, 1) * 8e6, 
#'  target_dim = c(512, 678), target_crs = "+proj=stere +lon_0=147 +lat_0=-90")
#'  
#' X4 <- gdal_raster_dsn(dsn, out_dsn = tempfile(fileext = ".tif"))
gdal_raster_data <- function(dsn, target_crs = NULL, target_dim = NULL, target_ext = NULL, target_res = NULL, 
                         resample = "near", bands = 1L, band_output_type = NULL, options = character()) {
  
   if (is.null(target_crs)) target_crs <- "" 
   if (is.null(target_ext)) {
     target_ext <-  numeric()
   } else {
     if (!length(target_ext) == 4L ) stop("'target_ext' must be of length 4 (xmin, xmax, ymin, ymax")
     
     if (anyNA(target_ext)) stop("NA values in 'target_ext'")
     dif <- diff(target_ext)[c(1L, 3L)]
     if (any(!dif > 0)) stop("all 'target_ext' values must xmin < xmax, ymin < ymax")
     
   }
   if (is.null(target_dim)) {
     target_dim <- integer() #info$dimension
   } else {
     if (length(target_dim) > 0 ) target_dim <- as.integer(rep(target_dim, length.out = 2L))
     
     if (anyNA(target_dim)) stop("NA values in 'target_dim'")
     if (any(target_dim < 0)) stop("all 'target_dim' values must be >= 0")
     if (all(target_dim < 1)) stop("one 'target_dim' value must be > 0")
     
   }
   if (is.null(target_res)) {
     target_res <- numeric() ## TODO
   } else {
     ## check res is above zero
     if (length(target_res) > 0 ) target_res <- as.numeric(rep(target_res, length.out = 2L))
     
     if (anyNA(target_res)) stop("NA values in 'target_res'")
     if (any(target_res <= 0)) stop("all 'target_res' values must be > 0")
   }
   if (is.null(band_output_type)) band_output_type <- "Float64"
   if (is.null(bands)) bands <- -1
   warp_general_cpp(dsn, target_crs, 
                             as.numeric(target_ext), 
                             as.integer(target_dim), 
                             as.numeric(target_res), 
                             bands = as.integer(bands), 
                             resample = resample, 
                             silent = FALSE, band_output_type = band_output_type, 
                             options = options, 
                             dsn_outname = "")
}

#' @name gdal_raster_data
#' @export
gdal_raster_dsn <- function(dsn, target_crs = NULL, target_dim = NULL, target_ext = NULL, target_res = NULL, 
                             resample = "near", bands = NULL, band_output_type = NULL, options = character(), out_dsn = tempfile(fileext = ".tif")) {
  
  if (is.null(target_crs)) target_crs <- "" 
  if (is.null(target_ext)) {
    target_ext <-  numeric()
  } else {
    if (!length(target_ext) == 4L ) stop("'target_ext' must be of length 4 (xmin, xmax, ymin, ymax")
    
    if (anyNA(target_ext)) stop("NA values in 'target_ext'")
    dif <- diff(target_ext)[c(1L, 3L)]
    if (any(!dif > 0)) stop("all 'target_ext' values must xmin < xmax, ymin < ymax")
    
  }
  if (is.null(target_dim)) {
    target_dim <- integer() #info$dimension
  } else {
    if (length(target_dim) > 0 ) target_dim <- as.integer(rep(target_dim, length.out = 2L))
    if (anyNA(target_dim)) stop("NA values in 'target_dim'")
    if (any(target_dim < 0)) stop("all 'target_dim' values must be >= 0")
    if (all(target_dim < 1)) stop("one 'target_dim' value must be > 0")
    
  }
  if (is.null(target_res)) {
    target_res <- numeric() ## TODO
  } else {
    ## check res is above zero
    if (length(target_res) > 0 ) target_res <- as.numeric(rep(target_res, length.out = 2L))
    
    if (anyNA(target_res)) stop("NA values in 'target_res'")
    if (any(target_res <= 0)) stop("all 'target_res' values must be >= 0")
  }
  if (is.null(band_output_type)) band_output_type <- "Float64"
  #if (grepl("tif$", out_dsn)) {
    ## we'll have to do some work here
  ## currently always COG
  options <- c(options, "-of",  "COG")
  
 if (!is.null(bands) || (is.integer(bands) && !length(bands) == 1 && bands[1] > 0)) {
   stop("bands cannot be set for gdal_raster_dsn, please use an upfront call to 'vapour_vrt(dsn,  bands = )' to create the dsn")
 } else {
   bands <- -1
 }
  
  warp_general_cpp(dsn, target_crs, 
                            target_ext, 
                            target_dim, 
                            target_res, 
                            bands = bands, 
                            resample = resample, 
                            silent = FALSE, band_output_type = band_output_type, 
                            options = options, 
                            dsn_outname = out_dsn[1L])
}

#' @name gdal_raster_data
#' @export
gdal_raster_image <- function(dsn, target_crs = NULL, target_dim = NULL, target_ext = NULL, target_res = NULL, 
                               resample = "near", bands = NULL, band_output_type = NULL, options = character()) {
  
  if (length(target_res) > 0 ) target_res <- as.numeric(rep(target_res, length.out = 2L))
  if (is.null(target_crs)) target_crs <- "" 
  if (is.null(target_ext)) target_ext <-  numeric()
  if (is.null(target_dim)) target_dim <- integer() #info$dimension
  if (is.null(target_res)) target_res <- numeric() ## TODO
  if (is.null(band_output_type)) band_output_type <- "UInt8"
  if (is.null(bands)) {
    nbands <- vapour_raster_info(dsn[1])$bands
    bands <- seq(min(c(nbands, 4L)))
  }
  bytes <- warp_general_cpp(dsn, target_crs, 
                            target_ext, 
                            target_dim, 
                            target_res, 
                            bands = bands, 
                            resample = resample, 
                            silent = FALSE, band_output_type = band_output_type, 
                            options = options, dsn_outname = "")
  atts <- attributes(bytes)
  out <- list(as.vector(grDevices::as.raster(array(unlist(bytes, use.names = FALSE), c(length(bytes[[1]]), 1, max(c(3, length(bytes))))))))
  attributes(out) <- atts
  out
}


