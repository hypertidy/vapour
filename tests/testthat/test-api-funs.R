test_that("api fun works", {
  
  dsn <- system.file("extdata/point.shp", package = "vapour", mustWork = TRUE)
  layer <- 0L
  sql <- ""
  ex <- 0
  format <- "wkt"
  
  ## these calls not being tested by examples or tests 2021-09-09
  wkt <- expect_length(gdal_dsn_read_geom_all(dsn, layer, sql, ex, format), 10L) 
  expect_true(all(grepl("^POINT", unlist(wkt, use.names = FALSE))))
  
  gml <- expect_length(gdal_dsn_read_geom_all(dsn, layer, sql, ex, "gml"), 10L) 
  expect_true(all(grepl("^<gml", unlist(gml, use.names = FALSE))))

  json <- expect_length(gdal_dsn_read_geom_all(dsn, layer, sql, ex, "json"), 10L) 
  expect_true(all(grepl("\\{ \"type\": \"Point", unlist(json, use.names = FALSE))))

  kml <- expect_length(gdal_dsn_read_geom_all(dsn, layer, sql, ex, "kml"), 10L) 
  expect_true(all(grepl("\\<Point\\>", unlist(json, use.names = FALSE))))
  
  
  expect_equal(vapour:::vapour_read_type(dsn), rep(1, 10))
})
