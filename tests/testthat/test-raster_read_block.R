f <- system.file("extdata", "sst.tif", package = "vapour", mustWork = TRUE)
test_that("raster read/write block works", {
 v <- vapour_read_raster_block(f, c(0L, 0L), dimension = c(2L, 3L), band = 1L)
 expect_length(v, 1L)
 expect_length(v[[1L]], 6L)
 
 
 v <- vapour_read_raster_block(f, c(10L, 20L), dimension = c(2L, 3L), band = 1L)
 expect_length(v, 1L)
 expect_length(v[[1L]], 6L)
 

 file.copy(f, tf <- tempfile(fileext = ".tif"))
 	
 expect_error(vapour_write_raster_block(tf, data = 1:6, offset = c(0L, 0L), dimension = c(2L, 3L), band = 1L), "overwrite")
 expect_true(vapour_write_raster_block(tf, data = 1, offset = c(0L, 0L), dimension = c(2L, 3L), band = 1L, overwrite = TRUE))
 expect_error(vapour_write_raster_block(tf, data = 1:2, offset = c(0L, 0L), dimension = c(2L, 3L), band = 1L, overwrite = TRUE))
 
 expect_error(vapour_write_raster_block(tf, data = NA, offset = c(0L, 0L), dimension = c(2L, 3L), band = 1L, overwrite = TRUE))
 expect_true(vapour_write_raster_block(tf, data = rep(NA, 6L), offset = c(0L, 0L), dimension = c(2L, 3L), band = 1L, overwrite = TRUE))
 expect_true(vapour_write_raster_block(tf, data = 1:6, offset = c(0L, 0L), dimension = c(2L, 3L), band = 1L, overwrite = TRUE))
 
 expect_equivalent(vapour_read_raster_block(tf, c(0L, 0L), dimension = c(2L, 3L), band = 1L)[[1]], 1:6)
 file.remove(tf)
})

