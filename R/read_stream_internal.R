.read_stream <-  function(dsn, layer, ..., sql = NA, options = NULL, quiet = FALSE, 
                        fid_column_name = character(0),
                        drivers = character(0), ## replace with vrt://..?if=
                         wkt_filter = character(0), ## extent
                         optional = FALSE, return_stream = FALSE) {
  
  ## vapourize this 
  layer <-  if (missing(layer)) {
    character()
  } else {
    enc2utf8(layer)
  }
  if (nchar(dsn) < 1L) {
    stop("`dsn` must describe a valid data source name for GDAL (input wasan empty string).", call. = FALSE)
  }
  dsn_exists <-  file.exists(dsn)  ## good to see this finally stuck in sf

  if (length(dsn) == 1 && dsn_exists) {
    dsn = enc2utf8(normalizePath(dsn))
  }
  

    stream = nanoarrow::nanoarrow_allocate_array_stream()

    info = gdal_dsn_read_vector_stream(stream, dsn, layer, sql, as.character(options), quiet,
                                drivers, wkt_filter, dsn_exists, dsn_isdb  = FALSE, fid_column_name, 80L)

    browser()
    if (return_stream) return(stream)
    ##// layer has been freed as this point
    # geometry_column <- unlist(lapply(
    #   stream$get_schema()$children, function(s) identical(s$metadata[["ARROW:extension:name"]], "ogc.wkb")
    # ))
    crs <- info[[1L]]
    if (info[[2L]] == -1) {
      num_features = NULL
    }
    list(data = suppressWarnings(nanoarrow::convert_array_stream(stream,  size = num_features)), crs = crs)
}



