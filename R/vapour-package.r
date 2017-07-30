#' vapour.
#'
#' @name vapour
#' @docType package
#' @useDynLib vapour
#' @importFrom Rcpp sourceCpp
NULL

#' SST contours
#'
#' Southern Ocean GHRSST contours from 2017-07-28, read from
#'
#' podaac-ftp.jpl.nasa.gov/allData/ghrsst/data/GDS2/L4
#' GLOB/JPL/MUR/v4.1/2017/209/
#' 20170728090000-JPL-L4_GHRSST-SSTfnd-MUR-GLOB-v02.0-fv04.1.nc
#'
#' See data-raw/sst_c.R for the derivation column \code{sst_c} in Celsius.
#'
#' Also stored in GeoPackage format in
#' \code{system.file("extdata/sst_c.gpkg", package = "vapour")}
#' @docType data
#' @name sst_c
#' @examples
#' ## library(sf)
#' ## plot(sst_c)
#' f <- system.file("extdata/sst_c.gpkg", package = "vapour")#
#' #d <- read_gdal_table(f) %>% mutate(json =  to_format(f, format = "json"))
NULL
