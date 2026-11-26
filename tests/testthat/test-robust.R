tif <- system.file("extdata", "sst.tif", package = "vapour", mustWork = TRUE)
gpkg <- system.file("extdata", "sst_c.gpkg", package = "vapour", mustWork = TRUE)

test_that("a source that cannot be opened is an R error with GDAL's reason", {
  ## every one of these reaches GDAL with a source that is not there
  expect_error(vapour_raster_info("nofile.tif"), "unable to open 'nofile.tif': .*No such file")
  expect_error(vapour_raster_gcp("nofile.tif"), "cannot open dataset 'nofile.tif': .*No such file")
  expect_error(vapour_geolocation("nofile.tif"), "cannot open dataset 'nofile.tif'")
  expect_error(vapour_sds_names("nofile.tif"), "unable to open 'nofile.tif'")
  expect_error(vapour_read_raster("nofile.tif", window = c(0, 0, 1, 1)), "unable to open 'nofile.tif'")
  expect_error(vapour_read_raster_block("nofile.tif", c(0L, 0L), c(1L, 1L)), "cannot open dataset 'nofile.tif'")
  expect_error(gdal_raster_data("nofile.tif", target_dim = c(2, 2)), "input source not readable: 'nofile.tif': .*No such file")
  expect_error(vapour_layer_names("nofile.gpkg"), "cannot open 'nofile.gpkg': .*No such file")
  expect_error(vapour_layer_info("nofile.gpkg"), "cannot open 'nofile.gpkg'")
  expect_error(vapour_read_fields("nofile.gpkg"), "cannot open 'nofile.gpkg'")
  expect_error(vapour_read_geometry("nofile.gpkg"), "cannot open 'nofile.gpkg'")
  expect_error(vapour_driver("nofile.gpkg"), "cannot open 'nofile.gpkg'")
  ## the wrong kind of source says so
  expect_error(vapour_layer_names(tif), "not recognized as being in a supported file format")
  ## internal entry points that had no check at all
  expect_error(warp_suggest_cpp("nofile.tif", "EPSG:4326"), "cannot open dataset")
  expect_error(vapour_read_raster_value_cpp("nofile.tif", 0L, 0L, 1L, ""), "cannot open dataset")
  expect_error(blocks_cpp("nofile.tif", 0L, FALSE), "cannot open dataset")
  expect_error(vapour_create_copy_cpp(tif, tempfile(fileext = ".tif"), "NotADriver"), "driver")
})

test_that("GDAL messages are R conditions", {
  ## a failure that does not stop the call is a warning
  expect_warning(v <- vapour_vrt("nofile.tif"), "^GDAL error [0-9]+: .*No such file")
  expect_equal(v, NA_character_)
  ## so it can be suppressed, and promoted
  expect_silent(suppressWarnings(vapour_vrt("nofile.tif")))
  op <- options(warn = 2)
  on.exit(options(op), add = TRUE)
  expect_error(vapour_vrt("nofile.tif"), "No such file")
  options(op)
  ## GDAL's own warnings come through as well
  expect_warning(vapour_layer_names(gpkg, open_options = "NOT_AN_OPTION=1"), "^GDAL warning [0-9]+: .*NOT_AN_OPTION")
  ## SQL that fails carries the driver's message
  expect_error(vapour_read_fields(gpkg, sql = "SELECT nope FROM nowhere"), "SQL execution failed: .*nowhere")
  ## and after errors and warnings the next call is clean
  expect_silent(vapour_raster_info(tif))
})

test_that("a failed read is an error and not a vector of zeros", {
  expect_error(vapour_read_raster_block(tif, c(100000L, 0L), c(10L, 10L)), "raster read failed: .*out of range")
  expect_error(vapour_read_raster_value_cpp(tif, 100000L, 0L, 1L, ""), "failed to read band values")
})

test_that("write block checks the data against the dimension", {
  f <- tempfile(fileext = ".tif")
  vapour_create(f, extent = c(0, 4, 0, 4), dimension = c(4L, 4L), projection = "EPSG:4326", n_bands = 1L)
  ## more values than cells used to read past the end of a buffer
  expect_error(vapour_write_raster_block_cpp(f, as.double(1:1000), c(0L, 0L), c(2L, 2L), 1L, character()),
               "'data' has 1000 values but 'dimension' needs 4")
  expect_error(vapour_write_raster_block_cpp(f, as.double(1:4), c(0L, 0L), c(2L, 2L), 5L, character()), "number of bands")
  expect_true(vapour_write_raster_block(f, 1:4, c(0L, 0L), c(2L, 2L), overwrite = TRUE))
  expect_equal(vapour_read_raster_block(f, c(0L, 0L), c(2L, 2L))[[1L]], c(1, 2, 3, 4))
  expect_error(vapour_write_raster_block("nofile.tif", 1:4, c(0L, 0L), c(2L, 2L), overwrite = TRUE), "cannot open 'nofile.tif' for update")
})

test_that("early exits do not leave datasets open", {
  skip_on_cran()
  skip_if_not(dir.exists("/proc/self/fd"), "needs /proc to count open files")
  d <- tempfile()
  dir.create(d)
  file.copy(tif, d)
  file.copy(gpkg, d)
  tt <- file.path(d, basename(tif))
  gg <- file.path(d, basename(gpkg))
  nfd <- function() length(list.files("/proc/self/fd"))
  ## warm up anything GDAL keeps open for itself (proj.db and so on)
  vapour_vrt(tt, extent = c(0, 1, 0, 1))
  try(vapour_read_fields(gg, sql = "SELECT nope FROM nowhere"), silent = TRUE)
  before <- nfd()
  ## a VRT with any change to the source used to leak the source, every call
  for (i in 1:30) vapour_vrt(tt, extent = c(0, 1, 0, 1), projection = "EPSG:4326")
  for (i in 1:30) vapour_vrt(tt, bands = 5)
  for (i in 1:30) try(vapour_read_raster_block(tt, c(100000L, 0L), c(10L, 10L)), silent = TRUE)
  for (i in 1:30) try(vapour_read_fields(gg, sql = "SELECT nope FROM nowhere"), silent = TRUE)
  for (i in 1:30) try(vapour_read_fields(gg, layer = 5L), silent = TRUE)
  op <- options(warn = 2)
  for (i in 1:30) try(vapour_read_fields(gg, limit_n = 1e6), silent = TRUE)
  options(op)
  expect_lte(nfd(), before + 1L)
})

test_that("building a VRT leaves nothing beside the source", {
  d <- tempfile()
  dir.create(d)
  file.copy(tif, d)
  tt <- file.path(d, basename(tif))
  vrt <- vapour_vrt(tt, geolocation = c(tt, tt))
  expect_true(grepl("GEOLOCATION", vrt))
  v0 <- vapour_vrt(tt)
  v1 <- vapour_vrt(tt, nomd = TRUE)
  ## 'nomd' does what it says
  expect_lt(nchar(v1), nchar(v0))
  expect_false(grepl("AREA_OR_POINT", v1))
  vapour_raster_info(tt, min_max = TRUE)
  expect_equal(list.files(d), basename(tif))
})

test_that("boolean fields are read as logical", {
  j <- tempfile(fileext = ".geojson")
  writeLines('{"type":"FeatureCollection","features":[
    {"type":"Feature","properties":{"flag":true,"n":1},"geometry":{"type":"Point","coordinates":[0,0]}},
    {"type":"Feature","properties":{"flag":false,"n":2},"geometry":null},
    {"type":"Feature","properties":{"flag":null,"n":3},"geometry":null}]}', j)
  ## this was an error for any layer with a boolean field
  x <- vapour_read_fields(j)
  expect_identical(x$flag, c(TRUE, FALSE, NA))
  expect_identical(x$n, 1:3)
  ## a feature with no geometry is NA in text form
  expect_identical(unlist(vapour_read_geometry_text(j, textformat = "wkt")), c("POINT (0 0)", NA, NA))
})

test_that("a FID that is not there is a warning and not a crash", {
  expect_warning(g <- gdal_dsn_read_geom_fa(gpkg, 0L, "", 0, "wkt", c(1, 99999), character()), "FID not found 99999")
  expect_type(g[[1L]], "character")
  expect_null(g[[2L]])
  ## index reads stop at the end of the index
  expect_length(gdal_dsn_read_fids_ia(gpkg, 0L, "", 0, c(0, 2), character()), 2L)
  expect_length(gdal_dsn_read_fields_ia(gpkg, 0L, "", 0, character(), c(0, 2), character())[[1L]], 2L)
})

test_that("open options reach the driver", {
  expect_error(vapour_layer_names(gpkg, open_options = "nonsense"), "'NAME=VALUE'")
  expect_error(vapour_layer_names(gpkg, open_options = 1), "character vector")
  expect_error(vapour_layer_names(gpkg, open_options = NA_character_), "missing values")
  ## none, three ways
  expect_identical(vapour_layer_names(gpkg, open_options = NULL), vapour_layer_names(gpkg))
  expect_identical(vapour_layer_names(gpkg, open_options = character()), vapour_layer_names(gpkg))
  expect_identical(vapour_layer_names(gpkg, open_options = ""), vapour_layer_names(gpkg))

  ## vector: a CSV whose geometry column has to be named (#224)
  csv <- tempfile(fileext = ".csv")
  writeLines(c("id,geom_wkt", "1,\"POINT (1 2)\"", "2,\"POINT (3 4)\""), csv)
  oo <- "GEOM_POSSIBLE_NAMES=geom_wkt"
  expect_true(all(is.na(vapour_layer_extent(csv))))
  expect_equal(vapour_layer_extent(csv, open_options = oo), c(1, 3, 2, 4))
  info <- vapour_layer_info(csv, open_options = oo)
  expect_equal(info$extent, c(1, 3, 2, 4))
  expect_equal(info$count, 2)
  expect_identical(unlist(vapour_read_geometry_text(csv, textformat = "wkt", open_options = oo)),
                   c("POINT (1 2)", "POINT (3 4)"))
  expect_equal(vapour_feature_extent(csv, open_options = oo)[, "xmin"], c(1, 3))
  expect_equal(vapour_geom_summary(csv, open_options = oo)$valid_geometry, c(TRUE, TRUE))
  expect_length(vapour_read_geometry(csv, open_options = oo)[[1L]], 21L)
  expect_length(vapour_read_geometry_ij(csv, ij = c(0, 1), open_options = oo), 2L)
  expect_length(vapour_read_geometry_ia(csv, ia = 1, open_options = oo), 1L)
  expect_equal(vapour_read_type(csv, open_options = oo), c(1L, 1L))
  expect_named(vapour_read_fields(csv, open_options = oo), c("id", "geom_wkt"))
  expect_named(vapour_report_fields(csv, open_options = oo), c("id", "geom_wkt"))
  expect_length(vapour_read_fids(csv, open_options = oo), 2L)
  expect_equal(vapour_driver(csv, open_options = oo), "CSV")

  ## raster: open a GeoTIFF at its first overview
  ov <- system.file("extdata", "volcano_overview.tif", package = "vapour", mustWork = TRUE)
  ro <- "OVERVIEW_LEVEL=0"
  full <- vapour_raster_info(ov)$dimension
  half <- vapour_raster_info(ov, open_options = ro)$dimension
  expect_true(all(half < full))
  expect_length(vapour_read_raster(ov, native = TRUE, open_options = ro)[[1L]], prod(half))
  expect_length(vapour_read_raster_dbl(ov, native = TRUE, open_options = ro), prod(half))
  expect_length(vapour_read_raster_block(ov, c(0L, 0L), half, open_options = ro)[[1L]], prod(half))
  expect_error(vapour_read_raster_block(ov, c(0L, 0L), full, open_options = ro), "raster read failed")
  expect_length(gdal_raster_data(ov, open_options = ro)[[1L]], prod(half))
  ## "-oo" among the warp options used to be accepted and have no effect
  expect_length(gdal_raster_data(ov, options = c("-oo", ro))[[1L]], prod(half))
  expect_equal(attr(gdal_raster_nara(ov, open_options = ro), "dimension"), half)
  ext <- vapour_raster_info(ov)$extent
  a <- vapour_warp_raster(ov, extent = ext, dimension = full, open_options = ro)[[1L]]
  b <- vapour_warp_raster(ov, extent = ext, dimension = full)[[1L]]
  expect_false(identical(a, b))
  ## the VRT keeps the option, so it applies again when the VRT is read
  vrt <- vapour_vrt(ov, extent = c(0, 1, 0, 1), open_options = ro)
  expect_true(grepl("OVERVIEW_LEVEL", vrt))
  expect_equal(vapour_raster_info(vrt)$dimension, half)
  expect_equal(nrow(vapour_sds(ov, open_options = ro)), 1L)
  expect_equal(vapour_sds_names(ov, open_options = ro), ov)
  expect_equal(vapour_geolocation(ov, open_options = ro), list(NULL))
})

test_that("raster info answers for the subdataset that is asked for", {
  skip_on_cran()
  fsds <- system.file("extdata/gdal", "sds.nc", package = "vapour", mustWork = TRUE)
  sds <- try(suppressWarnings(vapour_sds_names(fsds)), silent = TRUE)
  skip_if(inherits(sds, "try-error") || length(sds) < 2L, "netCDF driver not available")
  whole <- vapour_raster_info(fsds)
  expect_length(whole$subdatasets, 2L)
  one <- vapour_raster_info(fsds, sds = 2)
  expect_equal(one$bands, 1L)
  expect_equal(one$dimension, vapour_raster_info(sds[2L])$dimension)
  ## the read checks the window against the subdataset, not the container
  expect_length(vapour_read_raster(fsds, sds = 2, native = TRUE)[[1L]], prod(one$dimension))
  ## geolocation of a subdataset, where an unset item is NA (this was a crash)
  rad <- system.file("extdata/gdal", "geos_rad.nc", package = "vapour", mustWork = TRUE)
  g <- suppressWarnings(vapour_geolocation(rad))
  expect_type(g, "list")
})

test_that("warp sources and options are all used", {
  ## 'transformation_options' used to replace every other option
  expect_error(vapour_warp_raster(tif, extent = c(140, 150, -50, -40), dimension = c(4, 4),
                                  options = "-notanoption", transformation_options = "SRC_METHOD=NO_GEOTRANSFORM"),
               "invalid warp options")
  ## 'source_extent' used to be dropped. With the source placed at 0-1, 0-1 a
  ## warp to that extent gives what a warp to the real extent gives.
  real <- vapour_warp_raster(tif, extent = vapour_raster_info(tif)$extent, dimension = c(4, 4))[[1L]]
  moved <- vapour_warp_raster(tif, extent = c(0, 1, 0, 1), dimension = c(4, 4), source_extent = c(0, 1, 0, 1))[[1L]]
  expect_equal(moved, real)
  expect_false(all(is.na(moved)))
  unmoved <- vapour_warp_raster(tif, extent = c(0, 1, 0, 1), dimension = c(4, 4))[[1L]]
  expect_false(isTRUE(all.equal(unmoved, real)))
})

test_that("fields of features that are counted but not read are NA", {
  ## GeoPackage counts by bounding box and then filters by geometry, so this
  ## box near the origin counts features and reads none
  ex <- c(142, 146, -47, -43)
  expect_warning(x <- vapour_read_fields(gpkg, sql = "SELECT * FROM sst_c", extent = ex), "not as many features")
  skip_if(length(x$sst) < 1L, "this driver counts exactly")
  expect_true(all(is.na(x$sst)))
  expect_true(all(is.na(x$level)))
})
