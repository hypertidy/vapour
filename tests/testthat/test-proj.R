context("test-proj")

test_that("crs test works", {
 expect_true(vapour_crs_is_lonlat("EPSG:4326"))
})
