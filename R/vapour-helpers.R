## helpers

## TODO
## add limit_n, good example to ensure it works for extent, names, and check in combo with SQL


vapour_geom_summary <- function(dsource, layer = 0L, sql = "") {
  if (!is.numeric(layer)) layer <- index_layer(x = dsource, layername = layer)
  extents <- vapour_read_extent(dsource = dsource, layer = layer, sql = sql)
  fids <- vapour_read_names(dsource = dsource, layer = layer, sql = sql)
  na_geoms <- unlist(lapply(extents, anyNA))
  list(FID = fids,
       valid_geometry = !na_geoms,
       xmin = unlist(lapply(extents, "[", 1L)),
       xmax = unlist(lapply(extents, "[", 2L)),
       ymin = unlist(lapply(extents, "[", 3L)),
       ymax = unlist(lapply(extents, "[", 4L)))
}
