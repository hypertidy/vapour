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
  ## TODO: estimate size from all files, not just the dsn name
  ## GDAL can tell us all the files involved
  if (file.exists(x$source)) size <- file.info(x$source)$size/1e6
  cat(sprintf("   File: %s (%sMb)", !is.na(size), size))
}

sr <- ssraster(f)
sr
to_extent <- function(x) {
  xmin <- x$info$geotransform[1]
  xmax <- xmin + x$info$dimXY[1] * x$info$geotransform[2]
  ymax <- x$info$geotransform[4]
  ymin <- ymax + x$info$dimXY[2] * x$info$geotransform[6]
  raster::extent(c(xmin = xmin, xmax = xmax, ymin = ymin, ymax = ymax))
}
to_raster <- function(x, dim = NULL) {
  ## assume ssraster
  if (is.null(dim)) dim <- x$info$dimXY
  raster::raster(to_extent(x), nrows = dim[2], ncols = dim[1])
}

# ## a really massive file source, 72Gb uncompressed
# f <- "elvis.ga.gov.au/elevation/1sec-srtm/a05f7893-0050-7506-e044-00144fdd4fa6"
# x <- ssraster(f)
# #Super simple raster
#
# #source: /rdsi/PUBLIC/raad/data/elvis.ga.gov.au/elevation/1sec-srtm/a05f7893-0050-7506-e044-00144fdd4fa6
# #dim: 147600x122400
# #topleft: 112.999861, -10.000139
# #scale: 0.000278, -0.000278
#
# To plot actually takes a minute or so but it works!
# plot(x)
plot.ssraster <- function(x, resample = "NearestNeighbour") {
  ## TODO: this needs to account for the "usr" bounds, the current
  ## bounds that will be plotted to
  plotdim <- dev.size("px")
  ## TODO raster from ssraster, override with dim
  r <- to_raster(x, dim = plotdim)
  ## TODO pull window spec from info/plotdim, allow choice of resampling
  vals <- raster_io(f, window = c(0, 0, x$info$dimXY[1], x$info$dimXY[2], plotdim[1], plotdim[2]),
                    resample = resample)
  ## TODO clamp values to info$minmax - set NA
  vals[vals < x$info$minmax[1] | vals > x$info$minmax[2]] <- NA
  plot(setValues(r, vals))
}
