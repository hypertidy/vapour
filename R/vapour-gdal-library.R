#' Set and query GDAL configuration options
#'
#' These functions can get and set configuration options for GDAL, for fine
#' control over specific GDAL behaviours. 
#' 
#' Configuration options may also be set as environment variables. 
#' 
#' See [GDAL config options](https://trac.osgeo.org/gdal/wiki/ConfigOptions) for
#' details on available options. 
#' 
#' @param option GDAL config name (see Details), character string
#' @param value value for config option, character string
#'
#' @return character string for `vapour_get_config`, integer 1 for successful `vapour_set_config()`
#' @export
#'
#' @examples
#' \dontrun{
#' (orig <- vapour_get_config("GDAL_CACHEMAX"))
#' vapour_set_config("GDAL_CACHEMAX", "64")
#' vapour_get_config("GDAL_CACHEMAX")
#' vapour_set_config("GDAL_CACHEMAX", orig)
#' }
vapour_set_config <- function(option, value) {
  option <- as.character(option[1L])
  value <- as.character(value[1])
  if (length(option) < 1 || nchar(option) < 1 || is.na(option) || is.null(option) ) {
    stop(sprintf("invalid 'option': %s - must be valid character string"))
  }
  if (length(value) < 1 || nchar(value) < 1 || is.na(value) || is.null(value) ) {
    stop(sprintf("invalid 'value': %s - must be valid character string"))
  }
  set_gdal_config_cpp(option, value)
}

#' @export
#' @name vapour_set_config
vapour_get_config  <- function(option) {
  if (length(option) < 1 || nchar(option) < 1 || is.na(option) || is.null(option) ) {
    stop(sprintf("invalid 'option': %s - must be valid character string"))
  }
  get_gdal_config_cpp(option)
}


#' PROJ4 string to WKT
#'
#' Convert a projstring to Well Known Text.
#'
#' The function is vectorized because why not, but probably only ever will be
#' used on single element vectors of character strings.
#'
#' Note that no sanitizing is done on inputs, we literally just 'OGRSpatialReference.SetFromUserInput(crs)' and
#' give the output as WKT. If it's an error in GDAL it's an error in R.
#'
#' Common inputs are WKT variants,
#' 'AUTH:CODE's e.g. 'EPSG:3031', the 'OGC:CRS84' for long,lat WGS84, 'ESRI:code' and other authority variants, and
#' datum names such as 'WGS84','NAD27' recognized by PROJ itself.
#'
#' See help for 'SetFromUserInput' in 'OGRSpatialReference', and 'proj_create_crs_to_crs'.
#' 
#' [c.proj_create_crs_to_crs](https://proj.org/development/reference/functions.html#c.proj_create_crs_to_crs) 
#' 
#' [c.proj_create](https://proj.org/development/reference/functions.html#c.proj_create)
#' 
#' [SetFromUserInput](https://gdal.org/doxygen/classOGRSpatialReference.html#aec3c6a49533fe457ddc763d699ff8796)
#' 
#' @param crs projection string, see Details.
#' @export
#' @return WKT2 projection string
#' @examples
#' vapour_srs_wkt("+proj=laea +datum=WGS84")
vapour_srs_wkt <- function(crs) {
  do.call(c, lapply(crs, proj_to_wkt_gdal_cpp))
}


#' Is the CRS string representative of angular coordinates
#' 
#' Returns `TRUE` if this is longitude latitude data. Missing, malformed, zero-length values are disallowed. 
#'
#' @param crs character string of length 1
#'
#' @return logical value `TRUE` for lonlat, `FALSE` otherwise
#' @export
#'
#' @examples
#' vapour_crs_is_lonlat("+proj=laea")
#' vapour_crs_is_lonlat("+proj=longlat")
#' vapour_crs_is_lonlat("+init=EPSG:4326")
#' vapour_crs_is_lonlat("OGC:CRS84")
#' vapour_crs_is_lonlat("WGS84")
#' vapour_crs_is_lonlat("NAD27")
#' vapour_crs_is_lonlat("EPSG:3031")
vapour_crs_is_lonlat <- function(crs) {
  crs_in <- crs[1L]
  if (length(crs) > 1L) message("multiple crs input is not supported, using the first only")
  if (is.na(crs_in) || is.null(crs_in) || length(crs_in) < 1L || !nzchar(crs_in)) stop(sprintf("problem with input crs: %s", crs_in))
  crs_is_lonlat_cpp(crs)
}

#' Summary of available geometry
#'
#' Read properties of geometry from a source, optionally after SQL execution.
#'
#' Use `limit_n` to arbitrarily limit the number of features queried.
#' @inheritParams vapour_read_geometry
#'
#' @return list containing the following
#' * `FID` the feature id value (an integer, usually sequential)
#' * `valid_geometry` logical value if a non-empty geometry is available
#' * `type` integer value of geometry type from [GDAL enumeration](https://gdal.org/doxygen/ogr__core_8h.html#a800236a0d460ef66e687b7b65610f12a)
#' * `xmin, xmax, ymin, ymax` numeric values of the extent (bounding box) of each geometry
#' @export
#'
#' @examples
#' file <- "list_locality_postcode_meander_valley.tab"
#' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
#' vapour_geom_summary(mvfile, limit_n = 3L)
#'
#' gsum <- vapour_geom_summary(mvfile)
#' plot(NA, xlim = range(c(gsum$xmin, gsum$xmax), na.rm = TRUE),
#'          ylim = range(c(gsum$ymin, gsum$ymax), na.rm = TRUE))
#' rect(gsum$xmin, gsum$ymin, gsum$xmax, gsum$ymax)
#' text(gsum$xmin, gsum$ymin, labels = gsum$FID)
vapour_geom_summary <- function(dsource, layer = 0L, sql = "", limit_n = NULL, skip_n = 0, extent = NA) {
  #limit_n <- validate_limit_n(limit_n)
  dsource <- .check_dsn_single(dsource)
  if (!is.numeric(layer)) layer <- index_layer(x = dsource, layername = layer)
  extent <- validate_extent(extent, sql, warn = FALSE)

  extents <- vapour_read_extent(dsource = dsource, layer = layer, sql = sql, limit_n = limit_n, skip_n = skip_n, extent = extent)
  fids <- vapour_read_names(dsource = dsource, layer = layer, sql = sql, limit_n = limit_n, skip_n = skip_n, extent = extent)
  ## FIXME the other funs deal with these args
  #limit_n <- validate_limit_n(limit_n)
  extent <- validate_extent(extent, sql)
  types <- vapour_read_type(dsource = dsource, layer = layer, sql = sql, limit_n = limit_n, skip_n = skip_n, extent = extent)
  na_geoms <- unlist(lapply(extents, anyNA), use.names = FALSE)
  list(FID = fids,
       valid_geometry = !na_geoms,
       type = types,
       xmin = unlist(lapply(extents, "[", 1L), use.names = FALSE),
       xmax = unlist(lapply(extents, "[", 2L), use.names = FALSE),
       ymin = unlist(lapply(extents, "[", 3L), use.names = FALSE),
       ymax = unlist(lapply(extents, "[", 4L), use.names = FALSE))
}

#' GDAL version and drivers.
#'
#' Return information about the GDAL library in use.
#'
#' `vapour_gdal_version` returns the version of GDAL as a string. This corresponds to the "--version"
#' as described for "GDALVersionInfo". [GDAL documentation](https://gdal.org/).
#'
#' `vapour_all_drivers` returns the names and capabilities of all available drivers, in a list. This contains:
#' * `driver` the driver (short) name
#' * `name` the (long) description name
#' * `vector` logical vector indicating a vector driver
#' * `raster` logical vector indicating a raster driver
#' * `create` driver can create (note vapour provides no write capacity)
#' * `copy`   driver can copy (note vapour provides no write capacity)
#' * `virtual` driver has virtual capabilities ('vsi')
#'
#' `vapour_driver()` returns the short name of the driver, e.g. 'GPKG' or 'GTiff', to get the
#' long name and other properties use `vapour_all_drivers()` and match on 'driver'.
#'
#' @export
#' @aliases vapour_all_drivers vapour_driver
#' @rdname GDAL-library
#' @return please see Details, character vectors or lists of character vectors
#' @examples
#' vapour_gdal_version()
#'
#' drv <- vapour_all_drivers()
#'
#' f <- system.file("extdata/sst_c.gpkg", package = "vapour")
#' vapour_driver(f)
#'
#' as.data.frame(drv)[match(vapour_driver(f), drv$driver), ]
vapour_gdal_version <- function() {
  version_gdal_cpp()
}
#' @rdname GDAL-library
#' @export
vapour_all_drivers <- function() {
  drivers_list_gdal_cpp()
}



#' @rdname GDAL-library
#' @export
#' @param dsource data source string (i.e. file name or URL or database connection string)
vapour_driver <- function(dsource) {
  dsource <- .check_dsn_single(dsource)
  driver_id_gdal_cpp(dsource);
}

