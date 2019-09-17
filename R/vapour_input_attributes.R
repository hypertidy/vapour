
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

validate_limit_n <- function(x) {
  if (is.null(x)) {
    x <- 0L
  } else {
    if (x < 1) stop("limit_n must be 1 or greater")
  }
  stopifnot(is.numeric(x))
  x
}

validate_extent <- function(extent, sql, warn = TRUE) {
  if (length(extent) > 1) {
    if (is.matrix(extent) && all(colnames(extent) == c("min", "max")) && all(rownames(extent) == c("x", "y"))) {
      extent <- as.vector(t(extent))
    }
    if (inherits(extent, "bbox")) extent <- extent[c("xmin", "xmax", "ymin", "ymax")]

    if (!length(extent) == 4) stop("'extent' must be length 4 'c(xmin, xmax, ymin, ymax)'")
  } else {
    if (inherits(extent, "Extent")) extent <- c(xmin = extent@xmin, xmax = extent@xmax,
                                                ymin = extent@ymin, ymax = extent@ymax)
  }
  if (is.na(extent[1])) extent = 0.0
  if (warn && length(extent) == 4L && nchar(sql) < 1) warning("'extent' given but 'sql' query is empty, extent clipping will be ignored")
  if (!is.numeric(extent)) stop("extent must be interpretable as xmin, xmax, ymin, ymax")
  extent
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
#' An earlier version use 'OGRSQL' to obtain these names, which was slow for some
#' drivers and also clashed with independent use of the `sql` argument.
#' @inheritParams vapour_read_geometry
#' @export
#' @examples
#' file <- "list_locality_postcode_meander_valley.tab"
#' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
#' range(fids <- vapour_read_names(mvfile))
#' length(fids)
vapour_read_names <- function(dsource, layer = 0L, sql = "", limit_n = NULL, skip_n = 0, extent = NA) {
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  limit_n <- validate_limit_n(limit_n)
  skip_n <- skip_n[1L]
  if (skip_n < 0) stop("skip_n must be 0, or higher")
  extent <- validate_extent(extent, sql)
  fids <- vapour_read_names_cpp(dsource, layer = layer, sql = sql, limit_n = limit_n, skip_n = skip_n, ex = extent)
  unlist(lapply(fids, function(x) if (is.null(x)) NA_real_ else x))
}

#' Read feature field attributes types.
#'
#' Obtains the internal type-constant name for the data attributes in a source.
#' Use this to compare the interpreted versions converted into R types by
#' `vapour_read_attributes`.
#'
#' These are defined for the enum OGRFieldType in GDAL itself.
#' \url{https://gdal.org/doxygen/ogr__core_8h.html#a787194bea637faf12d61643124a7c9fc}
#'
#' @inheritParams vapour_read_geometry
#' @export
#' @examples
#' file <- "list_locality_postcode_meander_valley.tab"
#' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
#' vapour_report_attributes(mvfile)
#'
#' ## modified by sql argument
#' vapour_report_attributes(mvfile,
#'   sql = "SELECT POSTCODE, NAME FROM list_locality_postcode_meander_valley")
vapour_report_attributes <- function(dsource, layer = 0L, sql = "") {
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  vapour_report_attributes_cpp(dsource, layer, sql = sql)
}


#' Read feature attribute data
#'
#' Read features attributes, optionally after SQL execution.
#'
#' Internal types are not fully supported, there are straightforward conversions
#' for numeric, integer (32-bit) and string types. Date, Time, DateTime are
#' returned as character, and Integer64 is returned as numeric.
#'
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
vapour_read_attributes <- function(dsource, layer = 0L, sql = "", limit_n = NULL, skip_n = 0, extent = NA) {
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  limit_n <- validate_limit_n(limit_n)
  extent <- validate_extent(extent, sql)
  vapour_read_attributes_cpp(dsource = dsource, layer = layer, sql = sql, limit_n = limit_n, skip_n = skip_n, ex = extent)
}

