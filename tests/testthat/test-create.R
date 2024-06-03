ex <- c(10, 20, -10, 20)
test_that("create works", {
  expect_silent(vapour_create(tf <- tempfile(fileext = ".tif"), dimension = c(10, 5), extent = ex)) |> expect_equal(tf)
  expect_silent(info <- vapour_raster_info(tf))  
  expect_equal(info$extent, ex)
  expect_equal(info$dimension, c(10, 5))
})
