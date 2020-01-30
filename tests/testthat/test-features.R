context("test-features.R")


f <- system.file("extdata", "sst_c.gpkg", package = "vapour")
f2 <- system.file("extdata/osm/myosmfile.osm", package = "vapour", mustWork = TRUE)
Sys.setenv(OSM_USE_CUSTOM_INDEXING="NO")
pfile <- "list_locality_postcode_meander_valley.tab"
dsource <- system.file(file.path("extdata/tab", pfile), package="vapour")

test_that("geometry read works", {

  gbin <- vapour_read_geometry_cpp(f, what = "geometry")
  gpt <- vapour_read_geometry_cpp(f, what = "point")
  gpt1 <- vapour_read_geometry_cpp(f, what = "point", skip_n = 3, limit_n = 1)

  gjsn <- vapour_read_geometry_cpp(f, what = "text", textformat = "json")
  ggml <- vapour_read_geometry_cpp(f, what = "text", textformat = "gml")
 gwkt <- vapour_read_geometry_cpp(f, what = "text", textformat = "wkt")
  gext <- vapour_read_geometry_cpp(f, what = "extent")

  gbin %>% expect_length(7L)
  gjsn %>% expect_length(7L)
  ggml %>% expect_length(7L)

  gwkt %>% expect_length(7L)
  gext %>% expect_length(7L)
  gpt %>% expect_length(7L)
  gpt1 %>% expect_length(1L)

  expect_equivalent(round(gpt1[[1]][[2]][[1]], digits = 2),
                    c(-305926.42, -306449.2, -306959.29, -306557.76, -305926.42))


  gbin[[1]] %>% expect_type("raw")
  gjsn[[1]] %>% expect_type("character") %>% grepl("MultiLineString", .) %>% expect_true()
  ggml[[1]] %>% expect_type("character") %>% grepl("gml:MultiLineString", .) %>% expect_true()
 gwkt[[1]] %>% expect_type("character")   %>% grepl("MULTILINESTRING \\(\\(-16254", .) %>% expect_true()
  gext[[4]] %>% expect_type("double")  %>% trunc() %>% expect_identical(c(-9293382, 7088338, -6881739, 9067994))

  expect_identical(gjsn, vapour_read_geometry_text(f))
  expect_identical(ggml, vapour_read_geometry_text(f, textformat = "gml"))
  expect_silent(expect_identical(gwkt, vapour_read_geometry_text(f, textformat = "wkt")))
  expect_identical(gext, vapour_read_extent(f))
  expect_identical(vapour_layer_names(dsource,
                   sql = "SELECT 1 FROM list_locality_postcode_meander_valley"),
                   "list_locality_postcode_meander_valley")
  expect_error(vapour_read_extent(dsource, layer = "list_locality_postcode_meander_val"), "layer index not found for")
  ## no longer silent
  ## Warning 1: No translation from Polar_Stereographic to MapInfo known
  #Warning 6: Unhandled projection method Polar_Stereographic
  #expect_silent(vapour_read_attributes(dsource, layer = "list_locality_postcode_meander_valley"))
pprj <- "+proj=stere +lat_0=-90 +lat_ts=-71 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +no_defs"
pprj2 <- "+proj=stere +lat_0=-90 +lat_ts=-71 +lon_0=0 +x_0=0 +y_0=0 +datum=WGS84 +units=m +no_defs"
expect_warning(src0 <- trimws(vapour_projection_info_cpp(f)$Proj4[1]), "not null")
expect_true(src0 == pprj || src0 == pprj2 )

  expect_silent(vapour_geom_summary(f, layer = "sst_c"))
  expect_error(vapour_geom_summary(f, layer = "nolayer"))

  expect_silent(vapour_geom_summary(dsource, layer = 0))
  expect_silent(vapour_geom_summary(dsource, layer = "list_locality_postcode_meander_valley"))

  p3d <- system.file("extdata/point3d.gpkg", package = "vapour")
  ## attachPoints
  expect_equal(vapour_read_geometry_cpp(p3d, what = "point")[[1]],
               list(x = 0, y = 0, z = 0))

  expect_warning(vapour_read_attributes(f, layer = factor("sst_c")), "layer is a factor, converting to character")

  expect_error(vapour_layer_names("nothing at all"), "Open failed")

  expect_error(vapour_read_attributes(f, layer = "not a layer"), "layer index not found for:")

})

test_that("OSM read works", {
  skip_on_os("windows")
  ## can't usse sst gpkg because
  ## kml can't be in a projection
  expect_silent(gkml <- vapour_read_geometry_cpp(f2, what = "text", textformat = "kml"))
  gkml %>% expect_length(1L)
  gkml[[1]] %>% expect_type("character")  %>% grepl("<Point><coordinates>", .) %>% expect_true()

  expect_identical(gkml, vapour_read_geometry_text(f2, textformat = "kml"))

  #gkml <- vapour_read_geometry_cpp(system.file("extdata", "point.shp", package = "vapour"),
  # what = "text", textformat = "kml")

})
test_that("empty geometry set as expected", {

  efile <- system.file("extdata/point.dbf", package = "vapour")
  emptygeomfile <- sprintf("%s.dbf", tempfile())
  file.copy(efile, emptygeomfile)
  empty <- vapour_geom_summary(emptygeomfile)
  expect_true(!any(empty$valid_geometry))
  unlink(emptygeomfile)
})


test_that("limit_n works",
          {
            expect_silent(vapour_geom_summary(f, limit_n = 1L)) %>% unlist() %>% expect_length(7L)

            expect_silent(vapour_geom_summary(dsource, limit_n = 1L)) %>% unlist() %>% expect_length(7L)
            av_atts <- vapour_read_attributes(f, limit_n = 1) %>% expect_length(2L) %>% expect_named(c("level", "sst"))
            expect_silent(vapour_read_geometry(f, limit_n = 1L)) %>% expect_length(1L)

            expect_silent(vapour_read_geometry_text(f, limit_n = 3L)) %>% expect_length(3L)

            expect_silent(vapour_read_extent(f, limit_n = 3L)) %>% unlist() %>% expect_length(12L)

            expect_error(vapour_read_attributes(f, limit_n = 5, skip_n = 7), "is 'skip_n' set too high?")
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

          expect_equal(e1, list(c(-3273103.52164666, 4672692.82549952, -3562748.06089841,
                                  4870090.32178965), c(-4774642.02701368, 5167013.90124751, -4644892.46962219,
                                                       5303185.55727377), c(-6005059.83508923, 6080358.63750109, -5670292.85588759,
                                                                            6963437.49419418), c(-9293382.13554168, 7088338.80241629, -6881739.64161586,
                                                                                                 9067994.12361849), c(-12306571.6084815, 8614711.87011407, -8114743.77423273,
                                                                                                                      11869446.3939912), c(-9403103.938376, 10886059.5102222, -11285790.2140821,
                                                                                                                                           10798224.9521274), c(5106987.14577475, 5107839.31687039, -11065121.4051305,
                                                                                                                                                                -11063443.5304944)))
expect_equal(e3, list(c(-6005059.83508923, 6080358.63750109, -5670292.85588759,
                        6963437.49419418), c(-9293382.13554168, 7088338.80241629, -6881739.64161586,
                                             9067994.12361849), c(-12306571.6084815, 8614711.87011407, -8114743.77423273,
                                                                  11869446.3939912), c(-9403103.938376, 10886059.5102222, -11285790.2140821,
                                                                                       10798224.9521274))
             )




sf_extent <- structure(setNames(ex[c(1, 3, 2, 4)], c("xmin", "ymin", "xmax", "ymax")), class = "bbox",
                       crs = structure(list(epsg = NA_integer_, proj4string = NA_character_), class = "crs"))
#r_extent <- raster::extent(ex)
sp_extent <- matrix(ex, 2, byrow = TRUE, dimnames = list(c("s1", "s2"), c("min", "max")))

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
  expect_error(vapour_layer_names(""), "Open failed.")
})

