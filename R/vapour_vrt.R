#' Virtual raster 
#' 
#' Simple VRT creation of a GDAL virtual raster. The data source string is
#' **augmented** by input of other optional arguments. That means it **overrides**
#' their values provided by the source data, or stands in place of this information if
#' it is missing. 
#' 
#' Create a GDAL data source string (to be used like a filename) with various helpers. 
#' VRT stands for 'ViRTual'. A VRT string then acts as a representative of a data source for
#' further use (to read or warp it). 
#'
#' An input string will be converted to a single subdataset, use 'sds' argument to select. 
#'
#' If 'extent', 'projection' is provided this is applied to override the source's extent and/or projection. 
#' (These might be invalid, or missing, so we facilitate correcting this). 
#' 
#' If 'bands' is provided this is used to select a set of bands (numbered from 1), which might be repeated, or
#' in any order and contain repetitions. 
#' 
#' `vapour_vrt()` is vectorized, it will return multiple VRT strings for multiple inputs in 
#' a "length > 1" character vector. These are all independent, this is different to the function
#' `vapour_warp_raster()` where multiple inputs are merged (possibly by sequential overlapping).  
#' @section Rationale:
#' 
#' For a raster, the basic essentials we can specify or modify for a source  are
#' 1) the source, 2) the extent, 3) the projection 4) what subdataset (these are
#' variables from NetCDF and the like that contain multiple datasets) and 5)
#' which band/s to provided. For extent and projection we are simply providing
#' or correcting complete information about how to interpret the georeferencing,
#' with subdatasets and bands this is more like a query of which ones we want.
#' If we only wanted band 5, then the output data would have one band only (and
#' we we read it we need `band = 1`). 
#' 
#' We don't provide ability override the dimension, but that is possible as
#' well. More features may come with a 'VRTBuilder' interface.
#' 
#' @section Projections:
#' Common inputs for `projection` are WKT variants, "AUTH:CODE"s e.g.
#' "EPSG:3031", the "OGC:CRS84" for long,lat WGS84, "ESRI:code" and other
#' authority variants, and datum names such as 'WGS84','NAD27' recognized by
#' PROJ itself.
#'
#' See the following links to GDAL and PROJ documentation: 
#' 
#' [PROJ documentation: c.proj_create_crs_to_crs](https://proj.org/development/reference/functions.html#c.proj_create_crs_to_crs) 
#' 
#' [PROJ documentation: c.proj_create](https://proj.org/development/reference/functions.html#c.proj_create)
#' 
#' [GDAL documentation: SetFromUserInput](https://gdal.org/doxygen/classOGRSpatialReference.html#aec3c6a49533fe457ddc763d699ff8796)
#'  
#' @param x data source name, filepath, url, database connection string, or VRT text
#' @param extent (optional) numeric extent, xmin,xmax,ymin,ymax
#' @param projection (optional) character string, projection string ("auth:code", proj4, or WKT, or anything understood by PROJ, see Details)
#' @param bands (optional) which band/s to include from the source
#' @param sds which subdataset to select from a source with more than one
#' @param ... ignored
#' @param relative_to_vrt default `FALSE`, if `TRUE` input strings that identify as files on the system are left as-is (by default they are made absolute at the R level)
#' @param geolocation vector of 2 dsn to longitude, latitude geolocation array sources
#' @return VRT character string (for use by GDAL-capable tools, i.e. reading raster)
#' @export
#'
#' @examples
#' tif <- system.file("extdata", "sst.tif", package = "vapour", mustWork = TRUE)
#' vapour_vrt(tif)
#' 
#' vapour_vrt(tif, bands = c(1, 1))
#' 
vapour_vrt <- function(x, extent = NULL, projection = NULL,  sds = 1L, bands = NULL, geolocation = NULL, ..., relative_to_vrt = FALSE) {
  x <- .check_dsn_multiple_naok(x)
  
  if (!relative_to_vrt) { 
    ## GDAL does its hardest to use relative paths, so either implemente Even's proposal or force alway absolute (but allow turn that off)
    ## https://lists.osgeo.org/pipermail/gdal-dev/2022-April/055739.html
    ## relativeToVRT doesn't really make sense for an in-memory VRT string, so depends what you're doing and we'll review over time :)
    fe <- file.exists(x)
    if (any(fe)) x[fe] <- normalizePath(x[fe])
  }
  if (is.character(sds)) {
    sdsnames <- vapour_sds_names(x)
    #browser()
    sds <- which(unlist(lapply(sdsnames, function(.x) grepl(sprintf("%s$", sds[1L]), .x))))[1]
  }
  if (is.na(sds[1]) || length(sds) > 1L || !is.numeric(sds) || sds < 1) {
    stop("'sds' must be a valid name or integer for a GDAL-subdataset (1-based)")
  }
  
  ## FIXME: we can't do bands atm, also what else does boilerplate checks do that the new C++ in gdalraster doesn't do?
   # x <- sds_boilerplate_checks(x, sds)
   # if (!is.null(bands))  {
   #   version <- substr(vapour::vapour_gdal_version(), 6, 8)
   #   if (version < 3.1) {
   #     message("gdal 3.1 or above required for 'bands', ignoring")
   #   } else {
   #     srcbands <- vapour_raster_info(x)$bands
   #     if (any(bands) > srcbands) stop(sprintf("source has only %i bands" ,srcbands))
   #   x <- sprintf("vrt://%s?bands=%s", x, paste0(bands, collapse = ","))
   #   }
   # }
  if (is.null(extent)) {
    extent <- 1.0
  }

  if (is.null(geolocation)) {
    geolocation <- ""
  }
   if (!is.numeric(extent)) {
    #message("vapour_vrt(): extent must be a valid numeric vector of xmin,xmax,ymin,ymax")
    extent <- 1.0
  }
  if (is.null(projection) || is.na(projection)) {
    projection <- ""
  }
  if (!is.character(projection)) {
    message("vapour_vrt(): projection must a valid projection string for PROJ")
    projection <- ""
  }
  if (is.null(bands) || !is.numeric(bands)) {
    bands <- 0
  }
  if (length(sds) < 1 || !is.numeric(sds) || is.null(sds) || anyNA(sds)) {
    sds <- 1L
  }
  if (!is.null(bands) && (length(bands) < 1 || !is.numeric(bands) || anyNA(bands))) {
    bands <- 1L
  }
   if (!is.null(geolocation) && (length(geolocation) != 2 || !is.character(geolocation) || anyNA(geolocation))) {
    geolocation <- ""
  }
  raster_vrt_cpp(x, extent, projection[1L], sds, bands, geolocation)
}

  
