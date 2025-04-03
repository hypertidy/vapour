library(vapour)
## the extent of this one is a bit off, so we correct it
srcfile <- "/rdsi/PUBLIC/raad/data/www.ngdc.noaa.gov/mgg/global/relief/ETOPO1/data/ice_surface/grid_registered/netcdf/ETOPO1_Ice_g_gdal.grd"
src <- vapour_vrt(srcfile, extent = c(-180, 180, -90, 90))


## it is otherwise ok
info <- vapour_raster_info(src)
info[c("extent", "dimXY", "projection")]

## create a raster, here assume we match our input raster (but it could be different)
ex <- info$extent
prj <- info$projection
dm <- info$dimXYQA
## now we create a file, with 1 Float64 band (hardcoded for now)
rfile <- vapour:::vapour_create(tempfile(fileext = ".tif"), extent = ex,
                                dimension = dm, projection = prj, n_bands = 1L, driver = "GTiff")

#remotes::install_github("hypertidy/grout")
outinfo <- vapour::vapour_raster_info(rfile)  ## for now the tiling is also hardcoded to 512x512

## now get the tiling index, gives us everthing we need to read a tile for our target, either by the warper or by RasterIO
## the native tiling is tiny so we up it
info$tilesXY
BLOCKSIZE <- info$tilesXY * 1 ## 2, 4, 8, 16
index <- grout::tile_index( grout:::grout(outinfo, BLOCKSIZE))
index

library(dplyr)
system.time({
  for (i in seq_len(dim(index)[1L])) {
    tile <- as.matrix(index[i, ])
    
    vals <- vapour_warp_raster_dbl(src, 
                                   extent = tile[, c("xmin", "xmax", "ymin", "ymax"), drop = TRUE], 
                                   dimension = tile[, c("ncol", "nrow"), drop = TRUE], projection = outinfo$projection)
    
    vals <- vals + runif(1, -8000, 8000)  ## so we know we achieved something
    ## here we might read from multiple rasters or bands and coalesce, do calcs etc
    vapour:::vapour_write_raster_block(rfile, vals, tile[, c("offset_x", "offset_y"), drop = TRUE], 
                                       tile[, c("ncol", "nrow"), drop = TRUE] , 
                                       overwrite = TRUE)
  }
})

library(terra)
plot(rast(rfile), col = grey.colors(256))

