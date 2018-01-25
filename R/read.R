#'  GDAL geometry extent
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
  vapour_read_feature_what(dsource = dsource, layer = layer, sql = sql, what = "extent", textformat = "")
}


#' Read GDAL geometry as blob
#'
#' Simple read of geometry-only as WKB format.
#'
#'
#' @inheritParams vapour_read_attributes
#' @format indicate text output format, available are "json" (default), "gml", "kml", "wkt"
#' @examples
#' file <- "list_locality_postcode_meander_valley.tab"
#' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
#' #tib <- tibble::tibble(wkb = vapour_read_geometry(mvfile)) %>%
#' #  bind_cols(read_gdal_table(mvfile))
#' pfile <- system.file("extdata/point.shp", package = "vapour")
#' vapour_read_geometry(pfile)
#' @export
vapour_read_geometry <- function(dsource, layer = 0L, sql = "") {
  vapour_read_feature_what(dsource = dsource, layer = layer, sql = sql, what = "geometry", textformat = "")
}
#' Read GDAL geometry as text
#'
#' Simple read of geometry-only as text format.
#'
#' @inheritParams vapour_read_attributes
#' @param textformat indicate text output format, available are "json" (default), "gml", "kml", "wkt"
#' @examples
#' file <- "list_locality_postcode_meander_valley.tab"
#' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
#' vapour_read_geometry_text(mvfile)
#' pfile <- system.file("extdata/point.shp", package = "vapour")
#' vapour_read_geometry_text(pfile)
#' @export
vapour_read_geometry_text <- function(dsource, layer = 0L, sql = "", textformat = "json") {
  vapour_read_feature_what(dsource = dsource, layer = layer, sql = sql, what = "text", textformat = textformat)
}
