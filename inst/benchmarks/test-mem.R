file <- "list_locality_postcode_meander_valley.tab"
mfile <- system.file(file.path("extdata/tab", file), package="vapour")
pfile <- system.file("extdata/point.shp", package = "vapour")
#tfile <- "\\\\aad.gov.au/files/AADC/Scientific_Data/Data/gridded_new/data_local/tas.gov.au/TASVEG/GDB/TASVEG3.gdb"
tfile <- "//rdsi/PRIVATE/raad/data_local/tas.gov.au/TASVEG/GDB/TASVEG3.gdb"


vapour_projection_info_cpp(mfile)
vapour_projection_info_cpp(tfile)
vapour_projection_info_cpp(pfile)

bfile <- "/rdsi/PRIVATE/raad/data_local/protectedplanet.net/WDPA_Aug2017-shapefile-polygons.shp"
n <- vapour_read_names(bfile)
#Warning 1: organizePolygons() received an unexpected geometry.  Either a polygon with interior rings, or a polygon with less than 4 points, or a non-Polygon geometry.  Return arguments as a collection.
#> str(n)
#num [1:215375] 0 1 2 3 4 5 6 7 8 9 ...
e <- vapour_geom_summary(bfile)

for (i in 1:1e6) {
  lim <- sample(1e6)
  x <- vapour_read_attributes(mfile, limit_n = lim)
  x <- vapour_read_attributes(pfile, limit_n = lim)
  x <- vapour_read_attributes(tfile, limit_n = lim)

  x <- vapour_read_geometry(mfile, limit_n = lim)
  x <- vapour_read_geometry(pfile, limit_n = lim)
  x <- vapour_read_geometry(tfile, limit_n = lim)


  x <- vapour_read_geometry_text(mfile, limit_n = lim)
  x <- vapour_read_geometry_text(pfile, limit_n = lim)
  x <- vapour_read_geometry_text(tfile, limit_n = lim)

  x <- vapour_geom_summary(mfile, limit_n = lim)
  x <- vapour_geom_summary(pfile, limit_n = lim)
  x <- vapour_geom_summary(tfile, limit_n = lim)


}
