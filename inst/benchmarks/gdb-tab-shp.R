library(dplyr)
f <- raadfiles::thelist_files(format = "") %>% filter(grepl("parcel", fullname), grepl("shp$", fullname))
library(vapour)


x <- dplyr::bind_rows(x)
x[["wkb"]] <- lapply(f$fullname, read_gdal_geometry)








i <- 1
i <- i + 1
f1 <- f %>%  slice(i) %>% pull(fullname)
read_gdal_table(f1)


tibble::as_tibble(vapour:::vapour(f))
d <- sf::read_sf(f)
