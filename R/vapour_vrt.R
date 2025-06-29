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
#' 
#' If `geolocation` is set the 'GeoTransform' element is forcibly removed from the vrt output, in order
#' to avoid https://github.com/hypertidy/vapour/issues/210 (there might be a better fix). 
#' 
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
#' [GDAL documentation: SetFromUserInput](https://gdal.org/en/stable/doxygen/classOGRSpatialReference.html)
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
#' @param nomd if `TRUE` the Metadata tag is removed from the resulting VRT (it can be quite substantial)
#' @param options pass in options to the VRT creation, like 'c("-expand", "rgb", "-ot", "Byte"('
#' @param overview  pick an integer overview from the source (0L is highest resolution, default -1L does nothing)
#'
#' @export
#'
#' @examples
#' tif <- system.file("extdata", "sst.tif", package = "vapour")
#' vapour_vrt(tif)
#' 
#' vapour_vrt(tif, bands = c(1, 1))
#' 
vapour_vrt <- function(x, extent = NULL, projection = NULL,  sds = 1L, bands = NULL, geolocation = NULL, ..., 
                       relative_to_vrt = FALSE, nomd = FALSE, overview = -1L, options = character()) {
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
  if (is.null(overview)) overview <- -1L
  overview <- as.integer(overview[1])
  if (is.na(overview)) overview <- -1L

  
  # 
  # CharacterVector dsn, 
  # NumericVector extent, 
  # CharacterVector projection, 
  # IntegerVector sds, 
  # IntegerVector bands, 
  # CharacterVector geolocation, 
  # LogicalVector nomd, 
  # IntegerVector overview, 
  # CharacterVector options
  # 
  # 
  

  out <- raster_vrt_cpp(x, extent, projection[1L], sds, bands, geolocation, nomd, overview, options)
  ## scrub any transform, because of #210
  if (nzchar(geolocation[1L])) {
    out <- stringr::str_replace(out, "<GeoTransform.*GeoTransform>", "")
  }
  out
}



#' Vector VRT
#'
#' Just a simple text generator to generate the VRT for a vector layer, First layer is chosen if not
#' otherwise specified. 
#' 
#' Using 'sql' overrides the 'layer', and using 'projection' results in the geometries being transformed. 
#' No check is made of the layer source projection. 
#' 
#' Use 'a_srs' to ensure the source has a source crs (that might be the only thing you use this for, even if not reprojecting). 
#' 
#' It's expected that if you use this with a source without a source projection,
#' you'll get "Failed to import source SRS", so use argument "a_srs" to set it
#' if needed (or many other GDAL other facilities that do this).
#' @param x data source name
#' @param layer layer index (1-based) or name
#' @param projection crs of the output
#' @param sql SQL for ExecuteSQL to define the layer
#' @param a_srs set the source crs
#' @return single element character vector
#' @export
#'
#' @examples
#' file <- "list_locality_postcode_meander_valley.tab"
#' ## A MapInfo TAB file with polygons
#' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
#' vector_vrt(mvfile, sql = "SELECT * FROM list_locality_postcode_meander_valley LIMIT 5 OFFSET 4")
#' 
#' ## read this with vapour_read_geometry() and it will be projected to VicGrid
#' vector_vrt(mvfile, projection = "EPSG:3111")
#' 
vector_vrt <- function(x, layer = 1L, projection = NULL, sql = NULL, a_srs = NULL) {
 
  if(!is.character(layer)) {
    layer <- vapour::vapour_layer_names(x)[layer]
  }
  layer_srs <- ""
  if (!is.null(a_srs)) {
    layer_srs <- sprintf('<LayerSRS>%s</LayerSRS>', a_srs)
  }
  if (is.null(sql)) {
    
    
    layer <- sprintf('<OGRVRTLayer name="%s">
    <SrcDataSource>%s</SrcDataSource>
    <SrcLayer>%s</SrcLayer> %s
    </OGRVRTLayer>', layer, x, layer, layer_srs)
  } else {
    layer <- sprintf('<OGRVRTLayer name="%s">
    <SrcDataSource>%s</SrcDataSource>
    <SrcSQL>%s</SrcSQL> %s
    </OGRVRTLayer>', layer, x, sql, layer_srs)
  }
  
  
  if (file.exists(x)) {
    x <- normalizePath(x)
  }
  
  if (is.null(projection)) {
    out <- sprintf('<OGRVRTDataSource>
     %s
 </OGRVRTDataSource>', layer)
    
  } else {
    out <- sprintf('<OGRVRTDataSource>
  <OGRVRTWarpedLayer>
     %s
    <TargetSRS>%s</TargetSRS>
    </OGRVRTWarpedLayer>
 </OGRVRTDataSource>', layer, projection)
  }
  out
}


#' Build vrt, special case "-separate"
#'
#' @param dsn one or more raster sources
#'
#' @return a character string of the built vrt, multiple sources treated as bands
#' @export
#'
#' @examples
#' f <- system.file("extdata/sst.tif", package = "vapour", mustWork = TRUE)
#' vrt <- buildvrt(c(f, vapour_vrt(f)))
#' writeLines(vrt)
buildvrt <- function(dsn) {
   options = c("-separate")
   raster_buildvrt_cpp(dsn, options)
 } 
