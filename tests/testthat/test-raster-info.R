context("test-raster-info")

## too flaky
skip_on_appveyor()
skip_on_travis()
skip_on_cran()


test_that("sds checks work", {
  f <- system.file("extdata/gdal/geos_rad.nc", package = "vapour", mustWork = TRUE)
  f <- normalizePath(f)
  expect_silent(sds_boilerplate_checks(f))
  f1 <- system.file("extdata/sst.tif", package = "vapour", mustWork = TRUE)
  f1 <- normalizePath(f1)
  expect_silent(sds_boilerplate_checks(f1))
  fsds <- system.file("extdata/gdal/sds.nc", package = "vapour", mustWork = TRUE)
  fsds <- normalizePath(fsds)
  ## expectations not met 2019-06-12, why?
  expect_message(sds_boilerplate_checks(fsds))
  expect_error(sds_boilerplate_checks(fsds, "vv"), "sds must be specified by number, starting from 1")
  expect_error(sds_boilerplate_checks(fsds, 0))
  expect_error(sds_boilerplate_checks(fsds, 0:1))
  expect_equal(sds_boilerplate_checks(fsds, 1), sprintf("NETCDF:\"%s\":vv", fsds))
  expect_equal(sds_boilerplate_checks(fsds, 2), sprintf("NETCDF:\"%s\":vv2", fsds))
  expect_error(sds_boilerplate_checks(fsds, 3), "'sds' must not exceed number of subdatasets \\(2)")


  expect_silent(sds_boilerplate_checks(fsds, 1))

})

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
