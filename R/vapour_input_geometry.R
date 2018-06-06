
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
#' @param dsource data source name (path to file, connection string, URL)
#' @param layer integer of layer to work with, defaults to the first (0) or the name of the layer
#' @param sql if not empty this is executed against the data source (layer will be ignored)
#' @param textformat indicate text output format, available are "json" (default), "gml", "kml", "wkt"
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
vapour_read_geometry <- function(dsource, layer = 0L, sql = "") {
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  sql <- fid_select(sql)
  vapour_read_geometry_cpp(dsource = dsource, layer = layer, sql = sql, what = "geometry", textformat = "")
}

#' @export
#' @rdname vapour_read_geometry
vapour_read_geometry_text <- function(dsource, layer = 0L, sql = "", textformat = "json") {
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  textformat = match.arg (tolower (textformat), c ("json", "gml", "kml", "wkt"))
  sql <- fid_select(sql)
  vapour_read_geometry_cpp(dsource = dsource, layer = layer, sql = sql, what = "text", textformat = textformat)
}


#' @rdname vapour_read_geometry
#' @export
vapour_read_extent <- function(dsource, layer = 0L, sql = "") {
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  sql <- fid_select(sql)
  vapour_read_geometry_cpp(dsource = dsource, layer = layer, sql = sql, what = "extent", textformat = "")
}
