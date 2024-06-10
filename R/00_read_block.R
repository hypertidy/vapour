#' @name vapour_create
#' @export
vapour_create_options <- function(driver = "GTiff") {

  
    if (driver[1] == "GTiff")  {
      default_options = c(
  "SPARSE_OK=YES", 
  "TILED=YES", "BLOCKXSIZE=512", "BLOCKYSIZE=512", 
  "COMPRESS=DEFLATE", "BIGTIFF=IF_SAFER") 
    } else if (driver[1] == "COG") {
      default_options <- c(
        "SPARSE_OK=YES", 
         "BLOCKSIZE=512", 
        "COMPRESS=DEFLATE", "BIGTIFF=IF_SAFER") 
    } else {
      default_options <- character(0)
    }
  default_options
}


#' Create raster file
#'
#' This is in an incomplete interface to raster writing, for exploring. 
#' 
#' If GeoTIFF is used (`driver = "GTiff"`, recommended) then the output is tiled 512x512, and has DEFLATE compression, and
#' is sparse when created (no values are initiated, so the file is tiny). 
#' 
#' Note that there is no restriction on where you can read or write from, the responsibility is yours. There is no auto driver detection
#' done for the file format, it's up to you to set the file extension _and_ the driver. 
#' 
#' File is created using CreateCopy from a VRT in memory. This is so that we can instantiate COG layer with 'driver = "COG"'. 
#' Please note that performance is best for GTiff itself, with 'SPARSE_OK=YES'. We don't yet know how to instantiate a large
#' COG with overviews. 
#' 
#' There are default creation options set for COG and GTiff drivers, see 'vapour_create_options(driver "GTiff")' for what those are. 
#' 
#' @param filename filename/path to create
#' @param driver GDAL driver to use (GTiff is default, and recommended)
#' @param extent xmin,xmax,ymin,ymax 4-element vector
#' @param dimension dimension of the output, X * Y
#' @param projection projection of the output, best to use a full WKT but any string accepted
#' @param n_bands number of bands in the output, default is 1
#' @param overwrite not TRUE by default
#' @param datatype the name of a GDAL datatype ('Float32', 'Int64', etc)
#' @param options character vector of creation of options for the driver in use `c('COMPRESS=DEFLATE')` note how these are constructed (no '-co' element)
#'
#' @return the file path that was created
#' @export
#'
#' @examples
#' tfile <- tempfile(fileext = ".tif")
#' if (!file.exists(tfile)) {
#'  vapour_create(tfile, extent = c(-1, 1, -1, 1) * 1e6, 
#'                      dimension = c(128, 128), 
#'                      projection = "+proj=laea")
#'  file.remove(tfile)
#' }
vapour_create <- function(filename, driver = "GTiff", extent = c(-180, 180, -90, 90), 
                          dimension = c(2048, 1024), projection = "EPSG:4326", n_bands = 1L, overwrite = FALSE, 
                          datatype = "Float32",
                          options = vapour_create_options(driver)) {

  if (!overwrite && file.exists(filename)) stop("'filename' exists")
 
  driver <- driver[1L]
  if (length(driver) < 1 || !nzchar(driver) || is.na(driver)) {
    stop("driver name is not valid")
  }
  stopifnot(is.numeric(extent))
  stopifnot(length(extent) == 4L)
  df <- diff(extent)[c(1L, 3L)]
  if(!df[1] > 0) stop("extent is not valid, must be c(xmin, xmax, ymin, ymax) :  xmax !> xmin")
  if(!df[2] > 0) stop("extent is not valid, must be c(xmin, xmax, ymin, ymax) :  ymax !> ymin")
  
  if (is.null(options)) {
    options <- vapour_create_options(driver)
  } 
  if (!is.character(options)) options <- character()
  if (length(options) < 1) options <- character()
  if (!nzchar(options[1])) options <- character()
  if (length(options) == 0 || is.na(options[1])) options <- list()
  if (length(options) > 0) options <- strsplit(options, "=")
  ## options must a split list of name/values pairs
  vapour_create_cpp(filename, driver, as.numeric(extent), as.integer(dimension), projection, as.integer(n_bands), datatype, options)
}


vapour_create_copy <- function(dsource, filename, overwrite = FALSE, driver = "GTiff") {
  ##tf <- tempfile(fileext = ".tif")
  # f <- "inst/extdata/sst.tif"
  
  ## FIXME: we need to remove the SourceFilename element, else the data gets copied
  #vapour:::vapour_create_copy_cpp(gsub(f, "", vrt), tf, driver = "GTiff")
  ## tif has empty size
  #file.info(tf)$size
  #[1] 824

  ## but not (FIXME) cleared metadata, I think this is where we need to clear the PAM  
  # terra::rast(tf)
  # class       : SpatRaster 
  # dimensions  : 286, 143, 1  (nrow, ncol, nlyr)
  # resolution  : 0.07, 0.07000389  (x, y)
  # extent      : 140, 150.01, -60.01833, -39.99722  (xmin, xmax, ymin, ymax)
  # coord. ref. : lon/lat WGS 84 (EPSG:4326) 
  # source      : filefaae639737aad.tif 
  # name        : filefaae639737aad 
  # min value   :            271.35 
  # max value   :           289.859 
  # 
  if (!overwrite && file.exists(filename)) stop("'filename' exists")
  .check_dsn_single(dsource)
  
  ## 1) convert to VRT
  ## vrt <- vapour_vrt(dsource)
  ## 2) clear SourceFilename
  ## vrt <- vapour_create_copy_cpp(gsub(f, "", vrt), tf, driver = "GTiff")
  ##vapour_create_copy_cpp(vrt, tf, driver = driver)
  stop("not implemented")
}



#' Read or write raster block
#'
#' Read a 'block' from raster.
#'
#' @param dsource file name to read from, or write to
#' @param offset position x,y to start writing (0-based, y-top)
#' @param dimension window size to read from, or write to
#' @param band_output_type numeric type of band to apply (else the native type if '') can be one of 'Byte', 'Int32', or 'Float64'
#' @param band which band to read (1-based)
#' @param unscale default is `TRUE` so native values will be converted by offset and scale to floating point
#' @param nara if 'TRUE' return in nativeRaster format
#'
#' @return a list with a vector of data from the band read
#' @export
#'
#' @examples
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' v <- vapour_read_raster_block(f, c(0L, 0L), dimension = c(2L, 3L), band = 1L)
vapour_read_raster_block <- function(dsource, offset, dimension, band = 1L, band_output_type = "", unscale = TRUE, nara = FALSE) {
  dsource <- .check_dsn_single(dsource)
  if (anyNA(band) || length(band) < 1L) stop("missing band value")
  if (file.exists(dsource)) {
    dsource <- normalizePath(dsource)
  }
  vapour_read_raster_block_cpp(dsource, as.integer(rep(offset, length.out = 2L)),
                               as.integer(rep(dimension, length.out = 2L)), band = as.integer(band[1L]),
                               band_output_type = band_output_type, 
                               unscale = unscale, nara = nara)
}
#' Write data to a block *in an existing file*.
#'
#' Be careful! The write function doesn't create a file, you have to use an existing one.
#' Don't write to a file you don't want to update by mistake.
#' @export
#' @param data data vector, length should match  `prod(dimension)` or length 1 allowed
#' @param overwrite set to FALSE as a safety valve to not overwrite an existing file
#' @param dsource data source name
#' @param offset offset to start
#' @param dimension dimension to write
#' @param band which band to write to (1-based)
#'
#' @return a logical value indicating success (or failure) of the write
#' @examples
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' v <- vapour_read_raster_block(f, c(0L, 0L), dimension = c(2L, 3L), band = 1L)
#' file.copy(f, tf <- tempfile(fileext = ".tif"))
#' try(vapour_write_raster_block(tf, data = v[[1]], offset = c(0L, 0L), 
#'                dimension = c(2L, 3L), band = 1L))
#' if (file.exists(tf)) file.remove(tf)
vapour_write_raster_block <- function(dsource, data, offset, dimension, band = 1L, overwrite = FALSE) {
 ## if (!file.exists(dsource)) stop("file dsource must exist")
  if (!overwrite) stop(sprintf("set 'overwrite' to TRUE if you really mean to write to file %s", dsource))
  if (anyNA(band) || length(band) < 1L) stop("missing band value")
  band <- as.integer(band[1L])
  offset <- as.integer(rep(offset, length.out = 2L))
  dimension <- as.integer(rep(dimension, length.out = 2L))
  if (length(data) < 1 || length(prod(dimension)) < 1 || length(data) != prod(dimension)) {
    ## allow single value to write to entire block
    if (length(data) == 1L && !is.na(data)) {
      data <- rep(data, length.out = prod(dimension))
    } else {
      if (length(data) == 1L && anyNA(data)) stop("data is NA singleton but prod(dimension) > 1, please explicity input NA for every element not just a single value")
      stop("mismatched data and dimension")
    }
  }
  vapour_write_raster_block_cpp(dsource, data,
                                offset,
                                dimension,
                                band = band)
}

