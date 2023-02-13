

imfun <- function(X) {
  ximage::ximage(matrix(X[[1]], attr(X, "dimension")[2L], byrow = TRUE), 
                 extent = attr(X, "extent"), asp = 1)
  lines(whatarelief::coastline(attr(X, "extent"), projection = attr(X, "projection"), dimension = c(512, 512)))
}


## wrapper around in-dev vapour function to simplify the warper interface (let GDAL do the work)
#' Title
#'
#' @param dsn 
#' @param target_crs 
#' @param target_dim 
#' @param target_ext 
#' @param target_res 
#' @param resample 
#' @param bands 
#'
#' @return
#' @noRd
#'
#' @examples
#' dsn <- "inst/extdata/sst.tif"
#' par(mfrow = c(2, 2))
#' ## do nothing, get native
#' X <- gdal_raster_data(dsn)
#' imfun(X)
#' 
#' ## set resolution (or dimension, extent, crs, or combination thereof - GDAL will report/resolve incompatible opts)
#' X1 <- gdal_raster_data(dsn,  target_res = 1)
#' imfun(X1)
#' ## warp whole grid to give res
#' X2 <- gdal_raster_data(dsn,  target_res = 25000, target_crs = "EPSG:32755")
#' imfun(X2)
#' 
#' ## specify exactly (as per vapour originally)
#' X3 <- gdal_raster_data(dsn,  target_ext = c(-1, 1, -1, 1) * 8e6, target_dim = c(512, 678), target_crs = "+proj=stere +lon_0=147 +lat_0=-90")
#' imfun(X3)
gdal_raster_data <- function(dsn, target_crs = NULL, target_dim = NULL, target_ext = NULL, target_res = NULL, 
                         resample = "near", bands = 1L, band_output_type = NULL, options = character()) {
  
  if (length(target_res) > 0 ) target_res <- as.numeric(rep(target_res, length.out = 2L))
   if (is.null(target_crs)) target_crs <- "" 
   if (is.null(target_ext)) target_ext <-  numeric()
   if (is.null(target_dim)) target_dim <- integer() #info$dimension
   if (is.null(target_res)) target_res <- numeric() ## TODO
   if (is.null(band_output_type)) band_output_type <- "Float64"
   vapour:::warp_general_cpp(dsn, target_crs, 
                             as.numeric(target_ext), 
                             as.integer(target_dim), 
                             as.numeric(target_res), 
                             bands = as.integer(bands), 
                             resample = resample, 
                             silent = FALSE, band_output_type = band_output_type, 
                             options = options, 
                             dsn_outname = "")
}

gdal_raster_dsn <- function(dsn, target_crs = NULL, target_dim = NULL, target_ext = NULL, target_res = NULL, 
                             resample = "near", bands = 1L, band_output_type = NULL, options = character(), out_dsn = tempfile(fileext = ".tif")) {
  
  if (length(target_res) > 0 ) target_res <- as.numeric(rep(target_res, length.out = 2L))
  if (is.null(target_crs)) target_crs <- "" 
  if (is.null(target_ext)) target_ext <-  numeric()
  if (is.null(target_dim)) target_dim <- integer() #info$dimension
  if (is.null(target_res)) target_res <- numeric() ## TODO
  if (is.null(band_output_type)) band_output_type <- "Float64"
  #if (grepl("tif$", out_dsn)) {
    ## we'll have to do some work here
  ## currently always COG
  options <- c(options, "-of",  "COG")
  
  #}
  vapour:::warp_general_cpp(dsn, target_crs, 
                            target_ext, 
                            target_dim, 
                            target_res, 
                            bands = bands, 
                            resample = resample, 
                            silent = FALSE, band_output_type = band_output_type, 
                            options = options, 
                            dsn_outname = out_dsn[1L])
}
gdal_raster_image <- function(dsn, target_crs = NULL, target_dim = NULL, target_ext = NULL, target_res = NULL, 
                               resample = "near", bands = 1:3, band_output_type = NULL, options = character()) {
  
  if (length(target_res) > 0 ) target_res <- as.numeric(rep(target_res, length.out = 2L))
  if (is.null(target_crs)) target_crs <- "" 
  if (is.null(target_ext)) target_ext <-  numeric()
  if (is.null(target_dim)) target_dim <- integer() #info$dimension
  if (is.null(target_res)) target_res <- numeric() ## TODO
  if (is.null(band_output_type)) band_output_type <- "UInt8"
  bytes <- vapour:::warp_general_cpp(dsn, target_crs, 
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


