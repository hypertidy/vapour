
#' Read GDAL feature geometry
#'
#' Read GDAL geometry as binary blob, text, or numeric extent.
#'
#' `vapour_read_geometry` will read features as binary WKB, `vapour_read_geometry_text` as various text formats (geo-json, wkt, kml, gml),
#' `vapour_read_extent` a numeric extent which is the native bounding box, the four numbers (in this order) `xmin, xmax, ymin, ymax`.
#' For each function an optional SQL string will be evaluated against the data source before reading.
#'
#' `vapour_read_geometry_cpp` will read a feature for each of the ways listed above and is used by those functions. It's recommended
#' to use the more specialist functions rather than this more general one.
#'
#' Note that `limit_n` and `skip_n` interact with the affect of `sql`, first the query is executed on the data source, then
#' while looping through available features `skip_n` features are ignored, and then a feature-count begins and the loop
#' is stopped if `limit_n` is reached.
#'
#' Note that `extent` applies to the 'SpatialFilter' of 'ExecuteSQL': https://gdal.org/user/ogr_sql_dialect.html#executesql.
#' @param dsource data source name (path to file, connection string, URL)
#' @param layer integer of layer to work with, defaults to the first (0) or the name of the layer
#' @param sql if not empty this is executed against the data source (layer will be ignored)
#' @param textformat indicate text output format, available are "json" (default), "gml", "kml", "wkt"
#' @param limit_n an arbitrary limit to the number of features scanned
#' @param skip_n an arbitrary number of features to skip
#' @param extent apply an arbitrary extent, only when 'sql' used (must be 'ex = c(xmin, xmax, ymin, ymax)' but sp bbox, sf bbox, and raster extent also accepted)
#' @examples
#' file <- "list_locality_postcode_meander_valley.tab"
#' ## A MapInfo TAB file with polygons
#' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
#' ## A shapefile with points
#' pfile <- system.file("extdata/point.shp", package = "vapour")
#'
#' ## raw binary WKB points in a list
#' ptgeom <- vapour_read_geometry(pfile)
#' ## create a filter query to ensure data read is small
#' SQL <- "SELECT FID FROM list_locality_postcode_meander_valley WHERE FID < 3"
#' ## polygons in raw binary (WKB)
#' plgeom <- vapour_read_geometry_text(mvfile, sql = SQL)
#' ## polygons in raw text (GeoJSON)
#' txtjson <- vapour_read_geometry_text(mvfile, sql = SQL)
#'
#' ## polygon extents in a list xmin, xmax, ymin, ymax
#' exgeom <- vapour_read_extent(mvfile)
#'
#' ## points in raw text (GeoJSON)
#' txtpointjson <- vapour_read_geometry_text(pfile)
#' ## points in raw text (WKT)
#' txtpointwkt <- vapour_read_geometry_text(pfile, textformat = "wkt")
#' @export
#' @aliases vapour_read_geometry_text vapour_read_extent
#' @name vapour_read_geometry
vapour_read_geometry <- function(dsource, layer = 0L, sql = "", limit_n = NULL, skip_n = 0, extent = NA) {
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  limit_n <- validate_limit_n(limit_n)
  extent <- validate_extent(extent, sql)
  vapour_read_geometry_cpp(dsource = dsource, layer = layer, sql = sql, what = "geometry", textformat = "", limit_n = limit_n, skip_n = skip_n, ex = extent)
}

#' @export
#' @rdname vapour_read_geometry
vapour_read_geometry_text <- function(dsource, layer = 0L, sql = "", textformat = "json", limit_n = NULL, skip_n = 0, extent = NA) {
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  textformat = match.arg (tolower (textformat), c ("json", "gml", "kml", "wkt"))
  limit_n <- validate_limit_n(limit_n)
  extent <- validate_extent(extent, sql)
  vapour_read_geometry_cpp(dsource = dsource, layer = layer, sql = sql, what = "text",
                           textformat = textformat, limit_n = limit_n, skip_n = skip_n, ex = extent)
}


#' @rdname vapour_read_geometry
#' @export
vapour_read_extent <- function(dsource, layer = 0L, sql = "", limit_n = NULL, skip_n = 0, extent = NA) {
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  limit_n <- validate_limit_n(limit_n)
  extent <- validate_extent(extent, sql)
  out <- vapour_read_geometry_cpp(dsource = dsource,
                                  layer = layer, sql = sql,
                                  what = "extent", textformat = "",
                                  limit_n = limit_n, skip_n = skip_n, ex = extent)
  nulls <- unlist(lapply(out, is.null))
  if (any(nulls)) out[nulls] <- replicate(sum(nulls), rep(NA_real_, 4L), simplify = FALSE)
  out
}
