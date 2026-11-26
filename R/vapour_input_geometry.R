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
#'  \item{projection}{a list of formats of the projection metadata, see next}
#'  \item{filelist}{the files that make up the data source (may be none)}
#'  }
#'
#' `$projection` is a list with elements `Wkt`, `Proj4` and `EPSG`.
#' Use `$projection$Wkt` as most authoritative, but we don't enter into the discussion or limit what
#' might be done with this (that's up to you). `EPSG` is the integer EPSG code when the
#' projection carries one or GDAL can identify one for it, otherwise `NA`.
#'
#' To get the geometry type/s of a layer see [vapour_read_type()].
#'
#' @inheritParams vapour_read_geometry
#' @param ... unused, reserved for future use
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
#' 
#' ## info depends on the query/spatial-filter
#' vapour_layer_info(mvfile, extent = c(412000,  420000, 5352612.8, 5425154.3), 
#'   sql = "SELECT * FROM list_locality_postcode_meander_valley")$count
vapour_layer_info <- function(dsource, layer = 0L, sql = "", extent = NA, count = TRUE, ..., open_options = NULL) {
  dsource <- .check_dsn_single(dsource)
  open_options <- .check_open_options(open_options)
  layer_names <- try(vapour_layer_names(dsource, open_options = open_options), silent = TRUE)
  if (inherits(layer_names, "try-error")) {
    if (grepl("rest.*FeatureServer/0", dsource)) {
      message("looks like dsource is a ESRI Feature Services, try constructing the DSN as \n \ https://<_>rest/services/<sources>/<layerset>/FeatureServer/0query?where=objectid+%3D+objectid&outfields=*&f=json\n see ESRIJSON document in gdal.org and this example https://gist.github.com/mdsumner/c54bdc119accf95138f5ad7ab574337d\n")
    }
    stop(conditionMessage(attr(layer_names, "condition")), call. = FALSE)
  }
  layer_name <- layer
  if (!is.numeric(layer)) layer <- match(layer_name, layer_names) - 1
  if (is.numeric(layer_name)) layer_name <- layer_names[layer + 1]
  if (is.na(layer)) stop(sprintf("layer: %s not found", layer_name))

  driver <- vapour_driver(dsource, open_options = open_options)
  geom_name <- vapour_geom_name(dsource, layer, sql, open_options = open_options)

  fields <- vapour_report_fields(dsource, layer, sql, open_options = open_options)
  
  if (count) {
    cnt <- feature_count_gdal_cpp(dsource, as.integer(layer), sql, as.numeric(extent), open_options)
  } else {
    cnt <- NA_integer_
  }
    ext <- vapour_layer_extent(dsource, layer, sql, extent = extent, open_options = open_options)

  list(dsn = dsource, driver = driver, layer = layer_names[layer + 1],
       layer_names = layer_names,
       fields = fields,
       count = cnt,
       extent = ext,
       projection = projection_info_gdal_cpp(dsource, layer = as.integer(layer), sql = sql, open_options = open_options)[c("Wkt", "Proj4", "EPSG")],
       filelist = vector_filelist_gdal_cpp(dsource, open_options))
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
vapour_layer_extent <- function(dsource, layer = 0L, sql = "", extent = 0, ..., open_options = NULL) {
  dsource <- .check_dsn_single(dsource)
  open_options <- .check_open_options(open_options)
  layer_names <- vapour_layer_names(dsource, open_options = open_options)
  layer_name <- layer
  if (!is.numeric(layer)) layer <- match(layer_name, layer_names) - 1
  if (is.numeric(layer_name)) layer_name <- layer_names[layer + 1]
  if (is.na(layer)) stop(sprintf("layer: %s not found", layer_name))
  extent <- validate_extent(extent, sql)
  
  
 vapour_layer_extent_cpp(dsource, as.integer(layer), sql, as.numeric(extent), open_options) 
}

#' Read GDAL feature geometry
#'
#' Read GDAL geometry as binary blob, text, or numeric extent.
#'
#' `vapour_read_geometry` will read features as binary WKB, `vapour_read_geometry_text` as various text formats (geo-json, wkt, kml, gml),
#' 
#' `vapour_read_extent` a numeric extent which is the native bounding box, the four numbers (in this order) `xmin, xmax, ymin, ymax`.
#' `vapour_feature_extent` gives the same extents as a matrix, one row for each feature, with those four as named columns. A 
#' missing geometry is a row of `NA`.
#' For each function an optional SQL string will be evaluated against the data source before reading.
#'
#' `vapour_read_geometry_ia` will read features by *arbitrary index*, so any integer between 0 and one less than the number of
#' features. These may be duplicated. If 'ia' is greater than the highest index NULL is returned, but if less than 0 the function will error.
#'
#' `vapour_read_geometry_ij` will read features by *index range*, so two numbers to read every feature between those limits inclusively.
#' 'i' and 'j' must be increasing.
#'
#' `vapour_read_geometry_fa` will read features by *feature id*, the FID values that [vapour_read_fids()] returns, in any order and
#' with repeats. A FID that is not in the layer gives NULL, with a warning. This is direct access where the driver supports it,
#' so it does not scan the layer.
#'
#' The same three ways to choose features are available for fields, see [vapour_read_fields_ia()], and for feature ids,
#' see [vapour_read_fids_ia()].
#'
#' `vapour_read_type` will read the (wkb) type of the geometry as an integer. These are
#' `0` unknown, `1` Point, `2` LineString, `3` Polygon, `4` MultiPoint, `5` MultiLineString,
#' `6` MultiPolygon, `7` GeometryCollection, and the other more exotic types listed in "api/vector_c_api.html" from the
#' GDAL home page (as at October 2020).  A missing value 'NA' indicates an empty geometry. 
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
#' @param ij a range index, integer vector of length two with values between 0 and one less the number of features, this range of geometries is returned
#' @param fa feature id (FID) values, as returned by [vapour_read_fids()], in any order and with repeats
#' @param open_options open options for the driver, a character vector of `"NAME=VALUE"` strings (as `-oo` in the GDAL command line tools), `NULL` for none. The options a driver accepts are listed on its page at <https://gdal.org/en/stable/drivers/vector/index.html>
#' @return for [vapour_read_geometry()], [vapour_read_geometry_ia()], [vapour_read_geometry_ij()] and [vapour_read_geometry_fa()] a raw vector of
#'  geometry, for [vapour_read_extent()] a list of numeric vectors each with 'xmin,xmax,ymin,ymax' respectively for each geometry, 
#'  for [vapour_feature_extent()] a numeric matrix with columns 'xmin', 'xmax', 'ymin', 'ymax' and one row for each geometry, 
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
#' ## the same extents as a matrix, one row for each polygon
#' exmat <- vapour_feature_extent(mvfile)
#' head(exmat)
#'
#' ## points in raw text (GeoJSON)
#' txtpointjson <- vapour_read_geometry_text(pfile)
#' ## points in raw text (WKT)
#' txtpointwkt <- vapour_read_geometry_text(pfile, textformat = "wkt")
#'
#' ## chosen features: by position, by range of positions, and by feature id
#' g_ia <- vapour_read_geometry_ia(mvfile, ia = c(0, 5, 2))
#' g_ij <- vapour_read_geometry_ij(mvfile, ij = c(0, 2))
#' fids <- vapour_read_fids(mvfile)
#' g_fa <- vapour_read_geometry_fa(mvfile, fa = fids[c(1, 6, 3)])
#' identical(g_ia, g_fa)
#' @export
#' @name vapour_read_geometry
vapour_read_geometry <- function(dsource, layer = 0L, sql = "", limit_n = NULL, skip_n = 0, extent = NA, open_options = NULL) {
  dsource <- .check_dsn_single(dsource)
  open_options <- .check_open_options(open_options)
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer, open_options)
  limit_n <- validate_limit_n(limit_n)
  extent <- validate_extent(extent, sql)
  read_geometry_gdal_cpp( dsn = dsource, layer = as.integer(layer), 
                          sql = sql, what = "wkb", 
                          textformat = "", 
                          limit_n = limit_n, skip_n = as.integer(skip_n), ex = as.numeric(extent),
                          open_options = open_options)
}

#' @export
#' @rdname vapour_read_geometry
vapour_read_geometry_text <- function(dsource, layer = 0L, sql = "", textformat = "json", limit_n = NULL, skip_n = 0L, extent = NA_real_, open_options = NULL) {
  dsource <- .check_dsn_single(dsource)
  open_options <- .check_open_options(open_options)
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer, open_options)
  textformat = match.arg (tolower (textformat), c ("json", "gml", "kml", "wkt"))
  limit_n <- validate_limit_n(limit_n)
  extent <- validate_extent(extent, sql)
  read_geometry_gdal_cpp(dsn = dsource, layer = as.integer(layer), sql = sql, what = textformat,
                           textformat = "", limit_n = limit_n, skip_n = as.integer(skip_n), 
                         ex = as.numeric(extent), open_options = open_options)
}


#' @rdname vapour_read_geometry
#' @export
vapour_read_extent <- function(dsource, layer = 0L, sql = "", limit_n = NULL, skip_n = 0, extent = NA, open_options = NULL) {
  dsource <- .check_dsn_single(dsource)
  open_options <- .check_open_options(open_options)
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer, open_options)
  limit_n <- validate_limit_n(limit_n)
  extent <- validate_extent(extent, sql)

  out <- read_geometry_gdal_cpp(dsn = dsource,
                                  layer = as.integer(layer), sql = sql,
                                  what = "extent", textformat = "",
                                  limit_n = limit_n, skip_n = as.integer(skip_n), ex = as.numeric(extent),
                                  open_options = open_options)
  nulls <- unlist(lapply(out, is.null), use.names = FALSE)
  if (any(nulls)) out[nulls] <- replicate(sum(nulls), rep(NA_real_, 4L), simplify = FALSE)
  out

}

#' @rdname vapour_read_geometry
#' @export
vapour_feature_extent <- function(dsource, layer = 0L, sql = "", limit_n = NULL, skip_n = 0, extent = NA, open_options = NULL) {
  ex <- vapour_read_extent(dsource, layer = layer, sql = sql, limit_n = limit_n, skip_n = skip_n, extent = extent,
                           open_options = open_options)
  matrix(as.double(unlist(ex, use.names = FALSE)), ncol = 4L, byrow = TRUE,
         dimnames = list(NULL, c("xmin", "xmax", "ymin", "ymax")))
}

#' @rdname vapour_read_geometry
#' @export
vapour_read_type <- function(dsource, layer = 0L,  sql = "", limit_n = NULL, skip_n = 0, extent = NA, open_options = NULL) {
  dsource <- .check_dsn_single(dsource)
  open_options <- .check_open_options(open_options)
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer, open_options)
  limit_n <- validate_limit_n(limit_n)
  extent <- validate_extent(extent, sql)
  out <- read_geometry_gdal_cpp(dsn = dsource,
                                layer = as.integer(layer), sql = sql,
                                what = "type", textformat = "",
                                limit_n = limit_n, skip_n = as.integer(skip_n), ex = as.numeric(extent),
                                open_options = open_options)

  nulls <- unlist(lapply(out, is.null), use.names = FALSE)
  if (any(nulls)) out[nulls] <- list(NA_integer_)
  unlist( out, use.names = FALSE)
}
