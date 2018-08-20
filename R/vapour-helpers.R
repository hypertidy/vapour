## helpers

geom_summary <- function(dsource, layer = 0L, sql = "") {
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  extents <- vapour_read_extent(dsource, layer, sql)
  # if (nchar(sql) > 1) {
  #   warning("input SQL used to summarize attributes, leave as \"\" to apply default efficient LIMIT")
  # } else {
  #   layername <- vapour_layer_names(dsource, sql = sql)[layer + 1L]
  #   sql <- sprintf("SELECT * FROM %s LIMIT 1", layername)
  # }
  # attrs  <- vapour_read_attributes(dsource, layer, sql)
  na_geoms <- unlist(lapply(extents, anyNA))
  list(geometry = !na_geoms,
       xmin = unlist(lapply(extents, "[", 1L)),
       xmax = unlist(lapply(extents, "[", 2L)),
       ymin = unlist(lapply(extents, "[", 3L)),
       ymax = unlist(lapply(extents, "[", 4L)))
}
