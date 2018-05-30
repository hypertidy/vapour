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


#' Read GDAL layer names
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
#' The `sql` input is only used for the `WHERE` clause, and forcibly modifies
#' all input query to `SELECT FID` - use the [vapour_read_attributes] with SQL to
#' be more specific.
#' @inheritParams vapour_read_geometry
#' @export
#' @examples
#' file <- "list_locality_postcode_meander_valley.tab"
#' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
#' range(fids <- vapour_read_names(mvfile))
#' length(fids)
vapour_read_names <- function(dsource, layer = 0L, sql = "") {
  layers <- vapour_layer_names(dsource)
  if (nchar(sql) > 1) {
    sql <- fid_select(sql)
  } else {
    sql <- sprintf("SELECT FID FROM %s", layers[layer + 1])
  }
  vapour_read_attributes(dsource, layer = layer, sql = sql)[["FID"]]
}

#' Read feature attribute data
#'
#' Read features attributes, optionally after SQL select.
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
