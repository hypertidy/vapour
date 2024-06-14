context("test-features.R")


f <- system.file("extdata", "sst_c.fgb", package = "vapour", mustWork = TRUE)
f2 <- system.file("extdata/osm/myosmfile.osm", package = "vapour", mustWork = TRUE)
Sys.setenv(OSM_USE_CUSTOM_INDEXING="NO")
pfile <- "list_locality_postcode_meander_valley.tab"
dsource <- system.file(file.path("extdata/tab", pfile), package="vapour", mustWork = TRUE)

test_that("geometry read works", {

  gbin <- vapour_read_geometry(f)


  gjsn <- vapour_read_geometry_text(f, textformat = "json")
  ggml <- vapour_read_geometry_text(f, textformat = "gml")
 gwkt <- vapour_read_geometry_text(f, textformat = "wkt")
  gext <- vapour_read_extent(f)

  gbin %>% expect_length(7L)
  gjsn %>% expect_length(7L)
  ggml %>% expect_length(7L)

  gwkt %>% expect_length(7L)
  gext %>% expect_length(7L)


  gbin[[1]] %>% expect_type("raw")
  gjsn[[1]] %>% expect_type("character") %>% grepl("MultiLineString", .) %>% expect_true()
  ggml[[1]] %>% expect_type("character") %>% grepl("gml:MultiLineString", .) %>% expect_true()
 gwkt[[1]] %>% expect_type("character")   %>% grepl("MULTILINESTRING", .) %>% expect_true()
  

  expect_identical(gjsn, vapour_read_geometry_text(f))
  expect_identical(ggml, vapour_read_geometry_text(f, textformat = "gml"))
  expect_silent(expect_identical(gwkt, vapour_read_geometry_text(f, textformat = "wkt")))
  expect_identical(gext, vapour_read_extent(f))
  expect_identical(vapour_layer_names(dsource),
                   "list_locality_postcode_meander_valley")
  expect_error(vapour_read_extent(dsource, layer = "list_locality_postcode_meander_val"), "layer index not found for")
  ## no longer silent
  ## Warning 1: No translation from Polar_Stereographic to MapInfo known
  #Warning 6: Unhandled projection method Polar_Stereographic
  #expect_silent(vapour_read_attributes(dsource, layer = "list_locality_postcode_meander_valley"))
pprj <- "+proj=stere +lat_0=-90 +lat_ts=-71 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +no_defs"
pprj2 <- "+proj=stere +lat_0=-90 +lat_ts=-71 +lon_0=0 +x_0=0 +y_0=0 +datum=WGS84 +units=m +no_defs"

  expect_silent(vapour_geom_summary(f, layer = "sst_c"))
  expect_error(vapour_geom_summary(f, layer = "nolayer"))

  expect_silent(vapour_geom_summary(dsource, layer = 0))
  expect_silent(vapour_geom_summary(dsource, layer = "list_locality_postcode_meander_valley"))


  expect_warning(vapour_read_attributes(f, layer = factor("sst_c")), "layer is a factor, converting to character")

  expect_error(vapour_layer_names("nothing at all"), "Open failed")

  expect_error(vapour_read_attributes(f, layer = "not a layer"), "layer index not found for:")

})

test_that("OSM read works", {
  skip_on_os("windows")
  ## can't usse sst gpkg because
  ## kml can't be in a projection
  gkml <- vapour_read_geometry_text(f2, textformat = "kml")
  gkml %>% expect_length(1L)
  gkml[[1]] %>% expect_type("character")  %>% grepl("<Point><coordinates>", .) %>% expect_true()


  #gkml <- read_geometry_gdal_cpp(system.file("extdata", "point.shp", package = "vapour"),
  # what = "text", textformat = "kml")

})
test_that("empty geometry set as expected", {

  efile <- system.file("extdata/point.dbf", package = "vapour")
  emptygeomfile <- sprintf("%s.dbf", tempfile())
  file.copy(efile, emptygeomfile)
  empty <- vapour_geom_summary(emptygeomfile)
  expect_true(!any(empty$valid_geometry))
  file.remove(emptygeomfile)
})


test_that("limit_n works",
          {
            expect_silent(vapour_geom_summary(f, limit_n = 1L)) %>% unlist(use.names = FALSE) %>% expect_length(7L)

            expect_silent(vapour_geom_summary(dsource, limit_n = 1L)) %>% unlist(use.names = FALSE) %>% expect_length(7L)
            av_atts <- vapour_read_attributes(f, limit_n = 1) %>% expect_length(2L) %>% expect_named(c("level", "sst"))
            expect_silent(vapour_read_geometry(f, limit_n = 1L)) %>% expect_length(1L)
            expect_silent(vapour_read_geometry(f, limit_n = 1L, skip_n = 2)) %>% expect_length(1L)
            
            expect_silent(vapour_read_type(f, limit_n = 1L, skip_n = 2))  %>% expect_equal(5L)
            expect_silent(vapour_read_names(f, limit_n = 1L, skip_n = 2)) %>% expect_equal(2L)
            
            expect_silent(vapour_read_names(f)) %>% expect_length(7L)
            vapour_read_geometry_text(f, limit_n = 3L) %>% expect_length(3L)

            expect_named(vapour_layer_info(f), c("dsn", "driver", "layer", "layer_names", "fields", "count", 
                                                 "extent", "projection"))
            expect_silent(vapour_read_extent(f, limit_n = 3L)) %>% unlist(use.names = FALSE) %>% expect_length(12L)

            expect_error(vapour_read_attributes(f, limit_n = 5, skip_n = 7), "skip_n skips all available features")
          }


          )


test_that("extent clipping combined with sql works",
          {
            ex <- c(-5e6, 0, -1e8, -5e6)
            g1 <- vapour_geom_summary(f)
            expect_warning(g2 <- vapour_geom_summary(f, extent = ex))
            expect_identical(g1, g2)
            expect_length(g1$FID, 7L)
            expect_length(g1$valid_geometry, 7L)
            expect_length(g1$xmin, 7L)

            expect_silent(g3 <- vapour_geom_summary(f, sql = "SELECT * FROM sst_c", extent = ex))
            expect_length(g3$FID, 4L)
            expect_length(g3$valid_geometry, 4L)
            expect_length(g3$xmin, 4L)


            expect_silent(av1 <- vapour_read_attributes(f))
            expect_warning(av2 <- vapour_read_attributes(f, extent = ex))
            expect_silent(av3 <- vapour_read_attributes(f, extent = ex, sql = "SELECT * FROM sst_c"))
            expect_length(av3$level, 4L)
            expect_length(av3$sst, 4L)

            expect_identical(av1, av2)
            #vapour_read_geometry(f)
            expect_silent(gt1 <- vapour_read_geometry_text(f))
            expect_warning(gt2 <- vapour_read_geometry_text(f, extent = ex))
            expect_silent(gt3 <- vapour_read_geometry_text(f, extent = ex, sql = "SELECT *, sst, level FROM sst_c"))
            expect_identical(gt1, gt2)
            expect_length(gt3, 4L)


            expect_silent(e1 <- vapour_read_extent(f))
            expect_warning(e2 <- vapour_read_extent(f, extent = ex))
            expect_silent(e3 <- vapour_read_extent(f, sql = "select * FROM sst_c WHERE FID > -1", extent = ex))
          expect_identical(e1, e2)
          expect_length(e1, 7L)
          expect_length(e3, 4L)



sf_extent <- structure(setNames(ex[c(1, 3, 2, 4)], c("xmin", "ymin", "xmax", "ymax")), class = "bbox",
                       crs = structure(list(epsg = NA_integer_, proj4string = NA_character_), class = "crs"))
#r_extent <- raster::extent(ex)
sp_extent <- as.vector(t(matrix(ex, 2, byrow = TRUE, dimnames = list(c("s1", "s2"), c("min", "max")))))

expect_silent(s1 <- vapour_read_geometry_text(f))
expect_warning(s2 <- vapour_read_geometry_text(f, extent = sf_extent))
expect_silent(s3 <- vapour_read_geometry_text(f, extent = sf_extent, sql = "SELECT *, sst, level FROM sst_c"))
expect_identical(s1, s2)
## how to create this bogus raster extent without methods or raster? (save as data?)
# expect_warning(s2 <- vapour_read_geometry_text(f, extent = r_extent))
# expect_silent(s3 <- vapour_read_geometry_text(f, extent = r_extent, sql = "SELECT *, sst, level FROM sst_c"))
# expect_identical(s1, s2)
expect_warning(s2 <- vapour_read_geometry_text(f, extent = sp_extent))
expect_silent(s3 <- vapour_read_geometry_text(f, extent = sp_extent, sql = "SELECT *, sst, level FROM sst_c"))
expect_identical(s1, s2)

})
test_that("sanity prevails", {
  expect_error(vapour_layer_names(""), "Not a valid character string")
})

test_that("index geometry read works", {
          expect_length(vapour_read_geometry_ia(f, ia = c(0, 1)), 2L)
          expect_error(vapour_read_geometry_ia(f, ia = c(-1, 1)), "ia values < 0") 
          expect_error(vapour_read_geometry_ia(f, ia = c(NA, 1)), "missing values ia") 
          expect_silent(vapour_read_geometry_ia(f, ia = c(7))) %>% expect_equivalent(list(NULL)) 
          expect_length(vapour_read_geometry_ia(f, ia = c(7, 7)), 2L) 
          expect_length(vapour_read_geometry_ia(f, ia = c(6, 5)), 2L) 
          
          
          expect_length(vapour_read_geometry_ij(f, ij = c(0, 1)), 2L)
          expect_length(vapour_read_geometry_ij(f, ij = c(3, 5)), 3L)
          expect_error(vapour_read_geometry_ij(f, ij = c(-1, 1)), "ij values < 0") 
          expect_error(vapour_read_geometry_ij(f, ij = c(NA, 1)), "missing values ij") 
          expect_warning(vapour_read_geometry_ij(f, ij = c(6, 7))[2] %>% expect_equivalent(list(NULL)))
          
          expect_length(vapour_read_geometry_ij(f, ij = c(0, 6)), 7L)
          expect_error(vapour_read_geometry_ij(f, ij = c(5, 5)), "ij values must not be duplicated") 
          expect_error(vapour_read_geometry_ij(f, ij = c(5, 4)), "ij values must be increasing")         
                       
                                    
          }
)
          
