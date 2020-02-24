library(raster)
v <- setExtent(raster(volcano), extent(0, ncol(volcano), 0, nrow(volcano)))
writeRaster(v, "inst/extdata/volcano.tif")

file.copy("inst/extdata/volcano.tif", "inst/extdata/volcano_overview.tif")
system("gdaladdo inst/extdata/volcano_overview.tif -minsize 4 2 4 8 16")
