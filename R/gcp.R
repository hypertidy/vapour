
#' Read GCP from gdalinfo output
#'
#' @param x gdalinfo text output
#'
#' @return
#' @noRd
#'
#' @examples
#' x <- "data-raw/gdalinfo_gcp.txt"
.read_gcps <- function(x) {
  x <- readLines(x)
  GCPindex <-  "GCP[  %i]: Id=, Info="
  GCP <- "          (4.9,0.5) -> (-128.078125,51.7890625,0)"

  index <- grep("^GCP\\[", x) + 1
  tx <- x[index]
  cpts <- do.call(rbind, lapply(strsplit(gsub("\\) -> ", "", unlist(lapply(strsplit(tx, "\\("), "[", 2))), ","), as.numeric))
  xyz <- do.call(rbind, lapply(strsplit(gsub("\\)", "", unlist(lapply(strsplit(tx, "\\("), "[", 3))), ","), as.numeric))
  colnames(cpts) <- c("X", "Y")
  colnames(xyz) <- c("gx", "gy", "gz")
  cbind(cpts, xyz)
}

.find_antcoast_gcps <- function(x) {
  x <- x[x[,2] < -61 & x[,2] > -86, ]

}

to_bbox <- function(x) {
  c(xmin(x), ymin(x), xmax(x), ymax(x))
}

## gdalwarp NSS.GHRR.ND.D94182.S1520.E1650.B1625960.GC -te -3710100 -5697852 3710100 1000000 -tr 2000 2000 -t_srs "+proj=stere +lat_0=-73.5 +lon_0=-0.003906 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +no_defs +ellps=WGS84 +towgs84=0,0,0" out.tif
