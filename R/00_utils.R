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
                                  options = c("-proj4", "-json", "-listmdd")) {
  
  if (length(sd) > 1) message("'sd' argument cannot be vectorized over 'dsn', ignoring all but first value")
  sd <- suppressWarnings(as.integer(sd[1L]))
  if (length(sd) < 1L || is.na(sd)) sd <- 0L
  options <- options[!is.na(options)]  ## cant do unique because repeated arguments possible things like "-if" "GTiff" "-if" "netCDF"
  ## -mm computes the min and max of each band, and unlike -stats it does not save them beside the source
  if (isTRUE(stats) && !"-mm" %in% options) options <- c(options, "-mm")
  ## one string for each source, NA where a source cannot be opened (GDAL's reason is a warning)
  raster_gdalinfo_app_cpp(x, options, sd, .check_open_options(oo))
}

