f <- "~/albania-latest.osm.pbf"
vapour_layer_names(f)


td <- "~"
u = "http://download.geofabrik.de/europe/albania-latest.osm.pbf"
f <- normalizePath(file.path(td, basename(u)))
#download.file(u, f, mode = "wb")
g <- vapour::vapour_read_geometry(f, 0)
d <- vapour::vapour_read_attributes(f, layer = 0)
x <- sf::read_sf(f, "points")

# Driver: OSM
# Available layers:
#   layer_name       geometry_type         features fields
# 1           points               Point     56653     10
# 2            lines         Line String    83161      9
# 3 multilinestrings   Multi Line String      127      4
# 4    multipolygons       Multi Polygon    112854     25
# 5  other_relations Geometry Collection    713        4


## the multipolygons layer has 112854 features and 25 fields, so

d <- vapour::vapour_read_attributes(f,  sql = "SELECT osm_id, type FROM multipolygons")
g <- vapour::vapour_read_extent(f, sql = "SELECT osm_id FROM multipolygons WHERE type LIKE 'City_municipality'")

d <- vapour::vapour_read_attributes(f,  sql = "SELECT * FROM multilinestrings WHERE type LIKE 'route'")
g <- vapour::vapour_read_geometry(f, sql = "SELECT * FROM multilinestrings WHERE type LIKE 'route'")

#f <- list.files(basename(tempdir()), recursive = TRUE, pattern = "pbf$")
read_pbf_geometry <- function(file) {
  layers <- sf::st_layers(file)$name
  setNames(purrr::map(layers,
                      ~sf::read_sf(file, .x)), layers)
}
x <- read_pbf_geometry(f)

purrr::map_int(x, nrow)
purrr::map_int(x, ncol)



download.file(u, f, mode = "wb")
msg = "osmconvert albania.osh.pbf >albania.osm")
system(msg)
osmdata_xml(input_file = "albania.osh.pbf", filename = "albania.osm")
q = add_feature(opq("Albania"), "highway")
albanian_roads = osmdata_sf(q, doc = "albania.osm")
