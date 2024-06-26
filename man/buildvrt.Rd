% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/vapour_vrt.R
\name{buildvrt}
\alias{buildvrt}
\title{Build vrt, special case "-separate"}
\usage{
buildvrt(dsn)
}
\arguments{
\item{dsn}{one or more raster sources}
}
\value{
a character string of the built vrt, multiple sources treated as bands
}
\description{
Build vrt, special case "-separate"
}
\examples{
f <- system.file("extdata/sst.tif", package = "vapour", mustWork = TRUE)
vrt <- buildvrt(c(f, vapour_vrt(f)))
writeLines(vrt)
}
