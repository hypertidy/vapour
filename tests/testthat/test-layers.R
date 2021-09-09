f2 <- system.file("extdata/osm/myosmfile.osm", package = "vapour", mustWork = TRUE)
f <- system.file("extdata", "sst_c.gpkg", package = "vapour", mustWork = TRUE)


test_that("layer logic works", {
  expect_equal(vapour_layer_names(f2), c("points", "lines", "multilinestrings", "multipolygons", "other_relations"))
  
  lt <- expect_type(vapour_layer_info(f2, count = FALSE), "list")
  expect_equal(lt, vapour_layer_info(f2, 0, count = FALSE))
  expect_equal(lt, vapour_layer_info(f2, "points", count = FALSE))
  ltna <- lt
  ltna$count <- 1L
  expect_equal(ltna, vapour_layer_info(f2, "points", count = TRUE))  

  expect_equal(vapour_read_attributes(f2, 3), vapour_read_attributes(f2, "multipolygons"))
  expect_equal(vapour_read_names(f2, 3), vapour_read_names(f2, "multipolygons"))
  expect_equal(vapour_read_geometry(f2, 3), vapour_read_geometry(f2, "multipolygons"))
  expect_equal(vapour_read_geometry_text(f2, 3), vapour_read_geometry_text(f2, "multipolygons"))
  expect_equal(vapour_read_extent(f2, 3), vapour_read_extent(f2, "multipolygons"))
  
  
  
  expect_equal(vapour_read_attributes(f, 0), vapour_read_attributes(f, "sst_c"))
  expect_equal(vapour_read_names(f, 0), vapour_read_names(f, "sst_c"))
  expect_equal(vapour_read_geometry(f, 0), vapour_read_geometry(f, "sst_c"))
  expect_equal(vapour_read_geometry_text(f, 0), vapour_read_geometry_text(f, "sst_c"))
  expect_equal(vapour_read_extent(f, 0), vapour_read_extent(f, "sst_c"))
  
  
  expect_error(vapour_read_attributes(f, 2))
  expect_error(vapour_read_attributes(f, "sst_abc"))
  expect_error(vapour_read_names(f, 2))
  expect_error(vapour_read_names(f, "sst_abc"))
  expect_error(vapour_read_geometry(f, 2))
  expect_error(vapour_read_geometry(f, "sst_abc"))
  expect_error(vapour_read_geometry_text(f, 2))
  expect_error(vapour_read_geometry_text(f, "sst_abc"))
  expect_error(vapour_read_extent(f, 2))
  expect_error(vapour_read_extent(f, "sst_abc"))
  
  expect_error(index_layer("notafile.thatexist.anywhere.2323899933", "layername"), "cannot open data source")
  
  
  ex <- c(5235796, 12007371, -8823169, 9817684)
  #rext <- raster::extent(ex)
  a <- expect_warning(vapour_read_attributes(f, 0, extent = ex))
  #b <- expect_warning(vapour_read_attributes(f, 0, extent = rext))
  #expect_equal(a, b)
  # sql <- "SELECT * FROM sst_c"
  # expect_equal(vapour_read_attributes(f, 0, extent = ex, sql = sql), 
  #              vapour_read_attributes(f, 0, extent = rext, sql = sql))
  # expect_equal(vapour_read_names(f, 0, extent = ex, sql = sql), vapour_read_names(f, 0, extent = rext, sql = sql))
  # expect_equal(vapour_read_geometry(f, 0, extent = ex, sql = sql), vapour_read_geometry(f, 0, extent = rext, sql = sql))
  # # expect_equal(vapour_read_geometry_text(f, 0)
  # expect_equal(vapour_read_extent(f, 0)
  # 
})
