#' Functions deprecated from vapour
#'
#' These will be removed in a future release.
#' @name vapour-deprecated
#' @inheritParams vapour_read_geometry
#' @rdname vapour-deprecated
#' @export
vapour_read_geometry_what <- function(dsource, layer = 0L, sql = "", what = "geometry", textformat = "") {
  .Deprecated("vapour_read_geometry_cpp")
  vapour_read_geometry_cpp(dsource = dsource, layer = layer, sql = sql, what = what, textformat  = textformat)
}
#' @rdname vapour-deprecated
#' @export
#' @seealso vapour_read_geometry_cpp
vapour_read_feature_what <- function(dsource, layer = 0L, sql = "", what = "geometry", textformat = "json") {
  .Deprecated("vapour_read_geometry_cpp")
  vapour_read_geometry_cpp(dsource = dsource, layer = layer, sql = sql, what = what, textformat = textformat)
}

#' @rdname vapour-deprecated
#' @param x data source name
#' @seealso raster_sds_info
#' @export
sds_info <- function(x) {
  .Deprecated("raster_sds_info")
  raster_sds_info(x)
}


