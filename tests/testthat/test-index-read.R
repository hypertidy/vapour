tab <- system.file("extdata/tab", "list_locality_postcode_meander_valley.tab", package = "vapour", mustWork = TRUE)
gpkg <- system.file("extdata", "sst_c.gpkg", package = "vapour", mustWork = TRUE)

geom <- vapour_read_geometry(tab)
fields <- vapour_read_fields(tab)
fids <- vapour_read_fids(tab)
n <- length(fids)

test_that("geometry is read by position, range and feature id", {
  expect_identical(vapour_read_geometry_ia(tab, ia = c(0, 3, 5)), geom[c(1, 4, 6)])
  ## any order, and repeats: these used to give empty elements
  expect_identical(vapour_read_geometry_ia(tab, ia = c(5, 3, 0)), geom[c(6, 4, 1)])
  expect_identical(vapour_read_geometry_ia(tab, ia = c(3, 3, 0, 3)), geom[c(4, 4, 1, 4)])
  expect_identical(vapour_read_geometry_ia(tab, ia = n - 1), geom[n])
  ## a position past the end is NULL
  past <- vapour_read_geometry_ia(tab, ia = c(n + 10, 2))
  expect_null(past[[1L]])
  expect_identical(past[[2L]], geom[[3L]])

  expect_identical(vapour_read_geometry_ij(tab, ij = c(1, 3)), geom[2:4])
  expect_identical(vapour_read_geometry_ij(tab, ij = c(0, n - 1)), geom)

  expect_identical(vapour_read_geometry_fa(tab, fa = fids[c(1, 4, 6)]), geom[c(1, 4, 6)])
  expect_identical(vapour_read_geometry_fa(tab, fa = fids[c(6, 1, 1)]), geom[c(6, 1, 1)])
  expect_warning(g <- vapour_read_geometry_fa(tab, fa = c(fids[2], max(fids) + 100)), "FID not found")
  expect_identical(g[[1L]], geom[[2L]])
  expect_null(g[[2L]])
  ## by name of layer, as the other readers
  expect_identical(vapour_read_geometry_fa(tab, layer = "list_locality_postcode_meander_valley", fa = fids[3]), geom[3])
})

test_that("fields are read by position, range and feature id", {
  pick <- function(i) lapply(fields, function(x) x[i])
  expect_identical(vapour_read_fields_ia(tab, ia = c(0, 3, 5)), pick(c(1, 4, 6)))
  expect_identical(vapour_read_fields_ia(tab, ia = c(5, 3, 0, 5)), pick(c(6, 4, 1, 6)))
  expect_identical(vapour_read_fields_ij(tab, ij = c(1, 3)), pick(2:4))
  expect_identical(vapour_read_fields_ij(tab, ij = c(0, n - 1)), fields)
  expect_identical(vapour_read_fields_fa(tab, fa = fids[c(6, 1, 4)]), pick(c(6, 1, 4)))

  ## what is not there is NA, with a warning
  expect_warning(x <- vapour_read_fields_ia(tab, ia = c(1, n + 10)), "not all features found")
  expect_identical(x$NAME, c(fields$NAME[2L], NA))
  expect_warning(x <- vapour_read_fields_fa(tab, fa = c(max(fids) + 100, fids[2])), "FID not found")
  expect_identical(x$NAME, c(NA, fields$NAME[2L]))
  expect_warning(x <- vapour_read_fields_ij(tab, ij = c(n - 2, n + 1)), "not as many features")
  expect_identical(x$NAME, c(fields$NAME[(n - 1):n], NA, NA))
})

test_that("feature ids are read by position and range", {
  expect_identical(vapour_read_fids_ia(tab, ia = c(0, 3, 5)), fids[c(1, 4, 6)])
  expect_identical(vapour_read_fids_ia(tab, ia = c(5, 5, 0)), fids[c(6, 6, 1)])
  expect_identical(vapour_read_fids_ij(tab, ij = c(1, 3)), fids[2:4])
  expect_identical(vapour_read_fids_ij(tab, ij = c(0, n - 1)), fids)
  expect_warning(x <- vapour_read_fids_ia(tab, ia = c(0, n + 10)), "not all FIDS found")
  expect_identical(x, c(fids[1L], NA))
  ## positions to feature ids to features, the round trip the three are for
  i <- c(7, 2, 9)
  expect_identical(vapour_read_geometry_fa(tab, fa = vapour_read_fids_ia(tab, ia = i)), geom[i + 1])
})

test_that("index readers work on a SQL result and with a spatial filter", {
  ## five of the seven contours
  sql <- "SELECT * FROM sst_c WHERE sst > 10"
  g <- vapour_read_geometry(gpkg, sql = sql)
  a <- vapour_read_fields(gpkg, sql = sql)
  expect_length(g, 5L)
  expect_identical(vapour_read_geometry_ia(gpkg, sql = sql, ia = c(2, 0)), g[c(3, 1)])
  expect_identical(vapour_read_geometry_ij(gpkg, sql = sql, ij = c(0, 1)), g[1:2])
  expect_identical(vapour_read_fields_ia(gpkg, sql = sql, ia = c(2, 0))$sst, a$sst[c(3, 1)])
  expect_identical(vapour_read_fields_ij(gpkg, sql = sql, ij = c(0, 1))$sst, a$sst[1:2])
  expect_length(vapour_read_fids_ij(gpkg, sql = sql, ij = c(0, 1)), 2L)
})

test_that("index arguments are checked", {
  expect_error(vapour_read_fields_ia(tab), "ia, is NULL")
  expect_error(vapour_read_fields_ia(tab, ia = "1"), "ia must be numeric")
  expect_error(vapour_read_fields_ia(tab, ia = c(-1, 1)), "ia values < 0")
  expect_error(vapour_read_fids_ia(tab, ia = c(NA, 1)), "missing values ia")
  expect_error(vapour_read_fids_ia(tab, ia = numeric()), "no valid ia")
  expect_error(vapour_read_fields_ij(tab, ij = c(5, 4)), "must be increasing")
  expect_error(vapour_read_fids_ij(tab, ij = 1), "not of length 2")
  expect_error(vapour_read_fields_ij(tab, ij = c("a", "b")), "ij must be numeric")
  expect_error(vapour_read_geometry_fa(tab), "fa, is NULL")
  expect_error(vapour_read_geometry_fa(tab, fa = NA_real_), "missing values fa")
  expect_error(vapour_read_fields_fa(tab, fa = "1"), "fa must be numeric")
  expect_error(vapour_read_fields_fa("nofile.gpkg", fa = 1), "cannot open 'nofile.gpkg'")
})

test_that("the internal read-everything entry points agree with the exported readers", {
  no <- character()
  expect_identical(gdal_dsn_read_geom_all(tab, 0L, "", 0, "wkb", no), geom)
  expect_identical(gdal_dsn_read_fields_all(tab, 0L, "", 0, no, no), fields)
  expect_identical(gdal_dsn_read_fids_all(tab, 0L, "", 0, no), fids)
})
