
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

test_that("feature extents come back as a matrix", {
  f <- system.file("extdata", "sst_c.fgb", package = "vapour", mustWork = TRUE)
  m <- vapour_feature_extent(f)
  expect_true(is.matrix(m) && is.double(m))
  expect_identical(dim(m), c(7L, 4L))
  expect_identical(colnames(m), c("xmin", "xmax", "ymin", "ymax"))
  expect_equal(unname(m), do.call(rbind, vapour_read_extent(f)))
  expect_identical(dim(vapour_feature_extent(f, limit_n = 2L, skip_n = 1)), c(2L, 4L))

  ## a missing geometry is a row of NA, no features is no rows
  tf <- tempfile(fileext = ".geojson")
  writeLines(c('{"type":"FeatureCollection","features":[',
               '{"type":"Feature","properties":{"id":1},"geometry":{"type":"Point","coordinates":[2,3]}},',
               '{"type":"Feature","properties":{"id":2},"geometry":null}]}'), tf)
  mn <- vapour_feature_extent(tf)
  expect_equal(unname(mn[1L, ]), c(2, 2, 3, 3))
  expect_true(all(is.na(mn[2L, ])))
  layer <- vapour_layer_names(tf)[1L]
  none <- vapour_feature_extent(tf, sql = sprintf("SELECT * FROM \"%s\" WHERE id > 5", layer))
  expect_identical(dim(none), c(0L, 4L))
})
