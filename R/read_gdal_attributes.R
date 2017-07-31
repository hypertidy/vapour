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
#' #vapour(sfile)
#' read_gdal_table(sfile)
read_gdal_table <- function(dsource, layer = 0, sql = NULL) {
  assertthat::is.scalar(layer)
  assertthat::is.number(layer)
  if (is.null(sql)) {
    tibble::as_tibble(vapour(dsource, layer))
  } else {
    tibble::as_tibble(sql_vapour(dsource, layer, sql))
  }
}


# read_gdal_geom_table <- function(dsource, layer = 0, sql = NULL) {
#   assertthat::is.scalar(layer)
#   assertthat::is.number(layer)
#   if (!is.null(sql)) {
#     sql <- gsub("SELECT ", "SELECT FID, ", sql)
#   }
#   tab <- read_gdal_table(dsource, layer = layer, sql = sql)
#   tab[["wkb"]] <- read_gdal_geometry_filter(dsource, layer, filter = tab$FID)
#   tab
# }
