% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/raster-input.R
\name{vapour_warp_raster_raw}
\alias{vapour_warp_raster_raw}
\alias{vapour_warp_raster_int}
\alias{vapour_warp_raster_dbl}
\alias{vapour_warp_raster_chr}
\alias{vapour_warp_raster_hex}
\title{type safe(r) raster warp}
\usage{
vapour_warp_raster_raw(
  x,
  bands = NULL,
  extent = NULL,
  dimension = NULL,
  projection = "",
  set_na = TRUE,
  source_projection = NULL,
  source_extent = 0,
  resample = "near",
  silent = TRUE,
  ...,
  warp_options = "",
  transformation_options = "",
  open_options = "",
  options = ""
)

vapour_warp_raster_int(
  x,
  bands = NULL,
  extent = NULL,
  dimension = NULL,
  projection = "",
  set_na = TRUE,
  source_projection = NULL,
  source_extent = 0,
  resample = "near",
  silent = TRUE,
  ...,
  warp_options = "",
  transformation_options = "",
  open_options = "",
  options = ""
)

vapour_warp_raster_dbl(
  x,
  bands = NULL,
  extent = NULL,
  dimension = NULL,
  projection = "",
  set_na = TRUE,
  source_projection = NULL,
  source_extent = 0,
  resample = "near",
  silent = TRUE,
  ...,
  warp_options = "",
  transformation_options = "",
  open_options = "",
  options = ""
)

vapour_warp_raster_chr(
  x,
  bands = NULL,
  extent = NULL,
  dimension = NULL,
  projection = "",
  set_na = TRUE,
  source_projection = NULL,
  source_extent = 0,
  resample = "near",
  silent = TRUE,
  ...,
  warp_options = "",
  transformation_options = "",
  open_options = "",
  options = ""
)

vapour_warp_raster_hex(
  x,
  bands = NULL,
  extent = NULL,
  dimension = NULL,
  projection = "",
  set_na = TRUE,
  source_projection = NULL,
  source_extent = 0,
  resample = "near",
  silent = TRUE,
  ...,
  warp_options = "",
  transformation_options = "",
  open_options = "",
  options = ""
)
}
\arguments{
\item{x}{vector of data source names (file name or URL or database connection string)}

\item{bands}{index of band/s to read (1-based), may be new order or replicated, or NULL (all bands used, the default)}

\item{extent}{extent of the target warped raster 'c(xmin, xmax, ymin, ymax)'}

\item{dimension}{dimensions in pixels of the warped raster (x, y)}

\item{projection}{projection of warped raster (in Well-Known-Text, or any projection string accepted by GDAL)}

\item{set_na}{NOT IMPLEMENTED logical, should 'NODATA' values be set to \code{NA}}

\item{source_projection}{optional, override or augment the projection of the source (in Well-Known-Text, or any projection string accepted by GDAL)}

\item{source_extent}{extent of the source raster, used to override/augment incorrect source metadata}

\item{resample}{resampling method used (see details in \link{vapour_read_raster})}

\item{silent}{\code{TRUE} by default, set to \code{FALSE} to report messages}

\item{...}{unused}

\item{warp_options}{character vector of options, as in gdalwarp -wo - see Details}

\item{transformation_options}{character vector of options, as in gdalwarp -to see Details}

\item{open_options}{character vector of options, as in gdalwarp -oo - see Details}

\item{options}{character vectors of options as per the gdalwarp command line}
}
\value{
atomic vector of the nominated type raw, int, dbl, or character (hex)
}
\description{
These wrappers around \code{\link[=vapour_warp_raster]{vapour_warp_raster()}} guarantee single vector output of the nominated type.
}
\details{
_hex and _chr are aliases of each other.
}
\examples{
b <- 4e5
f <- system.file("extdata", "sst.tif", package = "vapour")
prj <- "+proj=aeqd +lon_0=147 +lat_0=-42"
bytes <- vapour_warp_raster_raw(f, extent = c(-b, b, -b, b),
                             dimension = c(18, 2),
                             bands = 1, 
                             projection = prj)
# not useful given source type floating point, but works
str(bytes)
}
