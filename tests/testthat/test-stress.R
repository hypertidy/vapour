# f <- system.file("extdata", "sst.tif", package = "vapour", mustWork = TRUE)
# skip()
# 
# test_that("terrible things don't crash", {
#   expect_message({
# terrible_things <- list("","abc",-1,numeric(),
#                         NA,NA_character_,letters ,list())
#   for (ia in terrible_things) {
#     #  i <- 1
#     # i <- i + 1
#     # print(i)
#     
#     try(vapour_vrt(f, projection = ia), silent = TRUE)
#     
#     try(vapour_vrt(f, bands = ia), silent = TRUE)
#     try(vapour_warp_raster(f, extent = 1:4, dimension = c(1, 1), projection = "OGC:CRS84", bands = ia), silent = TRUE)
#     try(vapour_warp_raster(f, extent = 1:4, dimension = c(1, 1), projection = ia), silent = TRUE)
#     try(vapour_warp_raster(f, extent = 1:4, dimension = ia), silent = TRUE)
#     try(vapour_warp_raster(f, extent = ia, dimension = c(1, 1)), silent = TRUE)
#     try(vapour_warp_raster(ia, extent  = 1:4, dimension = c(1, 1)), silent = TRUE)
#     
#     try(vapour_raster_info(ia), silent = TRUE)
#     
#     try(vapour_read_raster(ia, native = TRUE), silent = TRUE)
#     try(vapour_read_raster(ia, window = c(0, 0, 1, 1)), silent = TRUE)
#     try(vapour_read_raster(f, window = ia), silent = TRUE)
#     try(vapour_read_raster(f, window = c(0, 0, 1, 1), band = ia) , silent = TRUE)
#     
#     try(vapour_read_raster(f, window = c(0, 0, 1, 1), band = ia) , silent = TRUE)
#     try(vapour_read_raster(f, window = c(0, 0, 1, 1), band = 1, resample = ia) , silent = TRUE)
#     try(vapour_read_raster(f, window = c(0, 0, 1, 1), band = 1, sds = ia) , silent = TRUE)
#     try(vapour_read_raster(f, window = c(0, 0, 1, 1), band = 1, native = ia) , silent = TRUE)
#     try(vapour_read_raster(f, window = c(0, 0, 1, 1), band = 1, band_output_type = ia), silent = TRUE)
#     
#   }
# 
# 
# }, "unknown 'band_output_type") %>% expect_warning()
# })
# 



