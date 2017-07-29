<!-- README.md is generated from README.Rmd. Please edit that file -->
vapour
======

The goal of vapour is to learn C++ enough to help create a **GDAL API** package for R so that R package developers have a common foundation to extend. A common foundation is required so that general tools can be developed, without having to adhere to specific goals and choices made for other projects, or to be limited to the high level data models of GDAL itself (simple features and affine-based regular rasters composed of 2D slices).

Currently all it does is read the attribute table from a vector source (only integer, double and character types are supported.) This is inspired by and draws heavily on [the sf package, simple features for R](https://github.com/r-spatial/sf).

Big thanks to Edzer Pebesma and Roger Bivand for prior art that I crib and copy from.

Examples
--------

There's a low level function `vapour` that returns the attributes as list of vectors.

``` r
pfile <- system.file("extdata", "point.shp", package = "vapour")
library(vapour)
vapour(pfile)
#> $a
#>  [1]  1  2  3  4  5  6  7  8  9 10
```

A higher level function `read_gdal_attribute` wraps that function to return a data frame.

``` r
sfile <- system.file("shape/nc.shp", package="sf")

read_gdal_table(sfile)
#> # A tibble: 100 x 14
#>     AREA PERIMETER CNTY_ CNTY_ID        NAME  FIPS FIPSNO CRESS_ID BIR74
#>    <dbl>     <dbl> <dbl>   <dbl>       <chr> <chr>  <dbl>    <int> <dbl>
#>  1 0.114     1.442  1825    1825        Ashe 37009  37009        5  1091
#>  2 0.061     1.231  1827    1827   Alleghany 37005  37005        3   487
#>  3 0.143     1.630  1828    1828       Surry 37171  37171       86  3188
#>  4 0.070     2.968  1831    1831   Currituck 37053  37053       27   508
#>  5 0.153     2.206  1832    1832 Northampton 37131  37131       66  1421
#>  6 0.097     1.670  1833    1833    Hertford 37091  37091       46  1452
#>  7 0.062     1.547  1834    1834      Camden 37029  37029       15   286
#>  8 0.091     1.284  1835    1835       Gates 37073  37073       37   420
#>  9 0.118     1.421  1836    1836      Warren 37185  37185       93   968
#> 10 0.124     1.428  1837    1837      Stokes 37169  37169       85  1612
#> # ... with 90 more rows, and 5 more variables: SID74 <dbl>, NWBIR74 <dbl>,
#> #   BIR79 <dbl>, SID79 <dbl>, NWBIR79 <dbl>
```

There are many useful higher level operations that can be used with this. The simplest is the ability to use GDAL as a database-like connection to attribute tables.

Set up
------

These are just rough notes for myself at the moment.

-   hack the C++
-   autconf
-   run the register native routines
-   build the package

Register routines:

<https://ironholds.org/registering-routines/>

<http://dirk.eddelbuettel.com/blog/2017/04/30/#006_easiest_package_registration>

``` r
tools::package_native_routine_registration_skeleton("../vapour", "src/init.c",character_only = FALSE)
```

Code of conduct
===============

Please note that this project is released with a [Contributor Code of Conduct](CONDUCT.md). By participating in this project you agree to abide by its terms.
