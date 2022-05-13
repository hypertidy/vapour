## this is ongoing work to replace the innards of GDALInfo for vapour_raster_info and others
## for example, instead of
## f1 <- system.file("extdata/gcps", "volcano_gcp.tif", package = "vapour")
## vapour_raster_gcp(f1)
## we can now do
## jsonlite::fromJSON(vapour:::gdalinfo_internal(f1))$gcps
gdalinfo_internal <- function(x, json = TRUE,
                                  stats = FALSE,
                                  sd = 0,
                                  checksum = FALSE,
                                  wkt_format = "WKT2",
                                  oo = character(),
                                  initial_format = character(),
                                  options = character()) {
  
  rep_zip <- function(x, y) {
    as.vector(t(cbind(rep(x, length(y)), y)))
  }
  if (length(sd) > 1) message("'sd' argument cannot be vectorized over 'dsn', ignoring all but first value")
  
  extra <- c(if(json) "-json",
             if (is.numeric(sd) && sd[1L] > 0) c("-sd", sd[1L]),
             if (stats) "-stats",
             if (checksum) "-checksum",
             if (nchar(wkt_format[1]) > 0) c("-wkt_format", wkt_format[1L]),
             if (length(oo) > 0 && any(nchar(oo) > 0) ) rep_zip("-oo", oo[nchar(oo) > 0]),
             if (length(initial_format) > 0 && any(nchar(initial_format) > 0)) rep_zip("-if", initial_format[nchar(initial_format) > 0]))
  
  options <- c(options, "-proj4", "-listmdd", extra)
  options <- options[!is.na(options)]  ## cant do unique because repeated arguments possible things like "-if" "GTiff" "-if" "netCDF"
  
  info <- raster_gdalinfo_app_cpp(x, options)
  info
}

new_vapour_raster_info <- function(x, ..., sds = NULL, min_max = FALSE) {
  sd <- if (is.null(sds)) 0 else sds
  info <- gdalinfo_internal(x[1L], json  = TRUE, stats = min_max, sd = sd, ...)
  json <- jsonlite::fromJSON(info)
  list(geotransform = json$geoTransform, 
       dimension = json$size,  ## or/and dimXY
       ## this needs to be per band
       minmax = c(json$bands$min[1L], json$bands$max[1L]), 
       block = json$bands$block[[1L]],  ## or/and dimXY
       projection = json$coordinateSystem$wkt, 
       bands = dim(json$bands)[1L], 
       projstring = json$coordinateSystem$proj4, 
       nodata_value = json$bands$noDataValue[1L], 
       overviews = unlist(json$bands$overviews[[1]], use.names = FALSE), ## NULL if there aren't any (was integer(0)), 
       filelist = json$files, 
       datatype = json$bands$type[1L], 
       extent = c(json$cornerCoordinates$upperLeft, json$cornerCoordinates$lowerRight)[c(1, 3, 4, 2)])
}
