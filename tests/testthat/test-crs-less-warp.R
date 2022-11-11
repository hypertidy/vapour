test_that("with with no source crs works", {
  skip("skip fixme")
  f <- system.file("extdata/volcano.png", package = "vapour", mustWork = TRUE)
  info <- vapour_raster_info(f)
  #vapour_warp_raster(f, dimension = info$dimension, 
  #                   extent = c(0, 1, 0, 1))
  
 # sds <- "NETCDF:/rdsi/PUBLIC/raad/data/www.ncei.noaa.gov/data/sea-surface-temperature-optimum-interpolation/v2.1/access/avhrr/198109/oisst-avhrr-v02r01.19810901.nc:sst"
  ## expect no crashy crashy
#  expect_silent(vapour_warp_raster(sds, extent = c(0, 1, 0, 1), dimension = c(4, 4)))
 # expect_silent(vapour_warp_raster(sds, extent = c(0, 1, 0, 1), dimension = c(4, 4), source_projection = "+proj=laea"))
 ## expect_warning(vapour_warp_raster(sds, extent = c(0, 1, 0, 1), dimension = c(4, 4), projection = "OGC:CRS84"))
  
  

  ## this file is written upside down so the geoloc screws up (or something, not quite there yet)  
  badnc <- system.file("extdata/gdal", "bad_netcdf_geoloc_arrays.nc", package = "vapour")
  sds <- vapour_sds_names(badnc)
  
  lcc <-  "+proj=lcc +lat_0=-30 +lon_0=134.33 +lat_1=-50 +lat_2=-10 +R=6378137"
  ex <- c(-3077504,  3968504, -2763621,  3472383)
  vrt <- vapour_vrt(sds[3], geolocation = sds[2:1], bands = 1)
  info <- vapour_raster_info(vrt)
  expect_warning(im <- vapour_warp_raster_dbl(vrt, extent = ex, dimension = info$dimension, projection = lcc
                               , transformation_options = c("SRC_METHOD=NO_GEOTRANSFORM")))
  
  ex <- c(-180, 180, -90, 90)
  dm <- c(512, 1024)
  vrt <- vapour_vrt(sds[3], geolocation = sds[2:1], bands = 1)
  vapour::vapour_set_config("GDAL_NETCDF_BOTTOMUP", 'NO')
  expect_warning(  im <- vapour_warp_raster_dbl(vrt, extent = ex, dimension = dm, projection = "OGC:CRS84"
                              , transformation_options = c("SRC_METHOD=GEOLOC_ARRAY")))

})
