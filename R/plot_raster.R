#' Plot GDAL matrix.
#'
#' Plot a matrix directly read via GDAL to match the current device size.
#'
#' The data is read in at the resolution required by the current device, assuming it's a single
#' panel.  TODO: read off the space to plot into from the device, and
#' read only that window from the source at the right resolution.
#' @param filename source
#' @param add add to the plot
#' @export
#' @examples
#' ## this file takes 5s to read in total from NetCDF, 24 seconds via GDAL
#' filename <- "/rdsi/PRIVATE/raad/data_local/www.bodc.ac.uk/gebco/GRIDONE_2D.nc"
#' #filename <- raadtools::topofile()
#' #filename <- "D:/data/topography/etopo2/subset.tif"
#' library(vapour)
#' ## read in to match the device and it's quicker
#' plot_raster(filename)
#' #zoom(wrld_simpl, new = FALSE)
#' #plot_raster(filename, ext = spex::spex(), add = T, col = rainbow(100))
#' #maps::map(add = TRUE)
#' ## library(sf); plot(st_transform(sst_c, 4326), add = T)
plot_raster <- function(filename, add = FALSE, ext = NULL, cols = NULL) {
  ri <- raster_info(filename)
  xlim <- ri$geotransform[1]
  xlim <- c(xlim[1], xlim[1] + ri$dimXY[1] * ri$geotransform[2])
  ylim <- ri$geotransform[4]
  ylim <- c(ylim[1] + ri$dimXY[2] * ri$geotransform[6], ylim)
  flip <- function(x) x[,ncol(x):1]
  brks <- seq(ri$minmax[1], ri$minmax[2], length = 100)
  cols <- if (is.null(cols))  viridis::viridis(99) else  colorRampPalette(cols)(length(brks)-1)

  dv <- dev.size("px")
  rlocal <- raster::raster(extent(xlim, ylim), nrow= ri$dimXY[2], ncol = ri$dimXY[1])
  window <- c(0, 0, ri$dimXY)
  if (!is.null(ext)) {
    ## this index is Y-up
    index  <- spex:::as_double(tabularaster::index_extent(rlocal, ext))
    indexYdown <- c(index[1:2], nrow(rlocal) - index[4:3])
    window <- c(indexYdown[1], indexYdown[3],
                indexYdown[2] - indexYdown[1], indexYdown[4] - indexYdown[3])
    rlocal <- crop(rlocal, ext, snap = "out")
  }

  z <- flip(matrix(raster_io(filename, c(window, dv[1], dv[2])), dv[1]))

  dim(rlocal) <- rev(dim(z))
  l <- list(x = raster::xFromCol(rlocal),
            y = rev(raster::yFromRow(rlocal)),
            z = z)
  if(!add && dev.cur() == 1) {
    print(add)
    add <- TRUE
    par(mar = rep(0, 4))
    plot(xlim, ylim, type = "n", xaxs = "i", yaxs = "i")
  }

  image(l, useRaster = TRUE,
        col = cols, breaks = brks, add = add)

}
