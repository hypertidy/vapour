f <- system.file("extdata", "sst.tif", package = "vapour")

#raster::raster(f)
# class      : RasterLayer
# dimensions : 286, 143, 40898  (nrow, ncol, ncell)
# resolution : 0.07, 0.07000389  (x, y)
# extent     : 140, 150.01, -60.01833, -39.99722  (xmin, xmax, ymin, ymax)
# crs        : +proj=longlat +datum=WGS84 +no_defs
# source     : sst.tif
# names      : sst
# values     : 271.35, 289.859  (min, max)


## cases
# 1. source is valid, proper geotransform and crs
# 2. source has no crs
# 3. source has no crs and no proper geotransform (no proper geotransform occurs when xres/yres == 0)
## don't think we can have valid crs and no proper transform ...

# 1. but we override geotransform
# 1. but we override crs
# 1. but we override crs and geotransform

# 2. we supply crs
# 2. we supply crs and geotransform

## auto failures
# - no valid source crs, we can't do anything   DONE, detect empty string, close up and stop
# - no valid target crs, doesn't fail unless "", we just get zero values and a cryptic ERROR 1: missing [
# - no valid target geotransform, NA values give so seems ok
## ERROR 1: Too many points (529 out of 529) failed to transform, unable to compute output bounds.
## Warning 1: Unable to compute source region for output window 0,0,320,320, skipping.

# - no valid target dimension, DONE
# - need to see what raster does with floating point dims vs. gdal

# source meta is fine (or we augment it)
# - no wkt provided (doesn't need to be specified, input is assumed)
# -  no dim provided (I think it has to be, but native would be good)
# - no extent provided


test_that("warper no transformation works", {
 expect_that(vapour_warp_raster(f, extent = c(145, 146, -50, -48), dimension = c(2, 2)), is_a("list"))
})

test_that("warper no transformation and no dimension works", {
  expect_that(vapour_warp_raster(f, extent = c(145, 146, -50, -48)), is_a("list"))
  expect_error(vapour_warp_raster(f, extent = c(145, 146, -50, -48), wkt = "+proj=laea"), "'dimension' must be numeric")

})

test_that("warper bad transformation fails", {
  expect_error(vapour_warp_raster(f, extent = c(145, 146, -50, -48), dimension = c(2, 2), wkt = "aabbcc"), "cannot initialize target projection")

  ## this should get checked by GDAL itself, else crashy
  expect_error(vapour_warp_raster(f, extent = c(145, 146, -50, -48), dimension = c(2, 2), wkt = "PROJala[kakakaka]"), "cannot initialize target projection")
  expect_error(vapour_warp_raster(f, extent = c(145, 146, -50, -48), dimension = c(2, 2), source_wkt =   "PROJala[kakakaka]"), "cannot initialize source projection")

})

test_that("warper band repetition works", {
  expect_length(vapour_warp_raster(f, bands = c(1, 1), extent = c(145, 146, -50, -48), dimension = c(2, 2)), 2L)
})


test_that("warper gives the right number of values", {
  expect_length(vapour_warp_raster(f, bands = 1, extent = c(145, 146, -50, -48), dimension = c(2L, 2L))[[1L]], 4L)
  expect_length(vapour_warp_raster(f, bands = 1, extent = c(145, 146, -50, -48), dimension = c(2L, 12L))[[1L]], 24L)
  expect_length(vapour_warp_raster(f, bands = 1, extent = c(145, 146, -50, -48), dimension = c(1L, 1200L))[[1L]], 1200L)
  expect_message(vapour_warp_raster(f, bands = 1, extent = c(145, 146, 90, -48), dimension = c(300L, 301L))[[1L]], "expert use")

})

test_that("giving source extent and projection works", {
  expect_named(
    vapour_warp_raster(f, extent = c(0, 67, 0, 81), source_extent = c(0, 67, 0, 81)/2,
                     wkt = "+proj=laea", source_wkt = "+proj=laea", dimension = c(10, 10)), "Band1"
  )
})
test_that("robust to bad inputs", {
  expect_error(vapour_warp_raster(c(f, "afile"), extent = c(0, 1, 0, 1),
                                  wkt = "+proj=laea", source_wkt = "+proj=longlat", dimension = c(10, 10))
  )

  expect_named(vapour_warp_raster(c(f, f), extent = c(0, 1, 0, 1),
                     wkt = "+proj=laea", source_wkt = "+proj=longlat", dimension = c(10, 10)), "Band1")

  expect_named(vapour_warp_raster(f, extent = c(0, 1, 0, 1),
                                  wkt = "+proj=laea",  dimension = c(10, 10)))

  expect_error(vapour_warp_raster(f, extent = c(0, 1, 0, 1), band = 2,
                                  wkt = "+proj=laea", source_wkt = "+proj=longlat", dimension = c(10, 10)),
               "band requested exceeds"
  )

})
