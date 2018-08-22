context("test-features.R")


f <- system.file("extdata", "sst_c.gpkg", package = "vapour")

pfile <- "list_locality_postcode_meander_valley.tab"
dsource <- system.file(file.path("extdata/tab", pfile), package="vapour")

test_that("geometry read works", {

  gbin <- vapour_read_geometry_cpp(f, what = "geometry")
  gjsn <- vapour_read_geometry_cpp(f, what = "text", textformat = "json")
  ggml <- vapour_read_geometry_cpp(f, what = "text", textformat = "gml")
  gkml <- vapour_read_geometry_cpp(f, what = "text", textformat = "kml")
  gwkt <- vapour_read_geometry_cpp(f, what = "text", textformat = "wkt")
  gext <- vapour_read_geometry_cpp(f, what = "extent")

  gbin %>% expect_length(7L)
  gjsn %>% expect_length(7L)
  ggml %>% expect_length(7L)
  gkml %>% expect_length(7L)
  gwkt %>% expect_length(7L)
  gext %>% expect_length(7L)

  gbin[[1]] %>% expect_type("raw")
  gjsn[[1]] %>% expect_type("character") %>% grepl("MultiLineString", .) %>% expect_true()
  ggml[[1]] %>% expect_type("character") %>% grepl("gml:MultiLineString", .) %>% expect_true()
  gkml[[1]] %>% expect_type("character")  %>% grepl("<MultiGeometry><LineString>", .) %>% expect_true()
  gwkt[[1]] %>% expect_type("character")   %>% grepl("MULTILINESTRING \\(\\(-16254", .) %>% expect_true()
  gext[[4]] %>% expect_type("double")  %>% trunc() %>% expect_identical(c(-9293382, 7088338, -6881739, 9067994))

  expect_identical(gjsn, vapour_read_geometry_text(f))
  expect_identical(ggml, vapour_read_geometry_text(f, textformat = "gml"))
  expect_identical(gkml, vapour_read_geometry_text(f, textformat = "kml"))
  expect_identical(gwkt, vapour_read_geometry_text(f, textformat = "wkt"))
  expect_identical(gext, vapour_read_extent(f))

  expect_error(vapour_read_extent(dsource, layer = "list_locality_postcode_meander_val"), "layer index not found for")
  expect_silent(vapour_read_attributes(dsource, layer = "list_locality_postcode_meander_valley"))


})

test_that("empty geometry set as expected", {

  efile <- system.file("extdata/point.dbf", package = "vapour")
  emptygeomfile <- sprintf("%s.dbf", tempfile())
  file.copy(efile, emptygeomfile)
  expect_warning(empty <- vapour_geom_summary(emptygeomfile))
  expect_true(!any(empty$valid_geometry))
  unlink(emptygeomfile)
})


test_that("limit_n works",
          {
            vapour_geom_summary(f, limit_n = 1L) %>% unlist() %>% expect_length(6L)

            vapour_geom_summary(dsource, limit_n = 1L) %>% unlist() %>% expect_length(6L)
            av_atts <- vapour_read_attributes(f, limit_n = 1) %>% expect_length(2L) %>% expect_named(c("level", "sst"))
            vapour_read_geometry(f, limit_n = 1L) %>% expect_length(1L)

            vapour_read_geometry_text(f, limit_n = 3L) %>% expect_length(3L)

            vapour_read_extent(f, limit_n = 3L) %>% unlist() %>% expect_length(12L)

          }


          )
