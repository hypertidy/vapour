
<!-- README.md is generated from README.Rmd. Please edit that file -->

# vapour <img src="man/figures/logo.png" align="right" height="228" />

<!-- badges: start -->
[![R\_build\_status](https://github.com/hypertidy/vapour/workflows/R-CMD-check/badge.svg)](https://github.com/hypertidy/vapour/actions)
[![Travis-CI Build
Status](http://badges.herokuapp.com/travis/hypertidy/vapour?branch=master&env=BUILD_NAME=trusty_release&label=linux)](https://travis-ci.org/hypertidy/vapour)
[![OSX Build
Status](http://badges.herokuapp.com/travis/hypertidy/vapour?branch=master&env=BUILD_NAME=osx_release&label=osx)](https://travis-ci.org/hypertidy/vapour)
[![AppVeyor build
status](https://ci.appveyor.com/api/projects/status/github/hypertidy/vapour?branch=master&svg=true)](https://ci.appveyor.com/project/mdsumner/vapour)[![Coverage\_Status](https://img.shields.io/codecov/c/github/hypertidy/vapour/master.svg)](https://codecov.io/github/hypertidy/vapour?branch=master)
[![CRAN\_Status\_Badge](http://www.r-pkg.org/badges/version/vapour)](https://cran.r-project.org/package=vapour)
[![CRAN\_Download\_Badge](http://cranlogs.r-pkg.org/badges/vapour)](https://cran.r-project.org/package=vapour)
<!-- badges: end -->

## Overview

The vapour package provides access to the basic *read* functions
available in [GDAL](https://gdal.org/) for both
[raster](https://gdal.org/user/raster_data_model.html) and
[vector](https://gdal.org/user/vector_data_model.html) data sources.

The functions are deliberately *lower-level* than these data models and
provide access to the component entities independently.

For vector data:

-   read access to feature attributes.
-   read access to raw binary geometry.
-   read access to geometry in text forms (GeoJSON, WKT, GML, KML).
-   read access to the extent, or bounding box, of feature geometries.
-   helper functions to summarize feature identity and geometry status

All vector/feature read tasks can optionally apply an arbitrary limit to
the maximum number of features read or queried, and all allow execution
of [OGRSQL](https://gdal.org/user/ogr_sql_dialect.html) to a layer prior
to data extraction. In combination with a SQL query a [bounding box
spatial filter](https://gdal.org/user/ogr_sql_dialect.html#executesql)
can be applied via the `extent` argument.

For raster data:

-   read access to the list of available rasters within a collection
    source (subdatasets).
-   read access to *structural metadata* for individual raster sources.
-   read access for raw data using GDAL’s [RasterIO
    framework](https://gdal.org/tutorials/raster_api_tut.html) and its
    dynamic image decimation / replication resampling algorithms.
-   read access for raw data using GDAL’s [Warper
    framework](https://gdal.org/api/gdalwarp_cpp.html) and its dynamic
    image warping, a superset of the RasterIO capabilities.

The `vapour_warp_raster()` function is a new feature in vapour, and
reflects many learnings about how the warper works and what is needed
for use. We simplify the approach taken in `vapour_read_raster()` by
allowing specifying an *extent* and *dimensions* as a minimum, and this
works for data sources that contain *overviews* (or pyramid
levels-of-detail) as it automatically chooses an appropriate level for
the request made. This works for files, urls, database connections, and
all the various ways of specifying GDAL data sources.

The workflows available are intended to support development of
applications in R for these vector and [raster
data](https://en.wikipedia.org/wiki/Raster_data) without being
constrained to any particular data model.

## Installation

The package can be installed from CRAN.

``` r
## install.packages("remotes")
remotes::install_cran("vapour")
```

The development version can be installed from Github, easiest is via the
hypertidy universe:

``` r
options(repos = c(
    hypertidy = 'https://hypertidy.r-universe.dev',
    CRAN = 'https://cloud.r-project.org'))
install.packages("vapour")
```

To install the development version the more github-traditional way:

``` r
remotes::install_github("hypertidy/vapour")
```

You will need development tools for building R packages.

On Linux and MacOS building also requires an available GDAL
installation, but on Windows the ROpenSci rwinlib tools are used and the
required GDAL will be downloaded and used when building the package. The
hypertidy universe way also has access to binaries. On windows this
installation is self-contained and only affects the use of R, it can be
used alongside other applications using GDAL.

For MacOS the package build is controlled by an internal CRAN process
including configure arguments for the gdal and proj data directories.

## Purpose

The goal of vapour is to provide a basic **GDAL API** package for R. The
key functions provide vector geometry or attributes and raster data and
raster metadata.

The priority is to give low-level access to key functionality rather
than comprehensive coverage of the library. The real advantage of
`vapour` is the flexibility of a modular workflow, not the outright
efficiency.

A parallel goal is to be freed from the powerful but sometimes limiting
high-level data models of GDAL itself, specifically these are *simple
features* and *affine-based regular rasters composed of 2D slices*.
(GDAL will possibly remove these limitations over time but still there
will always be value in having modularity in an ecosystem of tools.)

GDAL’s dynamic resampling of arbitrary raster windows is also very
useful for interactive tools on local data, and seems under-utilized in
favour of less accessible online image services.

This partly draws on work done in [the sf
package](https://github.com/r-spatial/sf) and in packages `rgdal` and
`rgdal2`. I’m amazed that something as powerful and general as GDAL is
still only available through these lenses, but recent improvements make
things much easier to use and share. Specifically `Rcpp` means that
access to external libs is simplified, easier to learn and easier to get
started and make progress. The other part is that cross-platform support
is now much better, with more consistency on the libraries available on
the CRAN machines and in other contexts.

## Warnings

It’s possible to give problematic “SELECT” statements via the `sql`
argument. Note that the geometry readers `vapour_read_geometry`,
`vapour_read_geometry_text`, and `vapour_read_extent` will strip out the
`SELECT ... FROM` clause and replace it with `SELECT * FROM` to ensure
that the geometry is accessible, though the attributes are ignored. This
means we can allow the user or `dplyr` to create any `SELECT` statement.
The function `vapour_read_geometry` will return a list of NULLs, in this
case.

I haven’t tried this against a real database, I’m not sure if we need
`AsBinary()` around EWKB geoms, for example - but at any rate these can
be ingested by `sf`.

## Examples

The package documentation page gives an overview of available functions.

``` r
help("vapour-package")
```

See the vignettes and documentation for examples WIP.

## Context

Examples of packages that use vapour are in development,
[RGDALSQL](https://github.com/mdsumner/RGDALSQL) and
[lazyraster](https://github.com/hypertidy/lazyraster). `RGDALSQL` aims
to leverage the facilities of GDAL to provide data *on-demand* for many
sources *as if* they were databases. `lazyraster` uses the
level-of-detail facility of GDAL to read just enough resolution from a
raster source using traditional window techniques.

Limitations, work-in-progress and other discussion are active here:
<https://github.com/hypertidy/vapour/issues/4>

We’ve kept a record of a minimal GDAL wrapper package here:

<https://github.com/diminutive/gdalmin>

Before those I had worked on getting sp and dplyr to at least work
together <https://github.com/dis-organization/sp_dplyrexpt> and recently
rgdal was updated to allow tibbles to be used, something that spbabel
and spdplyr really needed to avoid friction.

Early exploration of allow non-geometry read with rgdal was tried here:
<https://github.com/hypertidy/gladr>

Thanks to Edzer Pebesma and Roger Bivand and Tim Keitt for prior art
that I crib and copy from. Jeroen Ooms helped the R community hugely by
providing an accessible build process for libraries on Windows. Mark
Padgham helped kick me over a huge obstacle in using C++ libraries with
R. Simon Wotherspoon and Ben Raymond have endured ravings about wanting
this level of control for many years.

# Code of conduct

Please note that this project is released with a [Contributor Code of
Conduct](https://github.com/hypertidy/vapour/blob/master/CONDUCT.md). By
participating in this project you agree to abide by its terms.
