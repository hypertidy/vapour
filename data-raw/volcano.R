library(raster)
v <- setExtent(raster(volcano), extent(0, ncol(volcano), 0, nrow(volcano)))
writeRaster(v, "inst/extdata/volcano.tif")

file.copy("inst/extdata/volcano.tif", "inst/extdata/volcano_overview.tif")
system("gdaladdo inst/extdata/volcano_overview.tif -minsize 4 2 4 8 16")

info <- vapour_raster_info("inst/extdata/volcano.tif")
v <- vapour_read_raster_int("inst/extdata/volcano.tif", native = TRUE)

acol <- aperm(array(col2rgb(palr::image_pal(v, hcl.colors(26))), 
                    c(3, info$dimension)), c(2, 3, 1))
#ximage::ximage(acol)

png::writePNG(acol/255, "inst/extdata/volcano.png")
## why ximage
# ## have to set up an extent
# ex <-c(0, info$dimension[1], 0, info$dimension[2]) 
# ## and set up a plot with it
# plot(ex[1:2], ex[3:4], type = "n")
# aperm(array(col2rgb(palr::image_pal(v, hcl.colors(26))), 
#             c(3, info$dimension)), c(2, 3, 1)) |> 
#   ## scale to 0,1
#   scales::rescale() |> 
#   ## finally rasterImage() draws the pixels
#   rasterImage(image = _, ex[1], ex[3], ex[2], ex[4])
# 
