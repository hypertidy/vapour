#f <- tail(raadtools::sstfiles()$fullname, 1)
f <- "/rdsi/PUBLIC/raad/data/eclipse.ncdc.noaa.gov/pub/OI-daily-v2/NetCDF/2017/AVHRR/avhrr-only-v2.20170910_preliminary.nc"
f <- 'NETCDF:"/rdsi/PUBLIC/raad/data/eclipse.ncdc.noaa.gov/pub/OI-daily-v2/NetCDF/2017/AVHRR/avhrr-only-v2.20170910_preliminary.nc":sst'
f <-  "/rdsi/PUBLIC/raad/data/www.ngdc.noaa.gov/mgg/global/relief/ETOPO2/ETOPO2v2-2006/ETOPO2v2c/netCDF/ETOPO2v2c_f4.nc"

## let's assume we did this without raster
library(raster)
r <- raster(f)

tiles <- spex::polygonize(raster(raster::extent(r), crs = projection(r),
                                 nrow = nrow(r) / 240, ncol = ncol(r)/240))

populate_raster <- function(n, tile = NULL, data = NULL) {
  out <- replicate(n, tibble::as_tibble(list(data = numeric())))
  if (!is.null(tile)) out[[tile]] <- tibble::tibble(data = data)
  out
}
tiles$raster_data <- populate_raster(nrow(tiles))
library(sf)
library(spex)
extent <- function(x, ...) UseMethod("extent")
extent.sfc_POLYGON <- function(x, ...) raster::extent(attr(x, "bbox")[c("xmin", "xmax", "ymin", "ymax")])

gdal_source <- function(obj, geom) {
  ex <- tabularaster:::index_extent(obj, extent(geom))
  offset_source_x <- c(xmin(ex), xmax(ex))
  offset_source_y <- c(nrow(obj) - c(ymax(ex), ymin(ex)))
  c(offset_source_x, offset_source_y)
}

itile <- 1
gs <- gdal_source(r, st_geometry(tiles[itile, ]))
library(vapour)
tiles$raster_data <- populate_raster(nrow(tiles), itile, raster_io(f, c(gs, c(diff(gs[1:2])+1, diff(gs[3:4])+1))))

par(mfrow = c(2, 1))
plot(crop(r, tiles[itile, ]), asp = "")
plot(tiles$raster_data[[itile]]$data)
