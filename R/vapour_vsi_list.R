#' Read GDAL virtual source contents
#'
#' Obtain the names of available items in a virtual file source.
#'
#' The `dsource` must begin with a valid form of the special `vsiPREFIX`, for details
#' see [GDAL Virtual File Systems](https://gdal.org/user/virtual_file_systems.html).
#'
#'
#' Note that the listing is not recursive, and so cannot be used for automation. One would
#' use this function interactively to determine a  useable `/vsiPREFIX/dsource` data
#' source string.
#'
#' @param dsource data source name (path to file, connection string, URL) with virtual prefix, see Details
#' @param ... ignored
#' @return character vector listing of items
#'
#' @examples
#' @export
vapour_vsi_list <- function(dsource, ...) {
  VSI_list(dsource)
}
