sds_boilerplate_checks <- function(x, sds = NULL) {
  ## don't pass relative paths to GDAL
  ## but also don't prevent access to non-files
  if (file.exists(x)) x <- base::normalizePath(x, mustWork = FALSE)
  ## use sds wrapper to target the first by default
  datavars <- as.data.frame(vapour_sds_names(x), stringsAsFactors = FALSE)
  wasnull <- is.null(sds)
  if (wasnull) sds <- 1
  if (wasnull && nrow(datavars) > 1L) {
    varnames <- unlist(lapply(strsplit(datavars$subdataset, ":"), utils::tail, 1L))
    message("subdataset (variable) used is %s (1)", varnames[1])
    message("If that is not correct, set it to one of ", paste(sprintf("%i (%s)", seq_len(nrow(datavars))[-1], varnames[-1]), collapse = ", "))
  }
  stopifnot(length(sds) == 1L)

  if (sds < 1) stop("sds must be 1 at minimum")
  if (sds > nrow(datavars)) stop(sprintf("'sds' must not exceed number of subdatasets (%i)", nrow(datavars)))
  datavars$subdataset[sds]
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
#' \item{geotransform}{the affine transform}
#' \item{dimXY}{dimensions x-y, columns*rows}
#' \item{minmax}{range of data values}
#' \item{tilesXY}{dimensions x-y of internal tiling scheme}
#' \item{projection}{text version of map projection parameter string}
#' }
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
#' From \url{http://www.gdal.org/gdal_datamodel.html}.
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
#' \item{GT3, ymin}{the y position of the upper left corner of the upper left pixel}
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
#' @seealso vapour_sds_info
#' @param x data source string (i.e. file name or URL or database connection string)
#' @param sds a subdataset number, if necessary
#' @param ... currently unused
#' @export
#' @examples
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' vapour_raster_info(f)
vapour_raster_info <- function(x, ..., sds = NULL) {
  datasourcename <- sds_boilerplate_checks(x, sds = sds)
  raster_info_cpp(pszFilename = datasourcename)
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
#' Returns a list of `datasource` and `subdataset`. In the case of a normal data
#' source, with no subdatasets the value of both entries is the `datasource`.
#'
#' @param x a data source string, filename, database connection string, Thredds or other URL
#'
#' @return list of character vectors, see Details
#' @export
#'
#' @examples
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' vapour_sds_names(f)
vapour_sds_names <- function(x) {
  stopifnot(length(x) == 1L)
  sources <- sds_info_cpp(x)
  if (length(sources) > 1) {
    if (length(sources) %% 2 != 0) warning(sprintf("length of subdataset info not a factor of 2 (NAME and DESC expected)"))
    sources0 <- sources
    if (!sum(grepl("NAME=", sources)) == length(sources)/2) warning("sds mismatch")
    sources <- sources[seq(1, length(sources), by = 2L)]
    sources <- unlist(lapply(strsplit(sources, "="), "[", 2))
  }
  list(datasource = rep(x, length(sources)), subdataset = sources)
}


