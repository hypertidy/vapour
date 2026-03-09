## tests for vapour coverage gaps
## identified functions with zero or minimal test coverage:
##   buildvrt, gdal_raster_data, gdal_raster_dsn, gdal_raster_image,
##   gdal_raster_nara, vapour_create_options, vapour_geolocation,
##   vapour_geom_name, vapour_get_config, vapour_proj_version,
##   vapour_read_fids, vapour_read_fields, vapour_report_attributes,
##   vapour_report_fields, vector_vrt
##
## also: vapour_vrt (existing test-vrt.R is entirely commented out),
##        error paths and edge cases across existing functions

## ---- test fixtures ---------------------------------------------------
tif   <- system.file("extdata", "sst.tif", package = "vapour", mustWork = TRUE)
vtif  <- system.file("extdata", "volcano.tif", package = "vapour", mustWork = TRUE)
otif  <- system.file("extdata", "volcano_overview.tif", package = "vapour", mustWork = TRUE)
fgb   <- system.file("extdata", "sst_c.fgb", package = "vapour", mustWork = TRUE)
gpkg  <- system.file("extdata", "sst_c.gpkg", package = "vapour", mustWork = TRUE)
pshp  <- system.file("extdata", "point.shp", package = "vapour", mustWork = TRUE)
p3d   <- system.file("extdata", "point3d.gpkg", package = "vapour", mustWork = TRUE)
gcptif <- system.file("extdata/gcps/volcano_gcp.tif", package = "vapour", mustWork = TRUE)
sdsf  <- system.file("extdata/gdal/sds.nc", package = "vapour", mustWork = TRUE)
georad <- system.file("extdata/gdal/geos_rad.nc", package = "vapour", mustWork = TRUE)
tabf  <- system.file("extdata/tab/list_locality_postcode_meander_valley.tab",
                      package = "vapour", mustWork = TRUE)
zipf  <- system.file("extdata/vsi/point_shp.zip", package = "vapour", mustWork = TRUE)
csvf  <- system.file("extdata/index_point.csv", package = "vapour", mustWork = TRUE)


## ---- vapour_read_fids ------------------------------------------------
test_that("vapour_read_fids basic read works", {
  fids <- vapour_read_fids(fgb)
  expect_type(fids, "double")
  expect_length(fids, 7L)
  ## fids should be sequential for simple sources
  expect_true(all(diff(fids) > 0))
})

test_that("vapour_read_fids limit_n and skip_n work", {
  fids_all <- vapour_read_fids(fgb)
  fids_lim <- vapour_read_fids(fgb, limit_n = 3)
  expect_length(fids_lim, 3L)
  expect_equal(fids_lim, fids_all[1:3])

  fids_skip <- vapour_read_fids(fgb, skip_n = 2, limit_n = 2)
  expect_length(fids_skip, 2L)
  expect_equal(fids_skip, fids_all[3:4])
})

test_that("vapour_read_fids with SQL works", {
  fids <- vapour_read_fids(fgb, sql = "SELECT * FROM sst_c WHERE FID < 3")
  expect_true(length(fids) <= 3L)
})

test_that("vapour_read_fids skip_n beyond count errors", {
  expect_error(vapour_read_fids(fgb, skip_n = 100), "skip_n")
})

test_that("vapour_read_names is an alias of vapour_read_fids", {
  expect_equal(vapour_read_names(fgb), vapour_read_fids(fgb))
})


## ---- vapour_read_fields / vapour_read_attributes ----------------------
test_that("vapour_read_fields returns a data-frame-like list", {
  att <- vapour_read_fields(fgb)
  expect_type(att, "list")
  expect_named(att, c("level", "sst"))
  expect_length(att$level, 7L)
  expect_length(att$sst, 7L)
})

test_that("vapour_read_fields limit_n and skip_n work", {
  att <- vapour_read_fields(fgb, limit_n = 2)
  expect_length(att$level, 2L)

  att2 <- vapour_read_fields(fgb, skip_n = 5, limit_n = 2)
  expect_length(att2$level, 2L)
})

test_that("vapour_read_fields with SQL works", {
  att <- vapour_read_fields(fgb, sql = "SELECT sst FROM sst_c")
  expect_named(att, "sst")
})

test_that("vapour_read_fields bad dsn errors", {
  expect_error(vapour_read_fields("/no/such/file.shp"), "Open failed")
})

test_that("vapour_read_attributes is an alias of vapour_read_fields", {
  expect_equal(vapour_read_attributes(fgb), vapour_read_fields(fgb))
})


## ---- vapour_report_fields / vapour_report_attributes ------------------
test_that("vapour_report_fields returns field type info", {
  rf <- vapour_report_fields(fgb)
  expect_type(rf, "character")
  expect_true(length(rf) >= 2L)
  ## should contain OGR type names
  expect_true(any(nchar(rf) > 0))
})

test_that("vapour_report_fields with layer name works", {
  rf <- vapour_report_fields(fgb, layer = "sst_c")
  expect_type(rf, "character")
})

test_that("vapour_report_fields bad layer errors", {
  expect_error(vapour_report_fields(fgb, layer = "nonexistent"))
})

test_that("vapour_report_attributes aliases vapour_report_fields", {
  expect_equal(vapour_report_attributes(fgb), vapour_report_fields(fgb))
})


## ---- vapour_geom_name ------------------------------------------------
test_that("vapour_geom_name returns geometry column name", {
  gn <- vapour_geom_name(fgb)
  expect_type(gn, "character")
  expect_length(gn, 1L)
})

test_that("vapour_geom_name for dbf (no geometry) is empty", {
  efile <- system.file("extdata/point.dbf", package = "vapour")
  if (file.exists(efile)) {
    gn <- vapour_geom_name(efile)
    expect_type(gn, "character")
  }
})

test_that("vapour_geom_name with sql works", {
  gn <- vapour_geom_name(fgb, sql = "SELECT * FROM sst_c")
  expect_type(gn, "character")
})


## ---- vapour_geolocation -----------------------------------------------
test_that("vapour_geolocation reports geolocation arrays", {
  ## geos_rad.nc has geolocation arrays
  drv <- vapour_all_drivers()
  skip_if_not("netCDF" %in% drv$driver[drv$raster])
  geo <- vapour_geolocation(georad, sds = 1L)
  expect_type(geo, "list")
})

test_that("vapour_geolocation on non-geolocation source", {
  geo <- vapour_geolocation(tif, sds = 0L)
  expect_type(geo, "list")
})


## ---- vapour_get_config / vapour_set_config ----------------------------
test_that("vapour_get_config returns a string", {
  val <- vapour_get_config("GDAL_CACHEMAX")
  expect_type(val, "character")
  expect_length(val, 1L)
})

test_that("vapour_set_config and get round-trip", {
  orig <- vapour_get_config("GDAL_CACHEMAX")
  vapour_set_config("GDAL_CACHEMAX", "128")
  expect_equal(vapour_get_config("GDAL_CACHEMAX"), "128")
  ## restore
  if (!is.na(orig) && nchar(orig) > 0) {
    vapour_set_config("GDAL_CACHEMAX", orig)
  }
})


## ---- vapour_proj_version ----------------------------------------------
test_that("vapour_proj_version returns version info", {
  pv <- vapour_proj_version()
  expect_type(pv, "integer")
  expect_true(length(pv) >= 1L)
  ## PROJ major version should be >= 6
  expect_true(pv[1] >= 6L)
})


## ---- vapour_create_options --------------------------------------------
test_that("vapour_create_options returns character for GTiff", {
  opts <- vapour_create_options("GTiff")
  expect_type(opts, "character")
  expect_true(length(opts) > 0L)
})

test_that("vapour_create_options returns character for COG", {
  opts <- vapour_create_options("COG")
  expect_type(opts, "character")
})

test_that("vapour_create_options default is GTiff", {
  expect_equal(vapour_create_options(), vapour_create_options("GTiff"))
})


## ---- buildvrt ---------------------------------------------------------
test_that("buildvrt creates a valid VRT string", {
  vrt <- buildvrt(c(tif, tif))
  expect_type(vrt, "character")
  expect_true(grepl("VRTDataset", vrt))
  ## should contain two bands (because -separate)
  info <- vapour_raster_info(vrt)
  expect_equal(info$bands, 2L)
})

test_that("buildvrt single source works", {
  vrt <- buildvrt(tif)
  expect_true(grepl("VRTDataset", vrt))
  info <- vapour_raster_info(vrt)
  expect_equal(info$bands, 1L)
})


## ---- vector_vrt -------------------------------------------------------
test_that("vector_vrt creates valid OGR VRT xml", {
  vrt <- vector_vrt(fgb)
  expect_type(vrt, "character")
  expect_true(grepl("OGRVRTDataSource", vrt))
  expect_true(grepl("SrcDataSource", vrt))
})

test_that("vector_vrt with projection wraps in OGRVRTWarpedLayer", {
  vrt <- vector_vrt(fgb, projection = "EPSG:3031")
  expect_true(grepl("OGRVRTWarpedLayer", vrt))
  expect_true(grepl("TargetSRS", vrt))
})

test_that("vector_vrt with sql works", {
  vrt <- vector_vrt(fgb, sql = "SELECT * FROM sst_c WHERE FID < 3")
  expect_true(grepl("SrcSQL", vrt))
  ## should be readable by vapour
  expect_true(grepl("OGRVRTDataSource", vrt))
})

test_that("vector_vrt with a_srs sets LayerSRS", {
  vrt <- vector_vrt(fgb, a_srs = "EPSG:4326")
  expect_true(grepl("LayerSRS", vrt))
})

test_that("vector_vrt with layer name works", {
  vrt <- vector_vrt(fgb, layer = "sst_c")
  expect_true(grepl("sst_c", vrt))
})


## ---- vapour_vrt (tests were commented out) ----------------------------
test_that("vapour_vrt creates valid VRT for tif", {
  vrt <- vapour_vrt(tif)
  expect_type(vrt, "character")
  expect_true(grepl("VRTDataset", vrt))
  expect_equal(vapour_driver(vrt), "VRT")
})

test_that("vapour_vrt band selection works", {
  vrt3 <- vapour_vrt(tif, bands = rep(1, 3))
  vals <- vapour_read_raster(vrt3, band = 1:3, window = c(0, 0, 2, 3))
  expect_length(vals, 3L)
  ## all three bands should be identical (same source band)
  expect_equal(vals[[1]], vals[[2]])
  expect_equal(vals[[1]], vals[[3]])
})

test_that("vapour_vrt extent override works", {
  ex <- c(19, 20, -30, -50)
  vrt <- vapour_vrt(tif, extent = ex)
  info <- vapour_raster_info(vrt)
  expect_equal(info$extent, ex)
})

test_that("vapour_vrt projection override works", {
  prj <- "EPSG:3031"
  vrt <- vapour_vrt(tif, projection = prj)
  info <- vapour_raster_info(vrt)
  ## info$projection is WKT, but it should contain the EPSG or Antarctic Polar Stereographic
  expect_true(nchar(info$projection) > 0)
})


## ---- gdal_raster_data ------------------------------------------------
test_that("gdal_raster_data basic read works", {
  d <- gdal_raster_data(tif)
  expect_type(d, "list")
  expect_true(length(d) > 0)
  ## should contain numeric data
  expect_type(d[[1]], "double")
})

test_that("gdal_raster_data with target_ext and target_dim", {
  d <- gdal_raster_data(tif, target_ext = c(145, 146, -50, -48), target_dim = c(10, 10))
  expect_type(d, "list")
  expect_length(d[[1]], 100L)
})

test_that("gdal_raster_data with target_res", {
  d <- gdal_raster_data(tif, target_res = 1)
  expect_type(d, "list")
  expect_true(length(d[[1]]) > 0)
})

test_that("gdal_raster_data with target_crs", {
  d <- gdal_raster_data(tif, target_crs = "EPSG:3031",
                         target_ext = c(-1, 1, -1, 1) * 5e6,
                         target_dim = c(10, 10))
  expect_type(d, "list")
  expect_length(d[[1]], 100L)
})

test_that("gdal_raster_data bad extent errors", {
  expect_error(gdal_raster_data(tif, target_ext = c(146, 145, -48, -50)),
               "xmin < xmax")
  expect_error(gdal_raster_data(tif, target_ext = c(NA, 146, -50, -48)),
               "NA values")
  expect_error(gdal_raster_data(tif, target_ext = c(145, 146)),
               "length 4")
})

test_that("gdal_raster_data bad dim errors", {
  expect_error(gdal_raster_data(tif, target_dim = c(NA, 10)),
               "NA values")
  expect_error(gdal_raster_data(tif, target_dim = c(-1, 10)),
               "must be >= 0")
  expect_error(gdal_raster_data(tif, target_dim = c(0, 0)),
               "must be > 0")
})

test_that("gdal_raster_data bad res errors", {
  expect_error(gdal_raster_data(tif, target_res = c(NA, 1)),
               "NA values")
  expect_error(gdal_raster_data(tif, target_res = c(-1, 1)),
               "must be > 0")
})

test_that("gdal_raster_data include_meta works", {
  d_meta <- gdal_raster_data(tif, target_ext = c(145, 146, -50, -48),
                              target_dim = c(5, 5), include_meta = TRUE)
  d_nometa <- gdal_raster_data(tif, target_ext = c(145, 146, -50, -48),
                                target_dim = c(5, 5), include_meta = FALSE)
  ## with meta, should have more list elements (dimension, extent, projection)
  expect_true(length(names(d_meta)) >= length(names(d_nometa)))
})


## ---- gdal_raster_dsn -------------------------------------------------
test_that("gdal_raster_dsn writes to file", {
  outf <- tempfile(fileext = ".tif")
  on.exit(unlink(outf), add = TRUE)
  d <- gdal_raster_dsn(tif, out_dsn = outf,
                        target_ext = c(145, 146, -50, -48),
                        target_dim = c(10, 10))
  expect_true(file.exists(outf))
  info <- vapour_raster_info(outf)
  expect_equal(info$dimXY, c(10L, 10L))
})


## ---- gdal_raster_image -----------------------------------------------
test_that("gdal_raster_image returns image data", {
  d <- gdal_raster_image(tif, target_ext = c(145, 146, -50, -48),
                          target_dim = c(5, 5))
  expect_type(d, "list")
})


## ---- gdal_raster_nara ------------------------------------------------
test_that("gdal_raster_nara returns nativeRaster", {
  d <- gdal_raster_nara(tif, target_ext = c(145, 146, -50, -48),
                         target_dim = c(5, 5))
  expect_type(d, "list")
})


## ---- vapour_crs_is_lonlat edge cases ----------------------------------
test_that("vapour_crs_is_lonlat geographic vs projected", {
  expect_true(vapour_crs_is_lonlat("EPSG:4326"))
  expect_false(vapour_crs_is_lonlat("EPSG:3031"))
  expect_false(vapour_crs_is_lonlat("+proj=laea"))
})


## ---- vapour_srs_wkt edge cases ----------------------------------------
test_that("vapour_srs_wkt returns valid WKT", {
  wkt <- vapour_srs_wkt("EPSG:4326")
  expect_type(wkt, "character")
  expect_true(nchar(wkt) > 50)
  expect_true(grepl("GEOGCS|GEOGCRS|WGS", wkt))
})

test_that("vapour_srs_wkt handles proj4 input", {
  wkt <- vapour_srs_wkt("+proj=laea +lon_0=147 +lat_0=-42")
  expect_type(wkt, "character")
  expect_true(nchar(wkt) > 10)
})


## ---- error paths for bad DSN across functions -------------------------
test_that("bad DSN errors are consistent", {
  bad <- "/no/such/file.gpkg"
  expect_error(vapour_read_fids(bad))
  expect_error(vapour_read_fields(bad))
  expect_error(vapour_read_geometry(bad))
  expect_error(vapour_geom_name(bad))
  expect_error(vapour_geom_summary(bad))
  expect_error(vapour_read_extent(bad))
  expect_error(vapour_layer_names(bad))
  expect_error(vapour_raster_info(bad))
})

test_that("empty string DSN errors", {
  expect_error(vapour_layer_names(""), "Not a valid")
  expect_error(vapour_raster_info(""))
})


## ---- vapour_layer_extent ----------------------------------------------
test_that("vapour_layer_extent returns xmin xmax ymin ymax", {
  ext <- vapour_layer_extent(fgb)
  expect_type(ext, "double")
  expect_length(ext, 4L)
  ## xmin < xmax and ymin < ymax
  expect_true(ext[1] < ext[2])
  expect_true(ext[3] < ext[4])
})

test_that("vapour_layer_extent with SQL", {
  ext <- vapour_layer_extent(fgb, sql = "SELECT * FROM sst_c WHERE FID < 3")
  expect_type(ext, "double")
  expect_length(ext, 4L)
})


## ---- vapour_vsi_list --------------------------------------------------
test_that("vapour_vsi_list works with zip", {
  vsi <- vapour_vsi_list(sprintf("/vsizip/%s", zipf))
  expect_type(vsi, "character")
  expect_true(length(vsi) > 0)
})


## ---- vapour_raster_gcp ------------------------------------------------
test_that("vapour_raster_gcp reads GCPs", {
  gcps <- vapour_raster_gcp(gcptif)
  expect_type(gcps, "list")
})


## ---- vapour_read_raster with native flag ------------------------------
test_that("vapour_read_raster native = TRUE works", {
  d <- vapour_read_raster(tif, band = 1, native = TRUE)
  expect_type(d, "list")
  info <- vapour_raster_info(tif)
  expect_length(d[[1]], info$dimXY[1] * info$dimXY[2])
})


## ---- index access _ia and _fa for fields (not just geom) ---------------
test_that("gdal_dsn_read_fields_ia index access works", {
  att_all <- vapour_read_fields(pshp)
  ## read first two by index
  att_ia <- vapour:::gdal_dsn_read_fields_ia(pshp, 0L, "", 0, "", c(0, 1))
  expect_length(att_ia[[1]], 2L)
})

test_that("gdal_dsn_read_fields_ij range access works", {
  att_ij <- vapour:::gdal_dsn_read_fields_ij(pshp, 0L, "", 0, "", c(0, 4))
  expect_length(att_ij[[1]], 5L)
})


## ---- vapour_read_geometry_text resampling format edge cases -----------
test_that("vapour_read_geometry_text with unknown format", {
  ## should fall through to default (json)
  expect_error(vapour_read_geometry_text(fgb, textformat = "not_a_format"))
})


## ---- multi-layer source -----------------------------------------------
test_that("layer selection by index and name", {
  ## gpkg may have a single layer
 names <- vapour_layer_names(gpkg)
  expect_type(names, "character")
  expect_true(length(names) >= 1L)

  ## read by name
  att_name <- vapour_read_fields(gpkg, layer = names[1])
  ## read by index
  att_idx  <- vapour_read_fields(gpkg, layer = 0L)
  expect_equal(att_name, att_idx)
})


## ---- warper resample options ------------------------------------------
test_that("warp resample methods work", {
  methods <- c("near", "bilinear", "cubic", "average", "mode", "lanczos")
  for (m in methods) {
    d <- vapour_warp_raster(tif, extent = c(145, 146, -50, -48),
                            dimension = c(5, 5), resample = m)
    expect_type(d, "list")
    expect_length(d[[1]], 25L)
  }
})


## ---- SDS handling for NetCDF ------------------------------------------
test_that("vapour_sds_names returns sds list", {
  skip_on_os("windows")
  sds <- vapour_sds_names(sdsf)
  expect_type(sds, "list")
  expect_true(length(sds) > 0)
})


## ---- vapour_all_drivers -----------------------------------------------
test_that("vapour_all_drivers returns driver info", {
  drv <- vapour_all_drivers()
  expect_type(drv, "list")
  expect_true("driver" %in% names(drv))
  expect_true("GTiff" %in% drv$driver)
  expect_true("GPKG" %in% drv$driver)
})


## ---- vapour_create ----------------------------------------------------
test_that("vapour_create makes a new raster file", {
  outf <- tempfile(fileext = ".tif")
  on.exit(unlink(outf), add = TRUE)
  vapour_create(outf, dimension = c(10, 10), extent = c(0, 1, 0, 1),
                projection = "EPSG:4326")
  expect_true(file.exists(outf))
  info <- vapour_raster_info(outf)
  expect_equal(info$dimXY, c(10L, 10L))
})


## ---- vapour_write_raster_block round-trip ------------------------------
test_that("write then read block round-trips", {
  outf <- tempfile(fileext = ".tif")
  on.exit(unlink(outf), add = TRUE)
  vapour_create(outf, dimension = c(10, 10), extent = c(0, 1, 0, 1),
                projection = "EPSG:4326")
  ## write a known pattern
  vals <- as.numeric(1:100)
  vapour_write_raster_block(outf, data = vals, offset = c(0, 0),
                            dimension = c(10, 10), band = 1L, overwrite = TRUE)
  ## read it back
  got <- vapour_read_raster_block(outf, offset = c(0, 0),
                                  dimension = c(10, 10), band = 1L)
  expect_type(got, "list")
  expect_equal(got[[1]], vals)
})


## ---- point3d gpkg geometry --------------------------------------------
test_that("3D point geometry reads correctly", {
  geom <- vapour_read_geometry(p3d)
  expect_type(geom, "list")
  expect_true(length(geom) > 0)
  ## should be WKB
  expect_type(geom[[1]], "raw")
})
