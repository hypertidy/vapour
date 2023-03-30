validate_limit_ia <- function(x) {
  if (is.null(x)) stop("invalid ia, is NULL must be between 0 and one less the number of features")
  if (anyNA(x)) stop("missing values ia")
  if (!all(x >= 0)) stop("ia values < 0")

  x
}
validate_limit_ij <- function(x) {
  if (is.null(x)) stop("invalid ij, is NULL")
  if (length(x) < 2) stop("ij values not of length 2")
  if (length(x) > 2) {
    message("ij values longer than 2, only first two used")
    x <- x[1:2]
  }
  if (anyNA(x)) stop("missing values ij")
  if (!all(x >= 0)) stop("ij values < 0")
  if (x[2] < x[1]) stop("ij values must be increasing")
  if (as.integer(x[1]) == as.integer(x[2])) stop("ij values must not be duplicated (use vapour_read_geometry_ia() to read a single geometry)")
  x
}

validate_limit_fa <- function(x) {
  if (is.null(x)) stop("invalid fa, is NULL")
  if (length(x) < 1) stop("no valid fa value")
  if (anyNA(x) | any(!is.finite(x))) stop("missing values fa")
  x
}

#' @name vapour_read_geometry
#' @export
vapour_read_geometry_ia <- function(dsource, layer = 0L, sql = "", extent = NA, ia = NULL) {
  dsource <- .check_dsn_single(dsource)
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  ia <- validate_limit_ia(ia)
  extent <- validate_extent(extent, sql)
  gdal_dsn_read_geom_ia( dsn = dsource, layer = layer, sql = sql, ex = extent, format = "wkb",  ia = ia)
}
#' @name vapour_read_geometry
#' @export
vapour_read_geometry_ij <- function(dsource, layer = 0L, sql = "", extent = NA, ij = NULL) {
  dsource <- .check_dsn_single(dsource)
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer)
  ij <- validate_limit_ij(ij)
  extent <- validate_extent(extent, sql)
  gdal_dsn_read_geom_ij( dsn = dsource, layer = layer, sql = sql, ex = extent, format = "wkb",  ij = ij)
}


