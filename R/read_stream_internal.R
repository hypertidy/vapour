gdal_ptrs <-  function(dsource, layer) {
    layer <-  if (missing(layer)) {
    character()
  } else {
    enc2utf8(layer)
  }
  #gdal_ptrs_cpp(dsource, layer)
}

gdal_vector_data <-  function(dsource, layer, ..., sql = NA, options = NULL, quiet = FALSE,
                        fid_column_name = character(0),
                        if_drivers = character(0),
                         extent = NA,
                         optional = FALSE, return_stream = FALSE) {

  ## vapourize this
  layer <-  if (missing(layer)) {
    character()
  } else {
    enc2utf8(layer)
  }
  if (nchar(dsource) < 1L) {
    stop("`dsour` must describe a valid source description for GDAL (input was an empty string).", call. = FALSE)
  }
  dsn_exists <-  file.exists(dsource)

  if (length(dsource) == 1 && dsn_exists) {
    dsource <- enc2utf8(normalizePath(dsource))
  }


    stream <- nanoarrow::nanoarrow_allocate_array_stream()

     info <- gdal_dsn_read_vector_stream(stream,
                                         dsource, layer, sql, as.character(options), quiet,
                                 if_drivers, extent, dsn_exists,  fid_column_name, 80L)
    
     if (return_stream) return(structure(list(stream), crs = info[[1]], num_features = info[[2]]))
     #
    crs <- info[[1L]]
    if (info[[2L]] == -1) {
      num_features = NULL
    }


    d <-  nanoarrow::convert_array_stream(stream, num_features)
    d$wkb_geometry <- wk::wkb(d$wkb_geometry, crs = crs)
    d
}



