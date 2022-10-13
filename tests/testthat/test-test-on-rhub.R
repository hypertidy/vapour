test_that("multiplication works", {
  expect_type( vapour::vapour_srs_wkt("+proj=longlat +datum=WGS84"), "character")
})
