## NOTE: this file is named "000.*.R" so that it sorts before RcppExports.R
## so that vapour_read_geometry_cpp is at the end of the Usage list in
## ?vapour_read_geometry
## and we also use Collate in DESCRIPTION, in case this note gets missed and
## no one understands why - there's an @include 000_vapour_input.R in the C++
## for compileAttributes that puts this file ahead of RcppExports.R :)

## only select FID, no matter what user asks for
fid_select <- function(x) {
  if (nchar(x) < 1) return(x)

  sqlout <- sprintf("SELECT FID FROM %s",
                    substr(x, gregexpr("FROM ", x)[[1]][1] + 5, nchar(x)))
  if (x != sqlout) warning("select statement assumed, modified to 'SELECT FID FROM' boilerplate")
  sqlout

}
## only select *, no matter what user asks for
## pretty sure this is not used, fid_select is preferred
asterisk_select <- function(x) {
  if (nchar(x) < 1) return(x)
  sqlout <- sprintf("SELECT * FROM %s",
                    substr(x, gregexpr("FROM ", x)[[1]][1] + 5, nchar(x)))
  if (x != sqlout) warning("select statement assumed, modified to 'SELECT * FROM' boilerplate")
  sqlout

}


#' Layer names
#'
#' Obtain the names of available layers from a GDAL vector source.
#'
#' Some vector sources have multiple layers while many have only one. Shapefiles
#' for example have only one, and the single layer gets the file name with no path
#' and no extension. GDAL provides a quirk for shapefiles in that a directory may
#' act as a data source, and any shapefile in that directory acts like a layer of that
#' data source. This is a little like the one-or-many sleight that exists for raster
#' data sources with subdatasets (there's no way to virtualize single rasters into
#' a data source with multiple subdatasets, oh except by using VRT....)
#'
#' See [raster_sds_info()] for more on the multiple topic.
#'
#' @inheritParams vapour_read_geometry
#' @return character vector of layer names
#'
#' @examples
#' file <- "list_locality_postcode_meander_valley.tab"
#' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
#' vapour_layer_names(mvfile)
#' @export
vapour_layer_names <- function(dsource, sql = "") {
  vapour_layer_names_cpp(dsource = dsource, sql = sql)
}

#' Read feature names
#'
#' Obtains the internal 'Feature ID (FID)' for a data source.
#'
#' This may be virtual (created by GDAL for the SQL interface) and may be 0- or
#' 1- based. Some drivers have actual names, and they are persistent and
#' arbitrary. Please use with caution, this function can return the current
#' FIDs, but there's no guarantee of what it represents for subsequent access.
#'
#' @inheritParams vapour_read_attributes
#' @param ... ignored
#' @export
#' @examples
#' file <- "list_locality_postcode_meander_valley.tab"
#' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
#' range(fids <- vapour_read_names(mvfile))
#' length(fids)
vapour_read_names <- function(dsource, layer = 0L, sql = "", ...) {
  layers <- vapour_layer_names(dsource)
  if (nchar(sql) > 1) {
    sql <- fid_select(sql)
  } else {
    sql <- sprintf("SELECT FID FROM %s", layers[layer + 1])
  }
  vapour_read_attributes(dsource, layer = layer, sql = sql)[["FID"]]
}

#' Vector attributes read
#'
#' Read layer attributes, optionally after SQL select
#' @inheritParams vapour_read_geometry
#' @examples
#' file <- "list_locality_postcode_meander_valley.tab"
#' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
#' vapour_read_attributes(mvfile)
#' sq <- "SELECT * FROM list_locality_postcode_meander_valley WHERE FID < 5"
#' att <- vapour_read_attributes(mvfile, sql = sq)
#' dsource <- "inst/extdata/tab/list_locality_postcode_meander_valley.tab"
#'
#' @export
vapour_read_attributes <- function(dsource, layer = 0L, sql = "") {
  vapour_read_attributes_cpp(dsource = dsource, layer = layer, sql = sql)
}

#' GDAL geometry extent
#'
#' Read a GDAL geometry summary as just the native bounding box, the four
#' numbers xmin, xmax, ymin, ymax in the usual simple convention.
#'
#' @inheritParams vapour_read_attributes
#' @examples
#' file <- "list_locality_postcode_meander_valley.tab"
#' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
#' vapour_read_extent(mvfile)
#' @export
vapour_read_extent <- function(dsource, layer = 0L, sql = "") {
  sql <- fid_select(sql)
  vapour_read_geometry_cpp(dsource = dsource, layer = layer, sql = sql, what = "extent", textformat = "")
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
#' @param dsource data source name (path to file, connection string, URL)
#' @param layer integer of layer to work with, defaults to the first (0)
#' @param sql if not empty this is executed against the data source (layer will be ignored)
#' @param what what to read, "geometry", "text", "extent"
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
#' txtpointwkt <- vapour_read_geometry_text(pfile, format = "wkt)
#' @export
#' @aliases vapour_read_geometry vapour_read_geometry_text vapour_read_geometry_cpp
#' @name vapour_read_geometry
vapour_read_geometry <- function(dsource, layer = 0L, sql = "") {
  sql <- fid_select(sql)
  vapour_read_geometry_cpp(dsource = dsource, layer = layer, sql = sql, what = "geometry", textformat = "")
}


#' @export
#' @name vapour_read_geometry
vapour_read_geometry_text <- function(dsource, layer = 0L, sql = "", textformat = "json") {
  sql <- fid_select(sql)
  vapour_read_geometry_cpp(dsource = dsource, layer = layer, sql = sql, what = "text", textformat = textformat)
}


