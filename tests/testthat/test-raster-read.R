f <- system.file("extdata", "sst.tif", package = "vapour", mustWork = TRUE)
test_that("raster band type works", {
  
  expect_type(vapour_read_raster_chr(f, window = c(10, 20, 5, 3)), "character") %>% expect_length(15L)
  expect_type(vapour_read_raster_hex(f, window = c(10, 20, 5, 5)), "character") %>% expect_length(25L)
  
  expect_type(vapour_read_raster_raw(f, window = c(10, 20, 5, 3)), "raw") %>% expect_length(15L)
  expect_type(vapour_read_raster_raw(f, window = c(10, 20, 5, 5)), "raw") %>% expect_length(25L)
  
  expect_type(vapour_read_raster_int(f, window = c(10, 20, 5, 3)), "integer") %>% expect_length(15L)
  expect_type(vapour_read_raster_int(f, window = c(10, 20, 5, 5)), "integer") %>% expect_length(25L)

  expect_type(vapour_read_raster_dbl(f, window = c(10, 20, 5, 3)), "double") %>% expect_length(15L)
  expect_type(vapour_read_raster_dbl(f, window = c(10, 20, 5, 5)), "double") %>% expect_length(25L)
  

  expect_type(vapour_read_raster(f, window = c(10, 20, 5, 3), band_output_type = "Float32")[[1]], "double") %>% expect_length(15L)
  expect_type(vapour_read_raster(f, window = c(10, 20, 5, 5), band_output_type = "UInt16")[[1]], "integer") %>% expect_length(25L)
  
  
  expect_type(vapour_warp_raster(f, extent = c(145, 145.1, -45, -44.9), dimension = c(5, 3), band_output_type = "Byte", bands = 1)[[1]], "raw")
  
  expect_type(vapour_warp_raster_chr(f, extent = c(145, 145.1, -45, -44.9), dimension = c(5, 3)), "character") %>% expect_length(15L)
  expect_type(vapour_warp_raster_hex(f, extent = c(145, 145.1, -45, -44.9), dimension = c(5, 5)), "character") %>% expect_length(25L)
  
  expect_type(vapour_warp_raster_raw(f, extent = c(145, 145.1, -45, -44.9), dimension = c(5, 3)), "raw") %>% expect_length(15L)
  expect_type(vapour_warp_raster_raw(f, extent = c(145, 145.1, -45, -44.9), dimension = c(5, 5)), "raw") %>% expect_length(25L)
  
  expect_type(vapour_warp_raster_int(f, extent = c(145, 145.1, -45, -44.9), dimension = c(5, 3)), "integer") %>% expect_length(15L)
  expect_type(vapour_warp_raster_int(f, extent = c(145, 145.1, -45, -44.9), dimension = c(5, 5)), "integer") %>% expect_length(25L)
  
  expect_type(vapour_warp_raster_dbl(f, extent = c(145, 145.1, -45, -44.9), dimension = c(5, 3)), "double") %>% expect_length(15L)
  expect_type(vapour_warp_raster_dbl(f, extent = c(145, 145.1, -45, -44.9), dimension = c(5, 5)), "double") %>% expect_length(25L)
  
  
  expect_type(vapour_warp_raster(f, extent = c(145, 145.1, -45, -44.9), dimension = c(5, 3), band_output_type = "Float32")[[1]], "double") %>% expect_length(15L)
  expect_type(vapour_warp_raster(f, extent = c(145, 145.1, -45, -44.9), dimension = c(5, 5), band_output_type = "UInt16")[[1]], "integer") %>% expect_length(25L)
  
  
})
