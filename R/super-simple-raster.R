## stub for a demand-paged raster for plotting

ssraster <- function(x) {
  structure(list(source = x,
                 info = raster_info(x)), class = "ssraster")
}

print.ssraster <- function(x) {
  cat("Super simple raster\n")
  cat("\n")
  cat(sprintf(" source: %s\n", x$source))
  cat(sprintf("    dim: %ix%i\n", x$info$dimXY[1], x$info$dimXY[2]))
  cat(sprintf("topleft: %f, %f\n", x$info$geotransform[1], x$info$geotransform[4]))
  cat(sprintf("  scale: %f, %f\n", x$info$geotransform[2], x$info$geotransform[6]))
  size <- NA_character_
  if (file.exists(x$source)) size <- file.info(x$source)$size/1e6
  cat(sprintf("   File: %s (%sMb)", !is.na(size), size))
}

sr <- ssraster(f)
sr

plot.ssraster <- function(x, resample = "NearestNeighbour") {
  plotdim <- dev.size("px")
  ## TODO raster from ssraster, override with dim
  r <- to_raster(x$info, dim = plotdim)
  ## TODO pull window spec from info/plotdim, allow choice of resampling
  vals <- raster_io(f, window = c(0, 0, 70, 140, 143, 286),
                    resample = resample)
  ## TODO clamp values to info$minmax - set NA
  r <- setValues(r, vals)
}
