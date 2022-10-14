library(RNetCDF)
file.copy("../quarto-blog/data-raw/badnc/out.nc", "data-raw/badnc/bad_netcdf_geoloc_arrays.nc", overwrite = TRUE)
badnc <- "data-raw/badnc/bad_netcdf_geoloc_arrays.nc"
nc <- open.nc(badnc, write = TRUE )
lon <- var.get.nc(nc, "LON")
lat <- var.get.nc(nc, "LAT")
#topo <- raadtools::readtopo("etopo2")
v <- raster::extract(anglr::gebco, matrix(c(lon, lat), ncol = 2))
var.rename.nc(nc, "post_flux", "elevation")
a <- array(rep(v, 40), c(87, 77, 40))

var.put.nc(nc, "elevation", a)
att.put.nc(nc, "elevation", "units", value = "m", type = "NC_CHAR")
att.put.nc(nc, "NC_GLOBAL", "description","NC_CHAR", value = "Global topography values extracted from GEBCO 2021, for an illustration." )
att.put.nc(nc, "NC_GLOBAL", "institution", "NC_CHAR", "Australian Antarctic Division")
att.put.nc(nc, "NC_GLOBAL", "contact", "NC_CHAR", "Michael Sumner, michael.sumner@aad.gov.au")

print.nc(nc)
close.nc(nc)
#system("gdalwarp NETCDF:\"../quarto-blog/data-raw/badnc/out.nc\":elevation oo.tif")

image(angstroms::rawdata(badnc, "elevation")[,,1])


library(vapour)
sds <- vapour_sds_names(badnc)

lcc <-  "+proj=lcc +lat_0=-30 +lon_0=134.33 +lat_1=-50 +lat_2=-10 +R=6378137"

lon <- vapour_read_raster_dbl(sds[2], native = TRUE)
lat <- vapour_read_raster_dbl(sds[1], native = TRUE)
v <- vapour_read_raster_dbl(sds[3], native = TRUE)
xy <- reproj::reproj_xy(cbind(lon, lat), lcc, source = "OGC:CRS84")
plot(xy, pch = 19, cex = .5, col = palr::d_pal(v))
range(xy[,1])
range(xy[,2])
ex <- c(-3077504,  3968504, -2763621,  3472383) + c(-1, 1, -1, 1) * 1e7
vrt <- vapour_vrt(sds[3], geolocation = sds[2:1], bands = 1)
info <- vapour_raster_info(vrt)
ex <- c(-1, 1, -1, 1) * 1e7
prj <- "+proj=laea +lon_0=147 +lat_0=-20"
im <- vapour_warp_raster_dbl(vrt, extent = ex, dimension = info$dimension, projection = prj, transformation_options = "SRC_METHOD=GEOLOC_ARRAY")

                             #, transformation_options = c("SRC_METHOD=NO_GEOTRANSFORM"))
ximage::ximage(matrix(im, info$dimension[2L], byrow = TRUE), extent = ex)

ex <- c(90, 190, range(lat))
dm <- c(512, 512)
im <- vapour_warp_raster_dbl(vrt, extent = ex, dimension = dm, projection = "OGC:CRS84", warp_options = c("SAMPLE_GRID=YES", "SOURCE_EXTRA=20"))
                             #, transformation_options = c("SRC_METHOD=NO_GEOTRANSFORM"))
ximage::ximage(matrix(im, dm[2L], byrow = TRUE), extent = ex)

