f <- "/rdsi/PUBLIC/raad/data/www.ngdc.noaa.gov/mgg/global/relief/ETOPO2/ETOPO2v2-2006/ETOPO2v2c/netCDF/ETOPO2v2c_f4.nc"
f <- system.file("extdata", "sst.tif", package = "vapour")


par(mfrow = c(2, 2))







zipurl <- "https://www.ngdc.noaa.gov/mgg/global/relief/ETOPO2/ETOPO2v2-2006/ETOPO2v2c/netCDF/ETOPO2v2c_f4_netCDF.zip"
vsiurl <- file.path("/vsizip//vsicurl", zipurl)
(u <- file.path(vsiurl, 
               vapour_vsi_list(vsiurl)[1L]))

## neither gets the extent (because the file is bad)
stars::read_stars(u)
terra::rast(u)

## but we only need four numbers and a string
source_ex <- c(-180, 180, -90, 90)
source_proj <- "EPSG:4326"

## choose any grid, literally an extent, dimension, projection (this just a concise way to centre an example)
b <- 4e5
dm <- c(512, 512)
prj <- "+proj=aeqd +lon_0=147 +lat_0=-42"
v <- vapour_warp_raster(f, extent = c(-b, b, -b, b),
                           dimension = dm,
                           bands = 1, 
                        ## we have to *augment* the source, because the tools don't know the extent or its projection)
                        source_extent = source_ex, 
                        source_wkt = source_proj,
                           projection = prj, band_output_type = "vrt")

## put the filename back in (will hide this step soon ..)
vrt <- gsub("\"0\"></SourceDataset>", sprintf("\"0\">%s</SourceDataset>", f), v[[1]])

## and we good
terra::plot(tr <- terra::rast(vrt))
tr
library(stars)
image(st <- stars::read_stars(vrt, proxy = F, normalize_path = FALSE), col = grey.colors(256))
axis(1);axis(2)
st


## what if we want to match the source? that works too!
v <- vapour_warp_raster(f, extent = c(140, 154, -49, -35),
                        dimension = dm,
                        bands = 1, 
                        ## we have to *augment* the source, because the tools don't know the extent or its projection)
                        source_extent = source_ex, 
                        source_wkt = source_proj,
                        projection = "OGC:CRS84", band_output_type = "vrt")

## put the filename back in (will hide this step soon ..)
vrt <- gsub("\"0\"></SourceDataset>", sprintf("\"0\">%s</SourceDataset>", f), v[[1]])
## and we good
terra::plot(tr <- terra::rast(vrt))
image(st <- stars::read_stars(vrt, proxy = F, normalize_path = FALSE), col = grey.colors(256))
axis(1);axis(2)





library(vapour)
zipurl <- "https://www.ngdc.noaa.gov/mgg/global/relief/ETOPO2/ETOPO2v2-2006/ETOPO2v2c/netCDF/ETOPO2v2c_f4_netCDF.zip"
vsiurl <- file.path("/vsizip//vsicurl", zipurl)
(u <- file.path(vsiurl, 
                vapour_vsi_list(vsiurl)[1L]))
## helper fun
rinfo <- function(x) setNames(vapour_raster_info(x)[c("extent", "dimXY", "projection")], c("extent", "dimension", "projection"))
rinfo(u)

## relies on dev build of GDAL, with local patch to vrt:// connection support (adding a_ullr and a_srs)
u_augment <- sprintf("vrt://%s?a_ullr=-180 90 180 -90&a_srs=OGC:4326", u)
u_augment

rinfo(u_augment)

