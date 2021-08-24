  read_block <- function(dsn, offset, dimension, band = 1L) {
  if (file.exists(dsn)) {
    dsn <- normalizePath(dsn)
  }
  vapour:::vapour_read_raster_block_cpp(dsn, as.integer(rep(offset, length.out = 2L)), as.integer(rep(dimension, length.out = 2L)), band = as.integer(band[1L]))
}

write_block <- function(dsn, data, offset, dimension, band = 1L) {
  vapour:::vapour_write_raster_block_cpp(dsn, data, as.integer(rep(offset, length.out = 2L)), as.integer(rep(dimension, length.out = 2L)), band = as.integer(band[1L]))
}

