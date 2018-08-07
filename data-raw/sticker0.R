library(hexSticker)

library(raster)
r <- crop(brick("data-raw/IMG_20180614_152819_HDR.jpg"),
          extent(2000, 3200, 950, 1600))

extent(r) <- extent(0, 1, 0, 1)
sticker(expression(plotRGB(r)),
        package="vapour", p_size=8, s_x=1, s_y=.8, s_width=1.2, s_height=1,
        filename="data-raw/vapour-hex.png")
