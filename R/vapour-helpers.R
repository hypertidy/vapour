## helpers

vapour_geom_summary <- function(dsource, layer = 0L, sql = "") {
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  extents <- vapour_read_extent(dsource, layer, sql)
  na_geoms <- unlist(lapply(extents, anyNA))
  list(geometry = !na_geoms,
       xmin = unlist(lapply(extents, "[", 1L)),
       xmax = unlist(lapply(extents, "[", 2L)),
       ymin = unlist(lapply(extents, "[", 3L)),
       ymax = unlist(lapply(extents, "[", 4L)))
}
