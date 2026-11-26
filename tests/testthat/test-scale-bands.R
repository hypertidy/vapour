## an Int16 source with known stored values, and VRT views of it that add
## a nodata value, or a scale and offset
stored <- as.integer(c(-5, 0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50))
tf <- tempfile(fileext = ".tif")
vapour_create(tf, extent = c(0, 4, 0, 3), dimension = c(4L, 3L), projection = "EPSG:4326",
              n_bands = 1L, datatype = "Int16")
vapour_write_raster_block(tf, data = stored, offset = c(0L, 0L), dimension = c(4L, 3L),
                          band = 1L, overwrite = TRUE)
vrt_view <- function(extra) {
  sprintf(paste0('<VRTDataset rasterXSize="4" rasterYSize="3"><SRS>EPSG:4326</SRS>',
                 '<GeoTransform>0,1,0,3,0,-1</GeoTransform>',
                 '<VRTRasterBand dataType="Int16" band="1">%s<SimpleSource>',
                 '<SourceFilename>%s</SourceFilename><SourceBand>1</SourceBand>',
                 '</SimpleSource></VRTRasterBand></VRTDataset>'), extra, tf)
}
nodata10 <- vrt_view("<NoDataValue>10</NoDataValue>")
scaled <- vrt_view("<Offset>100</Offset><Scale>0.1</Scale>")
offset_only <- vrt_view("<Offset>100</Offset>")
ex <- c(0, 4, 0, 3)
dm <- c(4, 3)

test_that("only the nodata value is missing in integer reads", {
  expected <- replace(stored, stored == 10L, NA)
  expect_identical(vapour_read_raster(nodata10, native = TRUE)[[1L]], expected)
  expect_identical(vapour_warp_raster(nodata10, extent = ex, dimension = dm)[[1L]], expected)
  expect_equal(gdal_raster_data(nodata10)[[1L]], as.double(expected))
})

test_that("scale and offset are applied by default", {
  expect_equal(vapour_read_raster(scaled, native = TRUE)[[1L]], stored * 0.1 + 100)
  expect_equal(gdal_raster_data(scaled)[[1L]], stored * 0.1 + 100)
  expect_equal(vapour_warp_raster(scaled, extent = ex, dimension = dm)[[1L]], stored * 0.1 + 100)
  ## an offset with no scale is applied too
  expect_equal(vapour_read_raster(offset_only, native = TRUE)[[1L]], stored + 100)
  expect_equal(gdal_raster_data(offset_only)[[1L]], stored + 100)
})

test_that("unscale = FALSE gives the stored values", {
  expect_identical(vapour_read_raster(scaled, native = TRUE, unscale = FALSE)[[1L]], stored)
  expect_identical(vapour_read_raster_block(scaled, c(0L, 0L), c(4L, 3L), unscale = FALSE)[[1L]], stored)
  expect_identical(vapour_warp_raster(scaled, extent = ex, dimension = dm, unscale = FALSE)[[1L]], stored)
  ## gdal_raster_data() returns double unless a type is asked for
  expect_equal(gdal_raster_data(scaled, unscale = FALSE)[[1L]], as.double(stored))
})

test_that("an integer output type gives the stored values", {
  expect_identical(gdal_raster_data(scaled, band_output_type = "Int32")[[1L]], stored)
  expect_identical(vapour_read_raster_int(scaled, native = TRUE), stored)
  expect_identical(vapour_warp_raster_int(scaled, extent = ex, dimension = dm), stored)
})

test_that("band arguments follow one rule", {
  f <- system.file("extdata", "sst.tif", package = "vapour", mustWork = TRUE)
  v3 <- vapour_vrt(f, bands = c(1, 1, 1))
  w <- c(0, 0, 2, 1)
  ## NULL is all bands
  expect_length(vapour_read_raster(v3, band = NULL, window = w), 3L)
  expect_length(gdal_raster_data(v3, target_dim = c(2, 2), bands = NULL), 3L)
  expect_length(vapour_warp_raster(v3, extent = c(140, 150, -60, -40), dimension = c(2, 2)), 3L)
  ## the typed wrappers are one band
  expect_length(vapour_warp_raster_dbl(v3, extent = c(140, 150, -60, -40), dimension = c(2, 2)), 4L)
  ## repeats and any order
  expect_named(vapour_read_raster(v3, band = c(3, 1, 1), window = w), c("Band3", "Band1", "Band1"))
  expect_length(gdal_raster_data(v3, target_dim = c(2, 2), bands = c(3, 1)), 2L)

  ## less than one is an error, or for the old meaning of all bands a warning
  expect_error(vapour_read_raster(v3, band = 0, window = w), "1 or greater")
  expect_error(vapour_read_raster_block(f, c(0L, 0L), c(2L, 1L), band = 0), "1 or greater")
  expect_error(vapour_warp_raster(v3, bands = c(1, 0), extent = c(140, 150, -60, -40), dimension = c(2, 2)), "1 or greater")
  expect_error(gdal_raster_data(v3, target_dim = c(2, 2), bands = c(1, 0)), "1 or greater")
  expect_error(gdal_raster_image(v3, target_dim = c(2, 2), bands = 0), "1 or greater")
  expect_warning(all0 <- gdal_raster_data(v3, target_dim = c(2, 2), bands = 0), "deprecated")
  expect_length(all0, 3L)

  expect_error(gdal_raster_data(v3, target_dim = c(2, 2), bands = 1.5), "whole numbers")
  expect_error(gdal_raster_data(v3, target_dim = c(2, 2), bands = NA_integer_), "no missing values")
  expect_error(gdal_raster_data(v3, target_dim = c(2, 2), bands = "1"), "must be numeric")
  expect_error(gdal_raster_data(v3, target_dim = c(2, 2), bands = 4), "not available")
  expect_error(vapour_read_raster(v3, band = 4, window = w), "maximum band number is 3")
})

test_that("vapour_sds returns a row for each subdataset", {
  f <- system.file("extdata", "sst.tif", package = "vapour", mustWork = TRUE)
  one <- vapour_sds(f)
  expect_s3_class(one, "data.frame")
  expect_named(one, c("dsn", "sds", "subdataset", "description"))
  expect_identical(one$subdataset, f)
  expect_identical(one$sds, 1L)
  expect_true(is.na(one$description))
  expect_identical(vapour_sds(c(f, f))$dsn, c(f, f))
  expect_error(vapour_sds(c(f, "nofile.tif")), "unable to open 'nofile.tif'")

  nc <- system.file("extdata/gdal", "sds.nc", package = "vapour", mustWork = TRUE)
  skip_if_not("netCDF" %in% vapour_all_drivers()$driver)
  both <- vapour_sds(c(nc, f))
  expect_identical(both$sds, c(1L, 2L, 1L))
  expect_identical(both$subdataset[1:2], vapour_sds_names(nc))
  expect_match(both$description[1L], "vv")
})
