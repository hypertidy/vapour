#' Plot GDAL matrix.
#'
#' Plot a matrix directly read via GDAL to match the current device size.
#'
#' The data is read in at the resolution required by the current device, assuming it's a single
#' panel.  TODO: read off the space to plot into from the device, and
#' read only that window from the source at the right resolution.
#' @param filename source
#' @param add add to the plot
#' @examples
#' ## this file takes 5s to read in total from NetCDF, 24 seconds via GDAL
#' filename <- "/rdsi/PRIVATE/raad/data_local/www.bodc.ac.uk/gebco/GRIDONE_2D.nc"
#' library(vapour)
#' ## read in to match the device it's less than a second
#' plot_raster(filename)
#' #maps::map(add = TRUE)
#' ## library(sf); plot(st_transform(sst_c, 4326), add = T)
plot_raster <- function(filename, add = FALSE) {
  ri <- raster_info(filename)
  xlim <- ri$geotransform[1] - ri$geotransform[2]/2
  xlim <- c(xlim[1], xlim[1] + ri$dimXY[1] * ri$geotransform[2])
  ylim <- ri$geotransform[4] + ri$geotransform[6]/2
  ylim <- c(ylim[1] + ri$dimXY[2] * ri$geotransform[6], ylim)
  flip <- function(x) x[,ncol(x):1]
  brks <- seq(ri$minmax[1], ri$minmax[2], length = 100)
  cols <- viridis::viridis(99)
  dv <- dev.size("px")
  z <- flip(matrix(raster_io(filename, c(0, 0, ri$dimXY[1], ri$dimXY[2], dv[1], dv[2])), dv[1]))
  l <- list(x = seq(xlim[1], xlim[2], length = nrow(z) + 1),
            y = seq(ylim[1], ylim[2], length = ncol(z) + 1),
            z = z)
  if(!add && dev.cur() == 1) {
    add <- TRUE
    par(mar = rep(0, 4))
    plot(xlim, ylim, type = "n", xaxs = "i", yaxs = "i")
  }

  image(l, useRaster = TRUE,
        col = cols, breaks = brks, add = add)

}
