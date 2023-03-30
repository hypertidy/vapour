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
  
  version <- vapour_gdal_version()
  v3 <- TRUE
  if (grepl("GDAL 2", version )) v3 <- FALSE
  extra <- c(if(json) "-json",
             if (is.numeric(sd) && sd[1L] > 0) c("-sd", sd[1L]),
             if (stats) "-stats",
             if (checksum) "-checksum",
             if (nchar(wkt_format[1]) > 0 && v3) c("-wkt_format", wkt_format[1L]),
             if (length(oo) > 0 && any(nchar(oo) > 0) ) rep_zip("-oo", oo[nchar(oo) > 0]),
             if (length(initial_format) > 0 && any(nchar(initial_format) > 0)) rep_zip("-if", initial_format[nchar(initial_format) > 0]))
  
  options <- c(options, "-proj4", "-listmdd", extra)
  options <- options[!is.na(options)]  ## cant do unique because repeated arguments possible things like "-if" "GTiff" "-if" "netCDF"
  
  info <- raster_gdalinfo_app_cpp(x, options)
  info
}


