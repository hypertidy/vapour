library(raadtools)

sst <- readghrsst(latest = TRUE, xylim = extent(-180, 180, -90, 0))
cl <- raster::rasterToContour(sst)
cl <- sp::spTransform(cl, "+init=epsg:3031")
cl <- sf::st_as_sf(cl)
cl$sst <- as.numeric(levels(cl$level)[cl$level]) - 273.15
sst_c <- cl
pryr::object_size(cl)
devtools::use_data(sst_c)

sf::st_write(sst_c, "inst/extdata/sst_c.gpkg", driver = "GPKG")

sst <- aggregate(crop(sst, extent(130, 150, -60, -40)), fun = median, fact = 5)
writeRaster(sst, "inst/extdata/sst.tif")


sds <- 'NETCDF:"/rdsi/PUBLIC/raad/data/podaac-ftp.jpl.nasa.gov/allData/ghrsst/data/GDS2/L4/GLOB/JPL/MUR/v4.1/2002/152/20020601090000-JPL-L4_GHRSST-SSTfnd-MUR-GLOB-v02.0-fv04.1.nc":analysed_sst'

system(sprintf("gdal_translate %s /tmp/afile.tif", sds))
