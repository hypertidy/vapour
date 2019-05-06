system("free -h")
total        used        free      shared  buff/cache   available
Mem:            47G        3.6G         14G        4.7M         28G         42G
Swap:            0B          0B          0B
> for (i in 1:100) a <- vapour_read_attributes(f)
> system("free -h")
total        used        free      shared  buff/cache   available
Mem:            47G        5.6G         13G        4.7M         28G         41G
Swap:            0B          0B          0B
>

  findf <- function() {
    setNames(as.numeric(strsplit(system("free -m", intern = TRUE)[2], "\\s+", perl = TRUE)[[1]][-1]),
             c("total", "used", "free", "shared", "buff", "avail"))
  }

f<- "/rdsi/PUBLIC/raad/data/listdata.thelist.tas.gov.au/opendata/data/list_parcels_break_o_day.gdb"
gf <- "../rasterwise/extdata/gcp/NSS.GHRR.NH.D94241.S1549.E1736.B3055960.GC"


d <- vapour_read_attributes(f)
a <- findf()
for (i in 1:100) d <- vapour_read_attributes(f)
b <- findf()
rbind(a, b)


d <- vapour_read_extent(f)
a <- findf()
for (i in 1:1000) d <- vapour_read_extent(f)
b <- findf()
rbind(a, b)


d <- vapour_driver(f)
a <- findf()
for (i in 1:5000) d <- vapour_driver(f)
b <- findf()
rbind(a, b)


d <- vapour_gdal_version()
a <- findf()
for (i in 1:250000) d <- vapour_gdal_version()
b <- findf()
rbind(a, b)



d <- vapour_geom_summary(f)
a <- findf()
for (i in 1:100) d <- vapour_geom_summary(f)
b <- findf()
rbind(a, b)



d <- vapour_layer_names(f)
a <- findf()
for (i in 1:1000) d <- vapour_layer_names(f)
b <- findf()
rbind(a, b)



d <- vapour_raster_gcp(gf)
a <- findf()
for (i in 1:1000) d <- vapour_raster_gcp(gf)
b <- findf()
rbind(a, b)


d <- vapour_raster_info(gf)
a <- findf()
for (i in 1:1000) d <- vapour_raster_info(gf)
b <- findf()
rbind(a, b)


d <- vapour_read_raster(gf, window = c(0, 0, 40, 50, 80, 70))
a <- findf()
for (i in 1:1000) d <- vapour_read_raster(gf, window = c(0, 0, 40, 50, 80, 70))
b <- findf()
rbind(a, b)


d <- vapour_read_geometry(f)
a <- findf()
for (i in 1:10) d <- vapour_read_geometry(f)
b <- findf()
rbind(a, b)

d <- vapour_read_geometry_text(f)
a <- findf()
for (i in 1:10) d <- vapour_read_geometry_text(f)
b <- findf()
rbind(a, b)

d <- vapour_read_names(f)
a <- findf()
for (i in 1:100) d <- vapour_read_names(f)
b <- findf()
rbind(a, b)






system("free -h")
for (i in 1:10000) a <- vapour_all_drivers()
system("free -h")


