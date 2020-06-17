context("test-raster-info")

## too flaky
#skip_on_appveyor()
#skip_on_travis()
#skip_on_cran()

skip("don't do it")
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
 ##expect_error(sds_boilerplate_checks(fsds, 0))  ## does not error on windows atm
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

  f1 <- system.file("extdata/volcano_overview.tif", package = "vapour", mustWork = TRUE)
  f2 <- system.file("extdata/volcano.tif", package = "vapour", mustWork = TRUE)

  expect_true(vapour_raster_info(f2)$overviews == 0L)
  expect_equal(vapour_raster_info(f1)$overviews, ## Overviews: 31x44, 16x22, 8x11, 4x6
               c(4L, 31L, 44L, 16L, 22L, 8L, 11L, 4L, 6L))
  ## these are different on Windows and Linux ... because linux sees more SDS
  ## expect_error(sds_boilerplate_checks(f, 0), "sds must be 1 at minimum")
    # expect_message(sds_boilerplate_checks(f), "subdataset \\(variable\\) used is '//f16'")
  #
  # expect_match(sds_boilerplate_checks(f, 1),
  #              "^HDF5:\".*extdata/gdal/complex\\.h5\"://f16$")
  #
  #expect_silent(s1 <- vapour_sds_names(f)) %>% expect_named(c("datasource", "subdataset"))
  #expect_length(unlist(s1), 6L)

})

