#-gcp pixel line easting northing elevation:
#  Add the indicated ground control point to the output dataset. This option may be provided multiple times to provide a set of GCPs.

f <- "inst/extdata/gcps/volcano_gcp.tif"
tfile <- tempfile(fileext = ".tif")
raster::writeRaster(raster::raster(volcano[1:15, 1:20], xmn = 0, xmx = 20, ymn = 0, ymx = 15), tfile)
system(sprintf("gdal_translate %s %s -gcp 0 0 100 100 -gcp 5 5 200 200 -gcp 20 15 300 300", tfile, f))
