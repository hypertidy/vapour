library(raster)
v <- setExtent(raster(volcano), extent(0, ncol(volcano), 0, nrow(volcano)))
writeRaster(v, "inst/extdata/volcano.tif")
