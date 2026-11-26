validate_limit_ia <- function(x) {
  if (is.null(x)) stop("invalid ia, is NULL must be between 0 and one less the number of features")
  if (anyNA(x)) stop("missing values ia")
  if (!is.numeric(x)) stop("ia must be numeric, feature positions counted from 0")
  if (length(x) < 1) stop("no valid ia value")
  if (!all(x >= 0)) stop("ia values < 0")

  x
}
validate_limit_ij <- function(x) {
  if (is.null(x)) stop("invalid ij, is NULL")
  if (!is.numeric(x)) stop("ij must be numeric, two feature positions counted from 0")
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
  if (!is.numeric(x)) stop("fa must be numeric, feature id (FID) values")
  if (anyNA(x) | any(!is.finite(x))) stop("missing values fa")
  x
}

## The C++ readers by index walk the layer once in order, so they are given the
## distinct indexes in ascending order. 'put_back' gives the position of each
## requested index in that set, to return the result in the order asked for
## with any repeats.
.ia_plan <- function(ia) {
  ia <- as.numeric(ia)
  u <- sort(unique(ia))
  list(read = u, put_back = match(ia, u))
}
## 'x' is a vector or list read for plan$read, or a list of such columns
.ia_put_back <- function(x, plan, columns = FALSE) {
  if (identical(plan$put_back, seq_along(plan$read))) return(x)
  if (columns) lapply(x, function(column) column[plan$put_back]) else x[plan$put_back]
}

#' @name vapour_read_geometry
#' @export
vapour_read_geometry_ia <- function(dsource, layer = 0L, sql = "", extent = NA, ia = NULL, open_options = NULL) {
  dsource <- .check_dsn_single(dsource)
  open_options <- .check_open_options(open_options)
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer, open_options)
  ia <- validate_limit_ia(ia)
  extent <- validate_extent(extent, sql)
  plan <- .ia_plan(ia)
  out <- gdal_dsn_read_geom_ia( dsn = dsource, layer = as.integer(layer), sql = sql, ex = as.numeric(extent), format = "wkb",  ia = plan$read,
                         open_options = open_options)
  .ia_put_back(out, plan)
}
#' @name vapour_read_geometry
#' @export
vapour_read_geometry_ij <- function(dsource, layer = 0L, sql = "", extent = NA, ij = NULL, open_options = NULL) {
  dsource <- .check_dsn_single(dsource)
  open_options <- .check_open_options(open_options)
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer, open_options)
  ij <- validate_limit_ij(ij)
  extent <- validate_extent(extent, sql)
  gdal_dsn_read_geom_ij( dsn = dsource, layer = as.integer(layer), sql = sql, ex = as.numeric(extent), format = "wkb",  ij = as.numeric(ij),
                         open_options = open_options)
}
#' @name vapour_read_geometry
#' @export
vapour_read_geometry_fa <- function(dsource, layer = 0L, sql = "", extent = NA, fa = NULL, open_options = NULL) {
  dsource <- .check_dsn_single(dsource)
  open_options <- .check_open_options(open_options)
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer, open_options)
  fa <- validate_limit_fa(fa)
  extent <- validate_extent(extent, sql)
  gdal_dsn_read_geom_fa( dsn = dsource, layer = as.integer(layer), sql = sql, ex = as.numeric(extent), format = "wkb",  fa = as.numeric(fa),
                         open_options = open_options)
}

#' @rdname vapour_read_fields
#' @export
vapour_read_fields_ia <- function(dsource, layer = 0L, sql = "", extent = NA, ia = NULL, open_options = NULL) {
  dsource <- .check_dsn_single(dsource)
  open_options <- .check_open_options(open_options)
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer, open_options)
  ia <- validate_limit_ia(ia)
  extent <- validate_extent(extent, sql)
  plan <- .ia_plan(ia)
  out <- gdal_dsn_read_fields_ia(dsn = dsource, layer = as.integer(layer), sql = sql, ex = as.numeric(extent),
                                 fid_column_name = character(0), ia = plan$read, open_options = open_options)
  .ia_put_back(out, plan, columns = TRUE)
}
#' @rdname vapour_read_fields
#' @export
vapour_read_fields_ij <- function(dsource, layer = 0L, sql = "", extent = NA, ij = NULL, open_options = NULL) {
  dsource <- .check_dsn_single(dsource)
  open_options <- .check_open_options(open_options)
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer, open_options)
  ij <- validate_limit_ij(ij)
  extent <- validate_extent(extent, sql)
  gdal_dsn_read_fields_ij(dsn = dsource, layer = as.integer(layer), sql = sql, ex = as.numeric(extent),
                          fid_column_name = character(0), ij = as.numeric(ij), open_options = open_options)
}
#' @rdname vapour_read_fields
#' @export
vapour_read_fields_fa <- function(dsource, layer = 0L, sql = "", extent = NA, fa = NULL, open_options = NULL) {
  dsource <- .check_dsn_single(dsource)
  open_options <- .check_open_options(open_options)
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer, open_options)
  fa <- validate_limit_fa(fa)
  extent <- validate_extent(extent, sql)
  gdal_dsn_read_fields_fa(dsn = dsource, layer = as.integer(layer), sql = sql, ex = as.numeric(extent),
                          fid_column_name = character(0), fa = as.numeric(fa), open_options = open_options)
}

#' @rdname vapour_read_fids
#' @export
vapour_read_fids_ia <- function(dsource, layer = 0L, sql = "", extent = NA, ia = NULL, open_options = NULL) {
  dsource <- .check_dsn_single(dsource)
  open_options <- .check_open_options(open_options)
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer, open_options)
  ia <- validate_limit_ia(ia)
  extent <- validate_extent(extent, sql)
  plan <- .ia_plan(ia)
  out <- gdal_dsn_read_fids_ia(dsn = dsource, layer = as.integer(layer), sql = sql, ex = as.numeric(extent),
                               ia = plan$read, open_options = open_options)
  .ia_put_back(out, plan)
}
#' @rdname vapour_read_fids
#' @export
vapour_read_fids_ij <- function(dsource, layer = 0L, sql = "", extent = NA, ij = NULL, open_options = NULL) {
  dsource <- .check_dsn_single(dsource)
  open_options <- .check_open_options(open_options)
  if (!is.numeric(layer)) layer <- index_layer(dsource, layer, open_options)
  ij <- validate_limit_ij(ij)
  extent <- validate_extent(extent, sql)
  gdal_dsn_read_fids_ij(dsn = dsource, layer = as.integer(layer), sql = sql, ex = as.numeric(extent),
                        ij = as.numeric(ij), open_options = open_options)
}
