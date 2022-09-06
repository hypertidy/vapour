#' Read GDAL layer info
#'
#' Read GDAL layer information for a vector data source.
#'
#' Set `extent` and/or `count` to `FALSE` to avoid calculating them if not needed, it might take some time.
#'
#' The layer information elements are
#'
#' \describe{
#'  \item{dsn}{the data source name}
#'  \item{driver}{the short name of the driver used}
#'  \item{layer}{the name of the layer queried}
#'  \item{layer_names}{the name/s of all available layers (see [vapour_layer_names])}
#'  \item{fields}{a named vector of field types (see [vapour_report_fields])}
#'  \item{count}{the number of features in this data source (can be turned off to avoid the extra work `count`)}
#'  \item{extent}{the extent of all features xmin, xmax, ymin, ymax (can be turned off to avoid the extra work `extent`)}
#'  \item{projection}{a list of character strings, see next}
#'  }
#'
#' `$projection` is a list of various formats of the projection metadata.
#' Use `$projection$Wkt` as most authoritative, but we don't enter into the discussion or limit what
#' might be done with this (that's up to you). Currently we see
#' `c("Proj4", "MICoordSys", "PrettyWkt", "Wkt", "EPSG", "XML")` as names of this `$projection` element.
#'
#' To get the geometry type/s of a layer see [vapour_read_type()].
#'
#' @inheritParams vapour_read_geometry
#' @param ... unused, reserved for future use
#' @param extent logical to control if extent calculated and returned, TRUE by default (set to FALSE to avoid the extra calculation and missing value is the result)
#' @param count logical to control if count calculated and returned, TRUE by default (set to FALSE to avoid the extra calculation and missing value is the result)
#' @return list with a list of character vectors of projection metadata, see details
#' @export
#' @seealso vapour_geom_name vapour_layer_names vapour_report_fields vapour_read_fields vapour_driver vapour_read_names
#' @examples
#' file <- "list_locality_postcode_meander_valley.tab"
#' ## A MapInfo TAB file with polygons
#' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
#' info <- vapour_layer_info(mvfile)
#' names(info$projection)
vapour_layer_info <- function(dsource, layer = 0L, sql = "", ..., extent = TRUE, count = TRUE) {

  layer_names <- vapour_layer_names(dsource)
  layer_name <- layer
  if (!is.numeric(layer)) layer <- match(layer_name, layer_names) - 1
  if (is.numeric(layer_name)) layer_name <- layer_names[layer + 1]
  if (is.na(layer)) stop(sprintf("layer: %s not found", layer_name))

  driver <- vapour_driver(dsource)
  geom_name <- vapour_geom_name(dsource, layer, sql)

  fields <- vapour_report_fields(dsource, layer, sql)
  
  if (count) {
    cnt <- try(vapour_read_fields(dsource, sql = sprintf("SELECT COUNT(*) FROM \"%s\"", layer_name))[[1]], silent = TRUE)

    if (inherits(cnt, "try-error")) cnt <- length(vapour_read_names(dsource, layer, sql))
  } else {
    cnt <- NA_integer_
  }
  ## if we're getting extent
  if (extent) {
    ext <- vapour_layer_extent(dsource, layer, sql)
  } else {
    ext <- rep(NA_real_, 4L)
  }
  list(dsn = dsource, driver = driver, layer = layer_names[layer + 1],
       layer_names = layer_names,
       fields = fields,
       count = cnt,
       extent = ext,
       projection = projection_info_gdal_cpp(dsource, layer = layer, sql = sql)[c("Wkt", "Proj4", "EPSG")])
}

#' Read layer extent
#' 
#' Extent of all features in entire layer, possibly after execution of sql query and
#' input extent filter. 
#'
#' @inheritParams vapour_read_geometry
#' @param extent optional extent (xmin,xmax,ymin,ymax)
#' @param ... unused
#'
#' @return vector of numeric values xmin,xmax,ymin,ymax
#' @seealso vapour_read_extent vapour_layer_info
#' @export
#'
#' @examples
#' file <- "list_locality_postcode_meander_valley.tab"
#' ## A MapInfo TAB file with polygons
#' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
#' vapour_layer_extent(mvfile)
vapour_layer_extent <- function(dsource, layer = 0L, sql = "", extent = 0, ...) {
  layer_names <- vapour_layer_names(dsource)
  layer_name <- layer
  if (!is.numeric(layer)) layer <- match(layer_name, layer_names) - 1
  if (is.numeric(layer_name)) layer_name <- layer_names[layer + 1]
  if (is.na(layer)) stop(sprintf("layer: %s not found", layer_name))
  extent <- validate_extent(extent, sql)
  
  
 vapour_layer_extent_cpp(dsource, layer, sql, extent) 
}

#' Read GDAL feature geometry
#'
#' Read GDAL geometry as binary blob, text, or numeric extent.
#'
#' `vapour_read_geometry` will read features as binary WKB, `vapour_read_geometry_text` as various text formats (geo-json, wkt, kml, gml),
#' 
#' `vapour_read_extent` a numeric extent which is the native bounding box, the four numbers (in this order) `xmin, xmax, ymin, ymax`.
#' For each function an optional SQL string will be evaluated against the data source before reading.
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
#' @return for [vapour_read_geometry()], [vapour_read_geometry_ia()] and [vapour_read_geometry_ij()] a raw vector of
#'  geometry, for [vapour_read_extent()] a list of numeric vectors each with 'xmin,xmax,ymin,ymax' respectively for each geometry, 
#'  for [vapour_read_type()] a character vector. See Details for more information. 
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
  nulls <- unlist(lapply(out, is.null), use.names = FALSE)
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

  nulls <- unlist(lapply(out, is.null), use.names = FALSE)
  if (any(nulls)) out[nulls] <- list(NA_integer_)
  unlist( out, use.names = FALSE)
}
