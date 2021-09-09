#' Read or write raster block
#'
#' Read a 'block' from raster.
#'
#' @param dsource file name to read from, or write to
#' @param offset position x,y to start writing (0-based, y-top)
#' @param dimension window size to read from, or write to
#' @param band_output_type numeric type of band to apply (else the native type if '') can be one of 'Byte', 'Int32', or 'Float64'
#' @param band which band to read (1-based)
#'
#' @return for vapour_read_raster_block, a list with vector of data
#' @export
#'
#' @examples
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' v <- vapour_read_raster_block(f, c(0L, 0L), dimension = c(2L, 3L), band = 1L)
vapour_read_raster_block <- function(dsource, offset, dimension, band = 1L, band_output_type = "") {
  if (anyNA(band) || length(band) < 1L) stop("missing band value")
  if (file.exists(dsource)) {
    dsource <- normalizePath(dsource)
  }
  vapour_read_raster_block_cpp(dsource, as.integer(rep(offset, length.out = 2L)),
                               as.integer(rep(dimension, length.out = 2L)), band = as.integer(band[1L]),
                               band_output_type = band_output_type)
}
#'  write data to a block *in an existing file*.
#'
#' Be careful! The write function doesn't create a file, you have to use an existing one.
#' Don't write to a file you don't want to update by mistake.
#' @noRd
#' @return for vapour_write_raster_block a logical indicating success or failure to write
#' @param data data vector, length should match  `prod(dimension)` or length 1 allowed
#' @param overwrite set to FALSE as a safety valve to not overwrite an existing file
#' @param band which band to write to (1-based)
#' @examples
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' v <- vapour_read_raster_block(f, c(0L, 0L), dimension = c(2L, 3L), band = 1L)
#' file.copy(f, tf <- tempfile(fileext = ".tif"))
#' vapour_write_raster_block(tf, data = v[[1]], offset = c(0L, 0L), dimension = c(2L, 3L), band = 1L)
#' file.remove(tf)
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

