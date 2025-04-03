f <- system.file("extdata", "sst_c.fgb", package = "vapour", mustWork = TRUE)
r <- system.file("extdata", "sst.tif", package = "vapour", mustWork = TRUE)
test_that("multiplication works", {
  expect_equal(vapour_driver(f), "FlatGeobuf")
  expect_equal(vapour_driver(r), "GTiff")
  
})
