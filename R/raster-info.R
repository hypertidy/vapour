sds_boilerplate_checks <- function(x, sds = NULL) {

  ## don't pass relative paths to GDAL
  ## but also don't prevent access to non-files
  if (file.exists(x)) x <- base::normalizePath(x, mustWork = FALSE)
  ## use sds wrapper to target the first by default
  subdatasets <- try(vapour_sds_names(x), silent = TRUE)
  if (inherits(subdatasets, "try-error")) stop("GDAL was unable to open ^^")
  datavars <- data.frame(datsource = x, subdataset = subdatasets, stringsAsFactors = FALSE)

  ## catch for l1b where we end up with the GCP conflated with the data set #48
  if (nrow(datavars) < 2) return(x)  ## shortcut to avoid #48
  wasnull <- is.null(sds)
  if (wasnull) sds <- 1
  if (!is.numeric(sds)) stop("sds must be specified by number, starting from 1")
  if (wasnull && nrow(datavars) > 1L) {
    varnames <- unlist(lapply(strsplit(datavars$subdataset, ":"), utils::tail, 1L), use.names = FALSE)
    message(sprintf("subdataset (variable) used is '%s'\n", varnames[1]))
    message("If that is not correct (or to suppress this message) choose 'sds' by number from ",
      paste(sprintf("\n%i: '%s'", seq_len(nrow(datavars)), varnames), collapse = ", "))
  }
  stopifnot(length(sds) == 1L)

  if (sds < 1) stop("sds must be 1 at minimum")
  if (sds > nrow(datavars)) stop(sprintf("'sds' must not exceed number of subdatasets (%i)", nrow(datavars)))
  datavars$subdataset[sds]
}


.gt_dim_to_extent <- function (x, dim)
{
  xx <- c(x[1], x[1] + dim[1] * x[2])
  yy <- c(x[4] + dim[2] * x[6], x[4])
  c(xx, yy)
}
#' Raster information
#'
#' Return the basic structural metadata of a raster source understood by GDAL.
#' Subdatasets may be specified by number, starting at 1. See
#' [vapour_sds_names] for more.
#'
#' The structural metadata are
#'
#' \describe{
#' \item{extent}{the extent of the data, xmin, xmax, ymin, ymax - these are the lower left and upper right corners of pixels}
#' \item{geotransform}{the affine transform}
#' \item{dimension}{dimensions x-y, columns*rows}
#' \item{minmax}{numeric values of the computed min and max from the first band (optional)}
#' \item{block}{dimensions x-y of internal tiling scheme}
#' \item{projection}{text version of map projection parameter string}
#' \item{bands}{number of bands in the dataset}
#' \item{projstring}{the proj string version of 'projection'}
#' \item{nodata_value}{not implemented}
#' \item{overviews}{the number and size of any available overviews}
#' \item{filelist}{the list of files involved (may be none, and so will be a single NA character value)}
#' \item{datatype}{the band type name, in GDAL form 'Byte', 'Int16', 'Float32', etc.}
#' \item{subdatasets}{any subdataset DSNs is present, otherwise `NULL` }
#' \item{corners}{corner coordinates of the data, for non-zero skew geotransforms a 2-column matrix with rows upperLeft, lowerLeft, lowerRight, upperRight, and center}
#' }
#'
#' Note that the geotransform is a kind of obscure combination of the extent and dimension, I don't find it
#' useful and modern GDAL is moving away from needing it so much. Extent is more sensible and used in many places in
#' a straightforward way.
#'
#' On access vapour functions will report on the existence of subdatasets while
#' defaulting to the first subdataset found.
#'
#' @section Subdatasets:
#'
#' Some sources provide multiple data sets, where a dataset is described by a 2-
#' (or more) dimensional grid whose structure is described by the metadata
#' described above. Note that _subdataset_ is a different concept to _band or
#' dimension_. Sources that may have multiple data sets are HDF4/HDF5 and
#' NetCDF, and they are loosely analogous to the concept of *layer* in GDAL
#' vector data. Variables are usually seen as distinct data but in GDAL and
#' related 2D-interpretations this concept is leveraged as a 3rd dimension (and
#' higher). In a GeoTIFF a third dimension might be implicit across bands, i.e.
#' to express time varying data and so each band is not properly a variable.
#' Similarly in NetCDF, the data may be any dimensional but there's only an
#' implicit link for other variables that exist in that same dimensional space.
#' When using GDAL you are always traversing this confusing realm.
#'
#' If subdatasets are present but not specified the first is queried. The choice
#' of subdataset is analogous to the way that the `raster` package behaves, and
#' uses the argument `varname`. Variables in NetCDF correspond to subdatasets,
#' but a single data set might have multiple variables in different bands or in
#' dimensions, so this guide does not hold across various systems.
#'
#' @section The Geo Transform:
#'
#' From \url{https://gdal.org/en/stable/user/raster_data_model.html}.
#'
#' The affine transform consists of six coefficients returned by
#' `GDALDataset::GetGeoTransform()` which map pixel/line coordinates into
#' georeferenced space using the following relationship:
#'
#'   \code{Xgeo = GT(0) + Xpixel*GT(1) + Yline*GT(2)}
#'
#'   \code{Ygeo = GT(3) + Xpixel*GT(4) + Yline*GT(5)}
#'
#'
#' They are
#'  \describe{
#' \item{GT0, xmin}{the x position of the lower left corner of the lower left pixel}
#' \item{GT1, xres}{the scale of the x-axis, the width of the pixel in x-units}
#' \item{GT2, yskew}{y component of the pixel width}
#' \item{GT3, ymax}{the y position of the upper left corner of the upper left pixel}
#' \item{GT4, xskew}{x component of the pixel height}
#' \item{GT5, yres}{the scale of the y-axis, the height of the pixel in *negative* y-units}
#' }
#'
#' Please note that these coefficients are equivalent to the contents of a
#' *world file* but that the order is not the same and the world file uses cell
#' centre convention rather than edge.
#' \url{https://en.wikipedia.org/wiki/World_file}
#'
#' Usually the skew components are zero, and so only four coefficients are
#' relevant and correspond to the offset and scale used to position the raster -
#' in combination with the number of rows and columns of data they provide the
#' spatial extent and the pixel size in each direction.  Very rarely a an actual
#' affine raster will be use with this _rotation_ specified within the transform
#' coefficients.
#'
#' Calculation of 'minmax' can take a significant amount of time, so it's not done by default. Use
#' 'minmax = TRUE' to do it. (It does perform well, but may be prohibitive for very large or remote sources.)
#' @seealso vapour_sds_info
#'
#' @section Overviews:
#'
#' If there are no overviews this element will simply be a single-element vector
#' of value 0. If there are overviews, the first value will give the number of overviews and
#' their dimensions will be listed as pairs of x,y values.
#'
#' @param x data source string (i.e. file name or URL or database connection string)
#' @param sds a subdataset number, if necessary
#' @param min_max logical, control computing min and max values in source ('FALSE' by default)
#' @param ... currently unused
#'
#' @return list with vectors 'geotransform', 'dimXY', 'minmax', 'tilesXY', 'projection', 'bands', 'proj4', 'nodata_value',
#'  'overviews', 'filelist' see sections in Details for more on each element
#' @export
#' @examples
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' vapour_raster_info(f)
vapour_raster_info <- function(x, ..., sds = NULL, min_max = FALSE) {
  sd <- if (is.null(sds)) 0 else sds
  x <- .check_dsn_single(x)
  info <- gdalinfo_internal(x[1L], json  = TRUE, stats = min_max, sd = sd, ...)
  if (is.na(info)) {
    stop("GDAL was unable to open ^^")
  }
  json <- jsonlite::fromJSON(info)
  sds <- NULL
  if (!is.null(json$metadata$SUBDATASETS)) {
    sds <- unlist(json$metadata$SUBDATASETS[grep("NAME$", names(json$metadata$SUBDATASETS))], use.names = FALSE)
  }
  
  corners <- NULL
  extent <- NULL
  if (!is.null(json$cornerCoordinates)) {
    corners <- do.call(rbind, json$cornerCoordinates)
    extent <- c(range(corners[,1]), range(corners[,2]))
  }


  if (is.null(json$geoTransform) && !is.null(extent)) {
    geoTransform <- c(extent[1], diff(extent[c(1,2)])/json$size[1], 0, 
                      extent[4], 0, diff(extent[c(4:3)])/json$size[2])
  } else {
    geoTransform <- json$geoTransform
  }

  list(geotransform = geoTransform, 
       dimension = json$size,  ## or/and dimXY
       dimXY = json$size,
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
       extent = extent, 
       subdatasets = sds, 
       corners = corners)
}


old_vapour_raster_info <- function(x, ..., sds = NULL, min_max = FALSE) {
  x <- .check_dsn_single(x)
  datasourcename <- sds_boilerplate_checks(x, sds = sds)
  info <- raster_info_gdal_cpp(dsn = datasourcename, min_max = min_max)
  info[["extent"]] <- .gt_dim_to_extent(info$geotransform, info$dimXY)
  info
}

#' Raster ground control points
#'
#' Return any ground control points for a raster data set, if they exist.
#'
#' Pixel and Line coordinates do not correspond to cells in the underlying raster grid, they
#' refer to the index space of that array in 0, ncols and 0, nrows. They are usually a subsample of
#' the grid and may not align with the grid spacing itself (though they often do in satellite remote sensing products).
#'
#' The coordinate system of the GCPs is currently not read.
#'
#' @param x data source string (i.e. file name or URL or database connection string)
#' @param ... ignored currently
#'
#' @return list with
#' \itemize{
#'  \item \code{Pixel} the pixel coordinate
#'  \item \code{Line} the line coordinate
#'  \item \code{X} the X coordinate of the GCP
#'  \item \code{Y} the Y coordinate of the GCP
#'  \item \code{Z} the Z coordinate of the GCP (usually zero)
#' }
#' @export
#' @examples
#' ## this file has no ground control points
#' ## they are rare, and tend to be in large files
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' vapour_raster_gcp(f)
#'
#' ## a very made-up example with no real use
#' f1 <- system.file("extdata/gcps", "volcano_gcp.tif", package = "vapour")
#' vapour_raster_gcp(f1)
#'
vapour_raster_gcp <- function(x, ...) {
  x <- .check_dsn_single(x)
  if (file.exists(x)) x <- normalizePath(x)
  raster_gcp_gdal_cpp(x)
}
#' GDAL raster subdatasets (variables)
#'
#' A **subdataset** is a collection abstraction for a number of **variables**
#' within a single GDAL source. If there's only one variable the datasource and
#' the variable have the same data source string. If there is more than one the
#' subdatasets have the form **DRIVER:"datasourcename":varname**. Each
#' subdataset name can stand in place of a data source name that has only one
#' variable, so we always treat a source as a subdataset, even if there's only
#' one.
#'
#' Returns a character vector of 'subdatasets`. In the case of a normal data
#' source, with no subdatasets the value is simply  the `datasource`.
#'
#' If the raw SDS names contain spaces these are replaced by '%20' escape strings. A specific example is  
#' "WCS:https://elevation.nationalmap.gov:443" with request
#' "arcgis/services/3DEPElevation/ImageServer/WCSServer?version=2.0.1&coverage=DEP3Elevation_Hillshade Gray". 
#' This function will return "..DEP3Elevation_Hillshade%20Gray".  
#' See [wiki post](https://github.com/hypertidy/vapour/wiki/Examples-of-subdatasets) for more details. 
#' 
#' @param x a data source string, filename, database connection string,  or other URL
#'
#' @return character vector of subdataset names, or just the source itself if no SDS are present
#' @export
#'
#' @examples
#' f <- system.file("extdata/gdal", "sds.nc", package = "vapour")
#' ## protect from error with netcdf problems
#' result <- try(vapour_sds_names(f), silent = TRUE)
#' if (!inherits(result, "try-error")) {
#'   print(result)
#' }
#' vapour_sds_names(system.file("extdata", "sst.tif", package = "vapour"))
#' 
vapour_sds_names <- function(x) {
  x <- .check_dsn_single(x)
  info <- gdalinfo_internal(x[1L], json  = TRUE)

  if (is.na(info)) stop("GDAL was unable to open ^^")
  json <- jsonlite::fromJSON(info)
  if (!is.null(json$metadata$SUBDATASETS)) {
   sources <- unlist(json$metadata$SUBDATASETS[grep("NAME$", 
                                                    names(json$metadata$SUBDATASETS))], use.names = FALSE)
  } else {
    sources <- x[1L]  ## should return 0-vector, or NULL I think
  }
 sources
}

#' Retrieve geolocation information for a dataset
#' 
#' Value is a named vector in a list. 
#' 
#' If no geolocation exist the return value is an empty list. 
#'
#' @inheritParams vapour_raster_info
#' 
#' @return list with a single character vector
#' @export
#'
#' @examples
#' drivers <- vapour_all_drivers()
#' ok <- drivers$raster[ drivers$driver == "netCDF"]
#' if (isTRUE(ok) && interactive()) {
#'  vapour_geolocation(system.file("extdata/gdal/geos_rad.nc", package = "vapour"), 0L)
#' }
vapour_geolocation <- function(x, sds = NULL) {
  sd <- if (is.null(sds)) 0L else as.integer(sds[1L])
  if (is.na(sd) || length(sd) < 1L) sd <- 0L
  x <- .check_dsn_single(x)
  raster_list_geolocation_gdal_cpp(x, as.integer(sds)[1L])
}
