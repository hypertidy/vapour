#' Read attributes from source as a data frame
#'
#' See here for SQL details http://www.gdal.org/ogr_sql.html
#' @param dsource data source string
#' @param layer layer index (0)
#' @param sql optional sql query executed against the data source
#'
#' @return
#' @export
#'
#' @examples
#' f <- system.file("extdata", "point.shp", package = "vapour")
#' read_gdal_table(f, sql = "SELECT * FROM point WHERE FID > 4 AND FID < 8")
#' sfile <- system.file("shape/nc.shp", package="sf")
#' read_gdal_table(sfile, sql = "SELECT NAME, AREA, SID79 * 10 FROM nc WHERE PERIMETER < 1.2")
#' read_gdal_table(sfile)
read_gdal_table <- function(dsource, layer = 0, sql = NULL) {
  assertthat::is.scalar(layer)
  assertthat::is.number(layer)
  assertthat::is.string(dsource)
  if (is.null(sql)) sql <- ""
  assertthat::is.string(sql)
  tibble::as_tibble(vapour_read_attributes(dsource, layer, sql))
}



