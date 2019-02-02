context("test-raster-info")
skip_on_appveyor()
skip_on_travis()
skip_on_cran()
test_that("raster info works", {
  f <- system.file("extdata/gdal/complex.h5", package = "vapour", mustWork = TRUE)
  expect_error(vapour_sds_names(), 'argument "x" is missing, with no default')
  expect_error(vapour_sds_names(""), 'cannot open dataset')


  expect_message(sds_boilerplate_checks(f), "subdataset \\(variable\\) used is '//f16'")
  expect_error(sds_boilerplate_checks(f, 0), "sds must be 1 at minimum")
  expect_match(sds_boilerplate_checks(f, 1),
               "^HDF5:\".*extdata/gdal/complex\\.h5\"://f16$")

  expect_silent(s1 <- vapour_sds_names(f)) %>% expect_named(c("datasource", "subdataset"))
  expect_length(unlist(s1), 6L)

})
