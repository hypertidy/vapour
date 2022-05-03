











#' Virtual raster 
#' 
#' Simple VRT creation of a GDAL virtual raster. 
#' 
#' Create a GDAL data source string (to be used like a filename) with various helpers. 
#' VRT stands for 'ViRTual'. 
#'
#' An input string will be converted to a single subdataset, use 'sds' argument to select. 
#'
#' If 'extent', 'projection' is provided this is applied to override the source's extent and/or projection. 
#' (These might be invalid, or missing, so we facilitate correcting this). 
#' 
#' If 'bands' is provided this is used to select a set of bands (numbered from 1), which might be repeated, or
#' in any order.
#' 
#' Common inputs for `projection` are WKT variants, 'AUTH:CODE's e.g.
#' 'EPSG:3031', the 'OGC:CRS84' for long,lat WGS84, 'ESRI:<code>' and other
#' authority variants, and datum names such as 'WGS84','NAD27' recognized by
#' PROJ itself.
#'
#' See help for 'SetFromUserInput' in 'OGRSpatialReference', and 'proj_create_crs_to_crs'.
#' 
#' [c.proj_create_crs_to_crs](https://proj.org/development/reference/functions.html#c.proj_create_crs_to_crs) 
#' 
#' [c.proj_create](https://proj.org/development/reference/functions.html#c.proj_create)
#' 
#' [SetFromUserInput](https://gdal.org/doxygen/classOGRSpatialReference.html#aec3c6a49533fe457ddc763d699ff8796)
#'  
#' @param dsn data source name, filepath, url, database connection string, or VRT text
#' @param extent (optional) numeric extent, xmin,xmax,ymin,ymax
#' @param projection (optional) character string, projection string ("auth:code", proj4, or WKT, or anything understood by PROJ, see Details)
#' @param bands (optional) which band/s to include from the source
#' @param sds which subdataset to select from a source with more than one
#' @param ... ignored
#' @param relative_to_vrt default `FALSE`, if `TRUE` input strings that identify as files on the system are left as-is (by default they are made absolute at the R level)
#'
#' @return VRT character string (for use by GDAL-capable tools, i.e. reading raster)
#' @export
#'
#' @examples
#' tif <- system.file("extdata", "sst.tif", package = "vapour", mustWork = TRUE)
#' vapour_vrt(tif)
#' 
#' vapour_vrt(tif, bands = c(1, 1))
#' 
vapour_vrt <- function(dsn, extent = NULL, projection = NULL,  sds = 1L, bands = NULL, ..., relative_to_vrt = FALSE) {
  if (!relative_to_vrt) { 
    ## GDAL does its hardest to use relative paths, so either implemente Even's proposal or force alway absolute (but allow turn that off)
    ## https://lists.osgeo.org/pipermail/gdal-dev/2022-April/055739.html
    ## relativeToVRT doesn't really make sense for an in-memory VRT string, so depends what you're doing and we'll review over time :)
    fe <- file.exists(dsn)
    if (any(fe)) dsn[fe] <- normalizePath(dsn[fe])
  }
  ## FIXME: we can't do bands atm, also what else does boilerplate checks do that the new C++ in gdalraster doesn't do?
   # dsn <- sds_boilerplate_checks(dsn, sds)
   # if (!is.null(bands))  {
   #   version <- substr(vapour::vapour_gdal_version(), 6, 8)
   #   if (version < 3.1) {
   #     message("gdal 3.1 or above required for 'bands', ignoring")
   #   } else {
   #     srcbands <- vapour_raster_info(dsn)$bands
   #     if (any(bands) > srcbands) stop(sprintf("source has only %i bands" ,srcbands))
   #   dsn <- sprintf("vrt://%s?bands=%s", dsn, paste0(bands, collapse = ","))
   #   }
   # }
  if (is.null(extent)) {
    extent <- 1.0
  }
  if (!is.numeric(extent)) {
    #message("vapour_vrt(): extent must be a valid numeric vector of xmin,xmax,ymin,ymax")
    extent <- 1.0
  }
  if (!length(extent) == 1 || !length(extent) == 4) {
   # message("vapour_vrt(): extent must be a valid numeric vector of xmin,xmax,ymin,ymax")
  }
  
  if (is.null(projection)) {
    projection <- ""
  }
  if (!is.character(projection)) {
    message("vapour_vrt(): projection must a valid projection string for PROJ")
    projection <- ""
  }
  if (is.null(bands)) {
    bands <- 0
  }
  raster_vrt_cpp(dsn, extent, projection[1L], sds, bands)
}


vapour_warp_vrt <- function(dsn, extent = NULL, projection = NULL, bands = NULL, sds = 1L) {
  if (file.exists(dsn)) {
    dsn <- path.expand(dsn)
  }
  
  
  
}
