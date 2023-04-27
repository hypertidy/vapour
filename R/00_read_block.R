#' Create raster file
#'
#' This is in an incomplete interface to raster writing, for exploring. 
#' 
#' If GeoTIFF is used (`driver = "GTiff"`, recommended) then the output is tiled 512x512, and has DEFLATE compression, and
#' is sparse when created (no values are initiated, so the file is tiny). 
#' 
#' Note that there is no restriction on where you can read or write from, the responsibility is yours. In future we will 
#' allow control of output tiling and data type etc. 
#' 
#' @param filename filename/path to create
#' @param driver GDAL driver to use (GTiff is default, and recommended)
#' @param extent xmin,xmax,ymin,ymax 4-element vector
#' @param dimension dimension of the output, X * Y
#' @param projection projection of the output, best to use a full WKT but any string accepted
#' @param n_bands number of bands in the output, default is 1
#' @param overwrite not TRUE by default
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
                          dimension = c(2048, 1024), projection = "OGC:CRS84", n_bands = 1L, overwrite = FALSE) {

  if (!overwrite && file.exists(filename)) stop("'filename' exists")
 
  vapour_create_cpp(filename, driver, extent, dimension, projection, n_bands)
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
#'
#' @return a list with a vector of data from the band read
#' @export
#'
#' @examples
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' v <- vapour_read_raster_block(f, c(0L, 0L), dimension = c(2L, 3L), band = 1L)
vapour_read_raster_block <- function(dsource, offset, dimension, band = 1L, band_output_type = "", unscale = TRUE) {
  dsource <- .check_dsn_single(dsource)
  if (anyNA(band) || length(band) < 1L) stop("missing band value")
  if (file.exists(dsource)) {
    dsource <- normalizePath(dsource)
  }
  vapour_read_raster_block_cpp(dsource, as.integer(rep(offset, length.out = 2L)),
                               as.integer(rep(dimension, length.out = 2L)), band = as.integer(band[1L]),
                               band_output_type = band_output_type, 
                               unscale = unscale)
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
  if (!file.exists(dsource)) stop("file dsource must exist")
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

