#' Raster information
#'
#' Return the basic structural metadata of a raster source understood by GDAL.
#'
#' These are
#' \describe{
#' \item{geotransform}{the affine transform}
#' \item{dimXY}{dimensions x-y, columns*rows}
#' \item{minmax}{range of data values}
#' \item{tilesXY}{dimensions x-y of internal tiling scheme}
#' \item{projection}{text version of map projection parameter string}
#' }
#' The affine transform is the six parameter conversion of row-col position to map coordinates.
#' Usually the shear components are zero, and so these correspond to the offset and scale used to
#' position the raster - in combination with the number of rows and columns of data they
#' provide the spatial extent and the pixel size in each direction.
#' @param x data source string (i.e. file name or URL or database connection string)
#' @export
#' @examples
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' raster_info(f)
raster_info <- function(x, ..., sds = NULL) {
  datasourcename <- sds_boilerplate_checks(x, sds = sds)
  raster_info_cpp(pszFilename = datasourcename)
}

sds_boilerplate_checks <- function(x, sds = NULL) {
  ## don't pass relative paths to GDAL
  ## but also don't prevent access to non-files
  if (file.exists(x)) x <- base::normalizePath(x, mustWork = FALSE)
  ## use sds wrapper to target the first by default
  datavars <- as.data.frame(sds_info(x), stringsAsFactors = FALSE)
  wasnull <- is.null(sds)
  if (wasnull) sds <- 1
  if (wasnull && nrow(datavars) > 1L) {
    varnames <- unlist(lapply(strsplit(datavars$subdataset, ":"), tail, 1L))
    message("subdataset (variable) used is %s (1)", varnames[1])
    message("If that is not correct, set it to one of ", paste(sprintf("%i (%s)", seq_len(nrow(datavars))[-1], varnames[-1]), collapse = ", "))
  }
  stopifnot(length(sds) == 1L)

  if (sds < 1) stop("sds must be 1 at minimum")
  if (sds > nrow(datavars)) stop(sprintf("'sds' must not exceed number of subdatasets (%i)", nrow(datavars)))
  datavars$subdataset[sds]
}
#' Information on GDAL raster variables
#'
#' A 'subdataset' is a collection abstraction for a number of variables within a single GDAL source. If there's only one
#' the datasource and its variable have the same data source string. If there's more than one the subdatasets have the form
#' 'DRIVER:"datasourcename":varname'. Each subdataset name can stand in place of a data source name that has only one variable,
#' so we always treat a source as a subdataset, even if there's only one.
#'
#' Returns a list of `datasource` and `subdataset`. In the case of a single
#' @param x a data source string, filename, database connection string, Thredds or other URL
#'
#' @return list of character vectors, see Details
#' @export
#'
#' @examples
#' f <- system.file("extdata", "sst.tif", package = "vapour")
#' sds_info(f)
sds_info <- function(x) {
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
