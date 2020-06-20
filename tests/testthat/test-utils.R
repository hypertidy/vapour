context("test-utils")
file <- "list_locality_postcode_meander_valley.tab"
mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
test_that("utilities work", {

  drivers <- as.data.frame(vapour_all_drivers(), stringsAsFactors = FALSE)

  drivers %>% expect_s3_class("data.frame") %>% expect_named(c("driver", "name", "vector", "raster", "create", "copy", "virtual"))
  types <- unlist(lapply(drivers, typeof), recursive = FALSE)
  expect_equal(types, c(driver = "character", name = "character", vector = "logical",
                        raster = "logical", create = "logical", copy = "logical", virtual = "logical"
  ))
  expect_match(vapour_gdal_version(), "^GDAL .*, released .*/.*/")
  expect_error(vapour_driver(0))
  expect_error(vapour_driver(""))
  expect_error(vapour_driver("1"), "Open failed")
  expect_match(vapour_driver(mvfile), "MapInfo File")

})
