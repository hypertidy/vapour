#' Simple raster warping in-memory
#'
#' Warp a GDAL data source to in in-memory raster target
#'
#' @param gdalsource a file name or other source string
#' @param target raster target object
#' @noRd
#' @examples
#' laea_srs <- 'PROJCS["unnamed",GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4326"]],PROJECTION["Lambert_Azimuthal_Equal_Area"],PARAMETER["latitude_of_center",-42],PARAMETER["longitude_of_center",142],PARAMETER["false_easting",0],PARAMETER["false_northing",0]]'
#' extdim_to_geotransform <- function(ext, dim) {
#'   c(xmin = raster::xmin(ext),
#'     xres = (raster::xmax(ext) - raster::xmin(ext))/ dim[2],
#'     xsh = 0,
#'     ymax = raster::ymax(ext),
#'     ysh = 0,
#'     yres = -(raster::ymax(ext) - raster::ymin(ext))/ dim[1])
#' }
#'
#' ex <- c(-5e6, 5e6, -5e6, 5e6)
#'
#' library(raster)
#' ras <- raster(extent(ex), crs = "+proj=laea +lon_0=142 +lat_0=-42 +datum=WGS84", nrows= 100, ncols = 100)
#' #r <- projectRaster(raster("inst/extdata/sst.tif"), ras)
#' vals <- vapour:::warp_memory_cpp("inst/extdata/sst.tif", "",
#'                          target_WKT = laea_srs,
#'                          extdim_to_geotransform(extent(ex), dim(ras)[1:2]), dim(ras)[1:2])
#' vals[vals < -3.0000e+38 ] <- NA
#' plot(setValues(ras, vals))
lazywarp <- function(gdalsource, target) {
  ## the killer here is conversion of R's proj strings to WKT ...
 return("not implemented, see example")
}

