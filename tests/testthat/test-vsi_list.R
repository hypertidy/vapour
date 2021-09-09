test_that("vsi list works", {
  pointzipfile <- system.file("extdata/vsi/point_shp.zip", package = "vapour", mustWork = TRUE)
  
  expect_equal(vapour_vsi_list(sprintf("/vsizip/%s", pointzipfile)), paste("point", c("dbf", "shx", "shp"), sep = "."))
})
