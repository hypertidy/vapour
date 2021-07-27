



## IBCSO GeoTIFF  hs.pangaea.de/Maps/bathy/IBCSO_v1/ibcso_v1_is.tif
f <- "/vsizip/vsicurl/https://hs.pangaea.de/Maps/bathy/IBCSO_v1/IBCSO_v1_bed_PS65_500m_tif.zip"
# vapour_vsi_list(f)
# [1] "ibcso_v1_bed.tfw"         "ibcso_v1_bed.tif"         "ibcso_v1_bed.tif.aux.xml" "ibcso_v1_bed.tif.ovr"
# [5] "ibcso_v1_bed.tif.xml"

library(vapour)
vsiurl <- file.path(f, vapour_vsi_list(f)[2])

f <- raadtools::topofile("etopo2")
dm <- as.integer(c(512, 512))
b <- 4e6




prj <- "+proj=laea +lon_0=180"
ex <- c(-180 * 60 * 1852, 180 * 60 * 1852, -90 * 60 * 1852, 90 * 60 * 1852)

dm <- c(1024, 512)
v <- vapour_warp_raster(f, band = 1,
                                 extent = ex,
                                 dimension = dm,
                                 wkt = vapour_srs_wkt(prj),
                        source_wkt = vapour_srs_wkt("+proj=longlat"),
                              resample = "bilinear")

image(
  list(x = seq(-b, b, length.out = dm[1] + 1), y = seq(-b, b, length.out = dm[2] + 1),
       z = matrix(unlist(v, use.names = FALSE), dm[1])[,dm[2]:1]),
  asp = 1, col = hcl.colors(256))
