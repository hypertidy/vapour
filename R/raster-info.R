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
raster_info <- function(x) {
  raster_info_cpp(pszFilename = x)
}
