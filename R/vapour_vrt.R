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
#' 
#' 
#'
#' @param dsn data source name, filepath, url, database connnection string, or VRT text
#' @param extent (optional) numeric extent, xmin,xmax,ymin,ymax
#' @param projection (optional) character string, projection string ("auth:code", proj4, or WKT, or anything understood by PROJ)
#' @param bands (optional) which band/s to include from the source
#' @param sds which subdataset to select from a source with more than one
#'
#' @return
#' @export
#'
#' @examples
#' tif <- system.file("extdata", "sst.tif", package = "vapour")
#' vapour_vrt(tif)
#' 
#' vapour_vrt(tif, bands = c(1, 1))
vapour_vrt <- function(dsn, extent = NULL, projection = NULL, bands = NULL, sds = 1L) {
  if (file.exists(dsn)) {
    dsn <- path.expand(dsn)
  }
  
   dsn <- sds_boilerplate_checks(dsn, sds)
   if (!is.null(bands))  {
     version <- substr(vapour::vapour_gdal_version(), 6, 8)
     if (version < 3.1) {
       message("gdal 3.1 or above required for 'bands', ignoring")
     } else {
       srcbands <- vapour_raster_info(dsn)$bands
       if (any(bands) > srcbands) stop(sprintf("source has only %i bands" ,srcbands))
     dsn <- sprintf("vrt://%s?bands=%s", dsn, paste0(bands, collapse = ","))
     }
   }
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
  raster_vrt_cpp(dsn, extent, projection[1L])
}
