ant <- sfdct::antarctica
#gibble::gibble(ant) %>% mutate(n = row_number()) %>% arrange(desc(nrow))

antcoast <- rgdal::project(ant$geometry[[1]][[97]][[1]], attr(ant$geometry, "crs")$proj4string, inv = TRUE)
usethis::use_data(antcoast, internal = TRUE)
