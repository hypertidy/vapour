#' Read GDAL layer info
#'
#' Read GDAL layer information for a vector data source.
#'
#' Currently we only return `$projection` which is a list of various formats of the projection metadata.
#' Use `$projection$Wkt` as most authoritative, but we don't enter into the discussion or limit what
#' might be done with this (that's up to you). Currently we see
#' `c("Proj4", "MICoordSys", "PrettyWkt", "Wkt", "EPSG", "XML")` as names of this `$projection` element.
#'
#' Future versions might also include the attribute field types, the feature count, the file list, what else?
#' @inheritParams vapour_read_geometry
#' @param ... unused, reserved for future use
#' @return list with a list of character vectors of projection metadata, see details
#' @export
#'
#' @examples
#' file <- "list_locality_postcode_meander_valley.tab"
#' ## A MapInfo TAB file with polygons
#' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
#' names(vapour_layer_info(mvfile)$projection)
vapour_layer_info <- function(dsource, layer = 0L, sql = "", ...) {
  list(projection = projection_info_gdal_cpp(dsource, layer = layer, sql = sql))
}


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
#' `vapour_read_geometry_ia` will read features by *arbitrary index*, so any integer between 0 and one less than the number of
#' features. These may be duplicated. If 'ia' is greater than the highest index NULL is returned, but if less than 0 the function will error.
#'
#' `vapour_read_geometry_ij` will read features by *index range*, so two numbers to read ever feature between those limits inclusively.
#' 'i' and 'j' must be increasing.
#'
#' `vapour_read_type` will read the (wkb) type of the geometry as an integer. These are
#' `0` unknown, `1` Point, `2` LineString, `3` Polygon, `4` MultiPoint, `5` MultiLineString,
#' `6` MultiPolygon, `7` GeometryCollection, and the other more exotic types listed in "api/vector_c_api.html" from the
#' GDAL home page (as at October 2020).
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
#' @param ia an arbitrary index, integer vector with values between 0 and one less the number of features, duplicates allowed and arbitrary order is ok
#' @param ij an range index, integer vector of length two with values between 0 and one less the number of features, this range of geometries is returned
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
#' @name vapour_read_geometry
vapour_read_geometry <- function(dsource, layer = 0L, sql = "", limit_n = NULL, skip_n = 0, extent = NA) {
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  limit_n <- validate_limit_n(limit_n)
  extent <- validate_extent(extent, sql)
  read_geometry_gdal_cpp( dsn = dsource, layer = layer, sql = sql, what = "geometry", textformat = "", limit_n = limit_n, skip_n = skip_n, ex = extent)
}

#' @export
#' @rdname vapour_read_geometry
vapour_read_geometry_text <- function(dsource, layer = 0L, sql = "", textformat = "json", limit_n = NULL, skip_n = 0, extent = NA) {
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  textformat = match.arg (tolower (textformat), c ("json", "gml", "kml", "wkt"))
  limit_n <- validate_limit_n(limit_n)
  extent <- validate_extent(extent, sql)
  read_geometry_gdal_cpp(dsn = dsource, layer = layer, sql = sql, what = "text",
                           textformat = textformat, limit_n = limit_n, skip_n = skip_n, ex = extent)
}


#' @rdname vapour_read_geometry
#' @export
vapour_read_extent <- function(dsource, layer = 0L, sql = "", limit_n = NULL, skip_n = 0, extent = NA) {
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  limit_n <- validate_limit_n(limit_n)
  extent <- validate_extent(extent, sql)
  out <- read_geometry_gdal_cpp(dsn = dsource,
                                  layer = layer, sql = sql,
                                  what = "extent", textformat = "",
                                  limit_n = limit_n, skip_n = skip_n, ex = extent)
  nulls <- unlist(lapply(out, is.null))
  if (any(nulls)) out[nulls] <- replicate(sum(nulls), rep(NA_real_, 4L), simplify = FALSE)
  out

}

#' @rdname vapour_read_geometry
#' @export
vapour_read_type <- function(dsource, layer = 0L,  sql = "", limit_n = NULL, skip_n = 0, extent = NA) {
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  limit_n <- validate_limit_n(limit_n)
  extent <- validate_extent(extent, sql)
  out <- read_geometry_gdal_cpp(dsn = dsource,
                                layer = layer, sql = sql,
                                what = "type", textformat = "",
                                limit_n = limit_n, skip_n = skip_n, ex = extent)

  nulls <- unlist(lapply(out, is.null))
  if (any(nulls)) out[nulls] <- list(NA_integer_)
  unlist( out)
}
