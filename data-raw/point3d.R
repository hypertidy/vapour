## 3d
pt <- sf::st_sf(a = 1, geometry = sf::st_sfc(sf::st_point(cbind(0, 0, 0))))

# sf::write_sf(pt, "inst/extdata/point3d.tab",
#              driver = "MapInfo File")
sf::write_sf(pt, "inst/extdata/point3d.gpkg",
             driver = "GPKG")



