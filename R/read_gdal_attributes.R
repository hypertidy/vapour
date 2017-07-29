#' Read attributes from source as a data frame
#'
#' @param dsource data source string
#' @param layer layer index (0)
#'
#' @return
#' @export
#'
#' @examples
#' f <- system.file("extdata", "point.shp", package = "vapour")
#' read_gdal_table(f)
#' sfile <- system.file("shape/nc.shp", package="sf")
#' #vapour(sfile)
#' read_gdal_table(sfile)
read_gdal_table <- function(dsource, layer = 0) {
  assertthat::is.scalar(layer)
  assertthat::is.number(layer)
  tibble::as_tibble(vapour(dsource, layer))
}
