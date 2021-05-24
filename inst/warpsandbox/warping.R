
#f <- vapour::vapour_sds_names( raadtools::sstfiles()$fullname[1])$subdataset[1]
f <- "NETCDF:\"/rdsi/PUBLIC/raad/data/www.ncei.noaa.gov/data/sea-surface-temperature-optimum-interpolation/v2.1/access/avhrr/198109/oisst-avhrr-v02r01.19810901.nc\":sst"
#f <- system.file("extdata/sst.tif", package = "vapour", mustWork = TRUE)


# library(raadtools)
# f <- topofile("ibcso")
# ex <- extent(0, 1e6, 0, 1e6)
# eg <- crop(raster::raster(f), ex)

dm <- as.integer(c(1024L, 1024L)/3)
#dm <- dim(eg)[2:1]

b <- 3e6
ex <- raster::extent(-b, b, -b, b)
prj <- "+proj=laea"
prj <- "+proj=lcc +lon_0=145 +lat_0=-47 +lat_1=-42 +lat_2=-48"

v <- vapour:::vapour_warp_raster(f, band = 1,
                                 geotransform = affinity::extent_dim_to_gt(ex, dm),
                                 dimension = dm,
                                 wkt = vapour_srs_wkt(prj),
                                 source_wkt = vapour_srs_wkt("+proj=longlat"),
                                 resample = "bilinear")



library(raster)
rr <- raster(ex, nrows = dm[2L], ncols = dm[1L], crs = prj)
#d0 <- function(x) {x[!x > 0] <- NA; x}
plot(setValues(rr, unlist(v)), col = hcl.colors(256))


















library(gdalwebsrv)

available_sources()
#> [1] "wms_arcgis_mapserver_tms" "wms_bluemarble_s3_tms"
#> [3] "wms_googlemaps_tms"       "wms_openstreetmap_tms"
#> [5] "wms_virtualearth"

srcfile <- server_file("wms_bluemarble_s3_tms")
system.time({
dm <- c(1024L, 1024L)
ex <- raster::extent(-4e6, 4e6, -4e6, 4e6)
prj <- "+proj=laea +lon_0=147 +lat_0=-42"
v <- vapour:::vapour_warp_raster(srcfile, band = 1:3,
                                 geotransform = affinity::extent_dim_to_gt(ex, dm),
                                 dimension = dm,
                                 wkt = vapour:::proj_to_wkt_gdal_cpp(prj),
                                 resample = "bilinear")

})


library(raster)
rr <- raster(ex, nrows = dm[2L], ncols = dm[1L])
plotRGB(setValues(brick(rr, rr, rr), matrix(unlist(v), prod(dm))))







#gdalwarp -te -4e6 -4e6 4e6 4e6 -tr 25000 25000 -t_srs "+proj=laea +lon_0=147 +lat_0=-42" -s_srs "+proj=longlat"  $sst out.tif


f <- "NETCDF:/rdsi/PUBLIC/raad/data/www.ncei.noaa.gov/data/sea-surface-temperature-optimum-interpolation/v2.1/access/avhrr/200901/oisst-avhrr-v02r01.20090116.nc:sst"
#f <- "/rdsi/PUBLIC/raad/data/www.ncei.noaa.gov/data/sea-surface-temperature-optimum-interpolation/v2.1/access/avhrr/200901/oisst-avhrr-v02r01.20090116.nc"
#f <- normalizePath("out.vrt")
#f <- raadtools::topofile("ibcso")
f <- "/rdsi/PUBLIC/raad/data/hs.pangaea.de/Maps/bathy/IBCSO_v1/ibcso_v1_is.tif"
src_geotransform <- vapour_raster_info(f)$geotransform
dm <- c(320L, 320L)
prj <- "+proj=laea +lon_0=147 +lat_0=-90"

v <- vapour_warp_raster(f, band = 1L,
                        geotransform = affinity::extent_dim_to_gt(raster::extent(-4e6, 4e6, -4e6, 4e6), dm),
                        dimension = dm, wkt = vapour:::proj_to_wkt_gdal_cpp(prj),
                        #source_wkt = vapour:::proj_to_wkt_gdal_cpp("+proj=longlat"),
                        #source_geotransform = src_geotransform* c(1, 1, 1, 1, 1, 1),
                        resample = "bilinear")



library(raster)
plot(setValues(raster(extent(-4e6, 4e6, -4e6, 4e6), nrows = 320, ncols = 320), v[[1]]))






gt <- c(-4000000 ,   25000,        0,  4000000  ,      0,   -25000 )

f <- system.file("extdata", "sst.tif", package = "vapour")
vals <- vapour:::vapour_warp_raster(f, geotransform = gt,
                             dimension = c(186, 298),
                             wkt = vapour:::proj_to_wkt_gdal_cpp(prj))

