context("test-raster-info")

test_that("raster info works", {
  f <- system.file("extdata/gdal/complex.h5", package = "vapour", mustWork = TRUE)
  expect_error(vapour_sds_names(), 'argument "x" is missing, with no default')
  expect_error(vapour_sds_names(""), 'cannot open dataset')

  expect_silent(s1 <- vapour_sds_names(f)) %>% expect_named(c("datasource", "subdataset"))
  expect_length(unlist(s1), 6L)


})
