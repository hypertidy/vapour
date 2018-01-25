context("test-features.R")


f <- system.file("extdata", "sst_c.gpkg", package = "vapour")
gbin <- vapour_read_feature_what(f, what = "geometry")
gjsn <- vapour_read_feature_what(f, what = "text", textformat = "json")
ggml <- vapour_read_feature_what(f, what = "text", textformat = "gml")
gkml <- vapour_read_feature_what(f, what = "text", textformat = "kml")
gwkt <- vapour_read_feature_what(f, what = "text", textformat = "wkt")
gext <- vapour_read_feature_what(f, what = "extent")


test_that("geometry read works", {
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

})