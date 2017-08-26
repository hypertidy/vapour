#' Read geometry from source as a blob vector.
#'
#' @param dsource data source string
#' @param layer layer index (0)
#'
#' @return
#' @export
#' @importFrom blob new_blob
#' @examples
#' f <- system.file("extdata", "point.shp", package = "vapour")
#' tibble::tibble(wkb = read_gdal_geometry(f))
#' sfile <- system.file("shape/nc.shp", package="sf")
#' #vapour(sfile)
#' read_gdal_table(sfile)
read_gdal_geometry <- function(dsource, layer = 0, sql = NULL) {
  assertthat::is.scalar(layer)
  assertthat::is.number(layer)
  assertthat::is.string(dsource)
  if (is.null(sql)) sql <- ""
  assertthat::is.string(sql)
  blob::new_blob(vapour_read_geometry(dsource, layer, sql) )
}
