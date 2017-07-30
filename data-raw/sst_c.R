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
