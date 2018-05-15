#' vapour
#'
#' A lightweight GDAL API package for R.
#'
#'
#' Lightweight means we access parts of the GDAL API as near as
#' possible to their native usage. GDAL is not a particularly lightweight
#' library, but it does provide a very nice abstraction over format details.
#'
#' We include functions for raster and vector sources.
#'
#' \tabular{ll}{
#'  \code{\link{raster_info}} \tab structural metadata of a source \cr
#' \code{\link{raster_io}} \tab read data direct from a window of a raster band source \cr
#'  \code{\link{sds_info}} \tab list individual raster sources in a source containing subdatasets  \cr
#'  }
#' \tabular{ll}{
#'  \code{\link{vapour_layer_names}} \tab list names of vector layers \cr
#'  \code{\link{vapour_read_attributes}} \tab read attributes, the columnar data associated with each geometry \cr
#' \code{\link{vapour_read_extent}} \tab read the extent of geometries \cr
#' \code{\link{vapour_read_feature_what}} \tab read feature into specified format \cr
#' \code{\link{vapour_read_geometry}} \tab read feature into binary form \cr
#' \code{\link{vapour_read_geometry_text}} \tab read feature into text form \cr
#'  }
#'
#'  As far as possible vapour aims to minimize the level of interpretation
#'  provided for the functions, so that developers can choose how things are
#'  implemented. This means we return raw lists or vectors rather than data
#'  frames or classed types.
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
#' f <- system.file("extdata/sst_c.gpkg", package = "vapour")
#' #d <- read_gdal_table(f) %>% mutate(json =  to_format(f, format = "json"))
NULL


