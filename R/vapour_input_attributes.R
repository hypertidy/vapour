
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

## find index of a layer name, or error
index_layer <- function(x, layername) {
  if (is.factor(layername)) {
    warning("layer is a factor, converting to character")
    layername <- levels(layername)[layername]
  }
  available_layers <- try(vapour_layer_names(x), silent = TRUE)
  if (inherits(available_layers, "try-error")) stop(sprintf("cannot open data source: %s", x))
 idx <- match(layername, available_layers)
 if (length(idx) != 1 || !is.numeric(idx)) stop(sprintf("cannot find layer: %s", layername))
 if (is.na(idx) || idx < 1 || idx > length(available_layers)) stop(sprintf("layer index not found for: %s \n\nto determine, compare 'vapour_layer_names(dsource)'", layername))
 idx - 1L  ## layer is 0-based
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
#' See [vapour_sds_names] for more on the multiple topic.
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
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
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
#' Read features attributes, optionally after SQL execution.
#' @inheritParams vapour_read_geometry
#' @examples
#' file <- "list_locality_postcode_meander_valley.tab"
#' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
#' att <- vapour_read_attributes(mvfile)
#' str(att)
#' sq <- "SELECT * FROM list_locality_postcode_meander_valley WHERE FID < 5"
#' (att <- vapour_read_attributes(mvfile, sql = sq))
#' pfile <- "list_locality_postcode_meander_valley.tab"
#' dsource <- system.file(file.path("extdata/tab", pfile), package="vapour")
#' SQL <- "SELECT NAME FROM list_locality_postcode_meander_valley WHERE POSTCODE < 7300"
#' vapour_read_attributes(dsource, sql = SQL)
#' @export
vapour_read_attributes <- function(dsource, layer = 0L, sql = "") {
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  vapour_read_attributes_cpp(dsource = dsource, layer = layer, sql = sql)
}

