context("test-helpers")

file <- "list_locality_postcode_meander_valley.tab"
mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
gfile <- system.file(file.path("extdata", "sst_c.gpkg"), package="vapour")
osmfile <- system.file(file.path("extdata/osm", "osm-ways.osm"), package="vapour")
##dput(vapour_geom_summary(mvfile, sql = sprintf("SELECT FID FROM %s WHERE FID = 15", vapour_layer_names(mvfile)[1L])))
sql_bench <- list(FID = 15L, valid_geometry = TRUE, type = 3L, xmin = 455980.34, xmax = 463978.72,
                  ymin = 5392869.75, ymax = 5399258.91)

test_that("helpers work", {
  vapour_geom_summary(mvfile) %>% expect_named(c("FID", "valid_geometry", "type", "xmin", "xmax", "ymin", "ymax"))

  vapour_geom_summary(mvfile, sql = sprintf("SELECT FID FROM %s WHERE FID = 15", vapour_layer_names(mvfile)[1L])) %>%
    expect_equal(sql_bench)


  expect_error(validate_limit_n(-2))
  expect_equal(vapour_read_names(gfile, sql = "SELECT FID FROM sst_c LIMIT 2"), c(1, 2))
  expect_equal(vapour_read_names(mvfile), 1:58)
})

test_that("osm arbitrary FIDs as expected", {
  skip_on_os("windows")
  vapour_read_names(osmfile, limit_n = 1) %>% expect_equal(11)

  vapour_read_names(osmfile,  sql = "SELECT FID FROM lines", limit_n = 4) %>% expect_equal(100:103)
})

