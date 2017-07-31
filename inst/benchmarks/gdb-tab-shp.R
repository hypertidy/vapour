library(dplyr)
f <- raadfiles::thelist_files(format = "") %>% filter(grepl("parcel", fullname), grepl("shp$", fullname))
library(vapour)
library(blob)
system.time(purrr::map(f$fullname, sf::read_sf))
# user  system elapsed
# 43.124   2.857  39.386


system.time({
d <- purrr::map(f$fullname, read_gdal_table)
d <- dplyr::bind_rows(d)
g <- purrr::map(f$fullname, read_gdal_geometry)
d[["wkb"]] <- new_blob(unlist(g, recursive = FALSE))
})
# user  system elapsed
# 16.400   2.882  23.227

pryr::object_size(d)
## 359 MB

