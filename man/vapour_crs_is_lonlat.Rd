% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/vapour-gdal-library.R
\name{vapour_crs_is_lonlat}
\alias{vapour_crs_is_lonlat}
\title{Is the CRS string representative of angular coordinates}
\usage{
vapour_crs_is_lonlat(crs)
}
\arguments{
\item{crs}{character string of length 1}
}
\value{
logical value \code{TRUE} for lonlat, \code{FALSE} otherwise
}
\description{
Returns \code{TRUE} if this is longitude latitude data. Missing, malformed, zero-length values are disallowed.
}
\examples{
vapour_gdal_version() ## versions to catch problems with string input
vapour_proj_version()
vapour_crs_is_lonlat("+proj=aeqd +lon_0=147 +lat_0=-42")
vapour_crs_is_lonlat("EPSG:4326")
vapour_srs_wkt("+proj=laea")
vapour_crs_is_lonlat("+proj=laea +type=crs")
vapour_crs_is_lonlat("OGC:CRS84")
vapour_crs_is_lonlat("WGS84")
vapour_crs_is_lonlat("NAD27")
vapour_crs_is_lonlat("EPSG:3031")
}
