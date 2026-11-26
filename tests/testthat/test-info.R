
test_that("layer info reports the EPSG code and the file list", {
  f <- system.file("extdata", "sst_c.fgb", package = "vapour", mustWork = TRUE)
  info <- vapour_layer_info(f)
  expect_identical(info$projection$EPSG, 3031L)
  expect_identical(basename(info$filelist), "sst_c.fgb")
})

test_that("a raster with no geotransform reports the default one", {
  vrt <- '<VRTDataset rasterXSize="5" rasterYSize="4"><VRTRasterBand dataType="Byte" band="1"/></VRTDataset>'
  info <- vapour_raster_info(vrt)
  expect_equal(info$geotransform, c(0, 1, 0, 0, 0, 1))
  expect_equal(info$extent, c(0, 5, 0, 4))
})

test_that("raster info reports image structure metadata", {
  f <- system.file("extdata", "sst.tif", package = "vapour", mustWork = TRUE)
  expect_true("INTERLEAVE" %in% names(vapour_raster_info(f)$image_structure))
})

test_that("an invalid extent filter warns once", {
  f <- system.file("extdata", "sst_c.fgb", package = "vapour", mustWork = TRUE)
  msgs <- character()
  g <- withCallingHandlers(
    vapour_read_geometry(f, sql = "SELECT * FROM sst_c", extent = c(1, 0, 0, 1)),
    warning = function(w) {
      msgs <<- c(msgs, conditionMessage(w))
      invokeRestart("muffleWarning")
    })
  expect_identical(msgs, "extent filter invalid (xmax <= xmin), ignoring")
  expect_length(g, 7L)
})
