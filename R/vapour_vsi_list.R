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
#' pointzipfile <- system.file("extdata/vsi/point_shp.zip", package = "vapour")
#' vapour_vsi_list(sprintf("/vsizip/%s", pointzipfile))
#' \donttest{
#' \dontrun{
#' ## example from https://github.com/hypertidy/vapour/issues/55
#' #file <- "http/radmap_v3_2015_filtered_dose/radmap_v3_2015_filtered_dose.ers.zip"
#' #url <- "http://dapds00.nci.org.au/thredds/fileServer/rr2/national_geophysical_compilations"
#' #u <- sprintf("/vsizip//vsicurl/%s", file.path(url, file))
#' #vapour_vsi_list(u)
#' #[1] "radmap_v3_2015_filtered_dose"     "radmap_v3_2015_filtered_dose.ers"
#' #[3] "radmap_v3_2015_filtered_dose.isi" "radmap_v3_2015_filtered_dose.txt"
#' #gdalinfo /vsitar//home/ubuntu/LT05_L1GS_027026_20060116_20160911_01_T2.tar.gz
#' #vapour_vsi_list("/vsitar//home/ubuntu/LT05_L1GS_027026_20060116_20160911_01_T2.tar.gz")
#' #"LT05_L1TP_027026_20061218_20160911_01_T1_ANG.txt"
#' #"LT05_L1TP_027026_20061218_20160911_01_T1_B1.TIF"
#' #"LT05_L1TP_027026_20061218_20160911_01_T1_B2.TIF"
#' #"LT05_L1TP_027026_20061218_20160911_01_T1_B3.TIF"
#' #...
#' }}
#' @export
vapour_vsi_list <- function(dsource, ...) {
  vsi_list_gdal_cpp(dsource)
}
