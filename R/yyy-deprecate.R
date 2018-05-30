#' Functions deprecated from vapour
#'
#' These will be removed in a future release.
#' @name vapour-deprecated
#' @inheritParams vapour_read_geometry
#' @rdname vapour-deprecated
#' @export
vapour_read_geometry_what <- function(dsource, layer = 0L, sql = "", what = "geometry", textformat = "") {
  .Defunct("vapour_read_geometry")
  #vapour_read_geometry_cpp(dsource = dsource, layer = layer, sql = sql, what = what, textformat  = textformat)
}
#' @rdname vapour-deprecated
#' @export
#' @seealso vapour_read_geometry_cpp
vapour_read_feature_what <- function(dsource, layer = 0L, sql = "", what = "geometry", textformat = "json") {
  .Defunct("vapour_read_geometry")
  #vapour_read_geometry_cpp(dsource = dsource, layer = layer, sql = sql, what = what, textformat = textformat)
}
#' @rdname vapour-deprecated
#' @export
#' @seealso vapour_read_geometry_cpp
vapour_read_geometry_cpp <- function(dsource, layer = 0L, sql = "", what = "geometry", textformat = "json") {
  .Deprecated("vapour_read_geometry")
  vapour_read_geometry_cpp(dsource = dsource, layer = layer, sql = sql, what = what, textformat = textformat)
}


#' @rdname vapour-deprecated
#' @param x data source name
#' @seealso vapour_sds_names
vapour_sds_names(x)
#' @export
sds_info <- function(x) {
  .Deprecated("vapour_sds_names")
  vapour_sds_names(x)
}

#' @rdname vapour-deprecated
#' @seealso vapour_sds_names
raster_sds_info <- function(x) {
  .Deprecated("vapour_sds_names")
  vapour_sds_names(x)
}
