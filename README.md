<!-- README.md is generated from README.Rmd. Please edit that file -->
[![Travis-CI Build Status](https://travis-ci.org/hypertidy/vapour.svg?branch=master)](https://travis-ci.org/hypertidy/vapour) [![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/hypertidy/vapour?branch=master&svg=true)](https://ci.appveyor.com/project/hypertidy/vapour) [![Coverage Status](https://img.shields.io/codecov/c/github/hypertidy/vapour/master.svg)](https://codecov.io/github/hypertidy/vapour?branch=master) [![CRAN\_Status\_Badge](http://www.r-pkg.org/badges/version/vapour)](https://cran.r-project.org/package=vapour)

vapour
======

The goal of vapour is to provide a basic **GDAL API** package for R. Ideally, this could become a common foundation for other packages to specialize. A parallel goal is to be freed from the powerful but sometimes limiting high-level data models of GDAL itself, specifically these are *simple features* and *affine-based regular rasters composed of 2D slices*. (GDAL will possibly remove these limitations over time but still there will always be value in having modularity in an ecosystem of tools. )

This is inspired by and draws heavily on work done [the sf package](https://github.com/r-spatial/sf) and rgdal and rgdal2.

Purpose
=======

Current we have control to do the following:

-   read attributes only
-   read geometry only
-   read geometry as raw binary, or various text forms
-   read geometry bounding box only
-   (optionally) apply OGRSQL to a layer prior to any of the above <http://www.gdal.org/ogr_sql.html>

Limitations, work-in-progress and other discussion are active here: <https://github.com/hypertidy/vapour/issues/4>

Examples
--------

There's a function `vapour_read_attributes` that returns the attributes as list of vectors.

``` r
pfile <- system.file("extdata", "point.shp", package = "vapour")
library(vapour)
vapour_read_attributes(pfile)
#> $a
#>  [1]  1  2  3  4  5  6  7  8  9 10
```

A higher level function *somewhere else* could wrap that function to return a data frame, but we don't want that in `vapour` because it's not aligned with the goals of being lightweight and reducing the level of interpretation applied. The `data.frame` function in R is actually a very primitive implememtation for data frames, so we avoid putting that interpretation on the data and leave that up to the developer / user.

``` r
mvfile <- system.file("extdata/tab/list_locality_postcode_meander_valley.tab", package="vapour")

dat <- as.data.frame(vapour_read_attributes(mvfile),  stringsAsFactors = FALSE)

dim(dat)
#> [1] 58 11

head(dat)
#>   LOCAL_ID        NAME POSTCODE PLAN_REF   GAZ_DATE NOM_REG_NO
#> 1   100422    Caveside     7304  CPR5322 1970-01-01       947L
#> 2   100366     Weegena     7304  CPR5327 1970-01-01      1300M
#> 3   100337   Kimberley     7304  CPR5361 1970-01-01      1063T
#> 4   100308     Parkham     7304  CPR5327 1970-01-01      1179Y
#> 5   100263   Frankford     7275  CPR5142 1970-01-01      1003Q
#> 6   100311 Bridgenorth     7277  CPR5140 1970-01-01       925X
#>                                      UFI          CREATED_ON
#> 1 {4a5db4da-ca19-41a0-8dd4-c28a14bbee18} 2016-03-04 10:42:37
#> 2 {253b676e-2791-469c-ac5e-9cb3a95cc158} 2015-06-19 13:46:50
#> 3 {75f60a99-4c58-4d3e-911d-bbaa9a04164c} 2016-09-16 10:54:56
#> 4 {b008d456-4e80-4237-80f6-a26c03817e3c} 2014-06-06 16:50:22
#> 5 {953f4006-6397-4d03-af97-507eab170862} 2014-12-08 09:07:12
#> 6 {5cf8e2a3-631c-4d52-a79c-0ce475f76848} 2015-05-11 10:49:09
#>                                LIST_GUID SHAPE_AREA SHAPE_LEN
#> 1 {839edd46-01a7-4a45-9d97-499962fa952b}      -9999  39785.88
#> 2 {de35ebd4-0ac0-4299-947d-87d07c69426a}      -9999  31587.54
#> 3 {73ced9ad-ee9a-41d5-a5bc-c95c4ab948d9}      -9999  35693.32
#> 4 {37f17d1f-d2a0-4b78-ba0d-e5f62f216658}      -9999  67614.51
#> 5 {47f3a313-913f-4f83-8dc1-021907f9ee80}      -9999  70140.78
#> 6 {425d01cc-223b-4348-b965-384a98fd7999}      -9999  38156.70
```

OGRSQL
------

Note that each lower-level function accepts a `sql` argument, which sends a query to the GDAL library to be executed against the data source, this can create custom layers and so is independent of and ignores the `layer` argument.

``` r
vapour_read_attributes(mvfile, sql = "SELECT NAME, PLAN_REF FROM list_locality_postcode_meander_valley WHERE POSTCODE = 7310")
#> $NAME
#> character(0)
#> 
#> $PLAN_REF
#> character(0)


vapour_read_attributes(mvfile, sql = "SELECT NAME, PLAN_REF, FID FROM list_locality_postcode_meander_valley WHERE POSTCODE = 7306")
#> $NAME
#> [1] "Cradle Mountain" "Mount Roland"    "Middlesex"       "Lower Beulah"   
#> 
#> $PLAN_REF
#> [1] "CPR5363" "CPR5359" "CPR5362" "CPR5361"
#> 
#> $FID
#> [1] 20 31 36 45
```

Also note that FID is a special row number value, to be used a as general facility for selecting by structural row.

See <http://www.gdal.org/ogr_sql.html>

There are many useful higher level operations that can be used with this. The simplest is the ability to use GDAL as a database-like connection to attribute tables.

A low-level function will return a character vector of JSON, GML, KML or WKT.

``` r
vapour_read_geometry(pfile)[5:6]  ## format = "WKB"
#> [[1]]
#>  [1] 01 01 00 00 00 00 00 60 08 18 ad ec 3f 00 00 e0 9a ec 77 e2 3f
#> 
#> [[2]]
#>  [1] 01 01 00 00 00 00 00 c0 40 3c bb d0 3f 00 00 80 0e 30 25 d5 3f

vapour_read_geometry_text(pfile)[5:6]  ## format = "json"
#> [1] "{ \"type\": \"Point\", \"coordinates\": [ 0.89612962375395, 0.577139189234003 ] }" 
#> [2] "{ \"type\": \"Point\", \"coordinates\": [ 0.261427939636633, 0.330394758377224 ] }"

cfile <- system.file("extdata/sst_c.gpkg", package = "vapour")
vapour_read_geometry_text(pfile, format = "gml")[2]
#> [1] "<gml:Point><gml:coordinates>0.145755324047059,0.395469118840992</gml:coordinates></gml:Point>"

## don't do this with a non-longlat data set like cfile
vapour_read_geometry_text(pfile, format = "kml")[1:2]
#> [1] "<Point><coordinates>0.623376188334078,0.380098037654534</coordinates></Point>"
#> [2] "<Point><coordinates>0.145755324047059,0.395469118840992</coordinates></Point>"

str(vapour_read_geometry_text(cfile, format = "wkt")[1:2])
#>  chr [1:2] "MULTILINESTRING ((-16254.4210476553 -3269904.98849485,-48956.5880244328 -3282652.40200143,-82133.8545994558 -33"| __truncated__ ...
```

We can combine these together to get a custom data set.

``` r
library(dplyr)
dat <- as.data.frame(vapour_read_attributes(cfile),  stringsAsFactors = FALSE) %>% dplyr::mutate(wkt = vapour_read_geometry_text(cfile, format = "wkt"))
glimpse(dat)
#> Observations: 7
#> Variables: 3
#> $ level <chr> "275", "280", "285", "290", "295", "300", "305"
#> $ sst   <dbl> 1.85, 6.85, 11.85, 16.85, 21.85, 26.85, 31.85
#> $ wkt   <chr> "MULTILINESTRING ((-16254.4210476553 -3269904.98849485,-...
```

Fast summary
------------

There is a basic function `vapour_read_extent` to return a straightforward bounding box vector for every feature, so that we can flexibly build an index of a data set for later use.

``` r
mvfile <- system.file("extdata/tab/list_locality_postcode_meander_valley.tab", package="vapour")
str(vapour_read_extent(mvfile))
#> List of 58
#>  $ : num [1:4] 448353 457706 5386606 5397352
#>  $ : num [1:4] 453544 459318 5403972 5412505
#>  $ : num [1:4] 454840 461042 5411562 5417892
#>  $ : num [1:4] 461505 476213 5410911 5424854
#>  $ : num [1:4] 471573 483157 5417110 5424645
#>  $ : num [1:4] 491638 494048 5417262 5419331
#>  $ : num [1:4] 508512 510057 5408709 5409648
#>  $ : num [1:4] 435993 444517 5392900 5402213
#>  $ : num [1:4] 453298 462671 5383592 5393869
#>  $ : num [1:4] 457592 473357 5377059 5397051
#>  $ : num [1:4] 465137 477418 5394645 5408531
#>  $ : num [1:4] 495773 507915 5396427 5405789
#>  $ : num [1:4] 489939 499712 5410677 5418328
#>  $ : num [1:4] 459056 471905 5418849 5425154
#>  $ : num [1:4] 455980 463979 5392870 5399259
#>  $ : num [1:4] 475889 493826 5379077 5389494
#>  $ : num [1:4] 480223 490958 5397052 5414286
#>  $ : num [1:4] 485858 494029 5407194 5418267
#>  $ : num [1:4] 505330 508582 5407832 5411813
#>  $ : num [1:4] 412719 433694 5364102 5392659
#>  $ : num [1:4] 418130 431437 5352613 5368181
#>  $ : num [1:4] 452420 460772 5395775 5404447
#>  $ : num [1:4] 459779 470225 5398391 5405474
#>  $ : num [1:4] 471219 485530 5383849 5395087
#>  $ : num [1:4] 457719 469430 5407328 5420616
#>  $ : num [1:4] 486794 497211 5391896 5400794
#>  $ : num [1:4] 488190 498092 5396973 5409378
#>  $ : num [1:4] 479268 487581 5412326 5419668
#>  $ : num [1:4] 487082 491200 5405846 5410168
#>  $ : num [1:4] 507192 508682 5409572 5411550
#>  $ : num [1:4] 432998 441413 5402891 5406595
#>  $ : num [1:4] 457813 463400 5408117 5415279
#>  $ : num [1:4] 470867 483434 5404320 5419938
#>  $ : num [1:4] 481800 490524 5388711 5398922
#>  $ : num [1:4] 493483 506413 5403349 5412391
#>  $ : num [1:4] 416790 424281 5390142 5396738
#>  $ : num [1:4] 432186 439291 5396541 5404521
#>  $ : num [1:4] 462417 469412 5391797 5398604
#>  $ : num [1:4] 473323 481800 5392390 5399469
#>  $ : num [1:4] 466298 472556 5407297 5412735
#>  $ : num [1:4] 488006 497239 5385526 5393899
#>  $ : num [1:4] 500476 505427 5409888 5413864
#>  $ : num [1:4] 422718 443693 5364752 5398969
#>  $ : num [1:4] 436393 455345 5390197 5405569
#>  $ : num [1:4] 448464 450390 5404212 5404797
#>  $ : num [1:4] 459377 465847 5397255 5404093
#>  $ : num [1:4] 475502 482058 5400269 5408109
#>  $ : num [1:4] 476793 484115 5394878 5400905
#>  $ : num [1:4] 495936 499733 5389589 5396934
#>  $ : num [1:4] 425270 449448 5353987 5383046
#>  $ : num [1:4] 439927 462874 5358223 5393865
#>  $ : num [1:4] 468039 477380 5378281 5388344
#>  $ : num [1:4] 457541 466788 5403285 5409104
#>  $ : num [1:4] 495086 500372 5393786 5399069
#>  $ : num [1:4] 503880 507861 5396801 5404336
#>  $ : num [1:4] 502595 507481 5402752 5406382
#>  $ : num [1:4] 506075 509426 5404305 5408099
#>  $ : num [1:4] 506637 511687 5405460 5409603
```

This makes for a very lightweight summary data set that will scale to hundreds of large inputs.

``` r
dat <- as.data.frame(vapour_read_attributes(mvfile), 
                     stringsAsFactors = FALSE)
library(raster)
#> Loading required package: sp
#> 
#> Attaching package: 'raster'
#> The following object is masked from 'package:dplyr':
#> 
#>     select
dat$bbox <- vapour_read_extent(mvfile)

plot(purrr::reduce(lapply(dat$bbox, raster::extent), raster::union))
purrr::walk(lapply(dat$bbox, raster::extent), plot, add = TRUE)
```

![](README-unnamed-chunk-8-1.png)

An example is this set of *some number of* property boundary shapefiles, read into a few hundred Mb of simple features.

``` r
library(dplyr)
files <- raadfiles::thelist_files(format = "") %>% filter(grepl("parcel", fullname), grepl("shp$", fullname)) %>% 
  slice(1:8)
#> Warning in raadfiles::thelist_files(format = ""): datadir and file root
#> don't match?
library(vapour)
system.time(purrr::map(files$fullname, sf::read_sf))
#>    user  system elapsed 
#>   9.272   0.437   9.801
library(blob)

## our timing is competitive, and we get to choose what is read
## and when
read_table <- function(file) as.data.frame(vapour_read_attributes(file),  stringsAsFactors = FALSE)
system.time({
d <- purrr::map(files$fullname, read_table)
d <- dplyr::bind_rows(d)
g <- purrr::map(files$fullname, vapour_read_geometry)
d[["wkb"]] <- new_blob(unlist(g, recursive = FALSE))
})
#>    user  system elapsed 
#>   3.132   0.280   3.444
```

We can read that in this simpler way for a quick data set to act as an index.

``` r
system.time({
  d <- purrr::map_df(files$fullname, read_table)
  d$bbox <- unlist(purrr::map(files$fullname, vapour_read_extent), recursive = FALSE)
})
#>    user  system elapsed 
#>   2.987   0.336   3.357

pryr::object_size(d)
#> 46.7 MB
glimpse(d)
#> Observations: 107,854
#> Variables: 20
#> $ CID        <chr> "", "", "", "", "", "", "", "", "", "", "", "", "",...
#> $ VOLUME     <chr> "169864", "", "", "136703", "", "", "212990", "2449...
#> $ FOLIO      <int> 2, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 9, 0, 1, 0, 8, ...
#> $ PID        <chr> "", "", "", "", "", "", "", "", "", "", "", "", "",...
#> $ POT_PID    <chr> "", "", "", "", "", "", "", "", "", "", "", "", "",...
#> $ LPI        <chr> "FEV10", "KKL85", "", "HSY23", "GES42", "", "480244...
#> $ CAD_TYPE1  <chr> "Private Parcel", "Authority Land", "Authority Land...
#> $ CAD_TYPE2  <chr> "Private Parcel", "Forestry Tasmania", "Forestry Ta...
#> $ TENURE_TY  <chr> "Freehold Title", "Crown Land", "Crown Land", "Crow...
#> $ FEAT_NAME  <chr> "", "", "", "", "", "", "", "", "", "", "", "", "",...
#> $ STRATA_LEV <chr> "Not Applicable", "Not Applicable", "Not Applicable...
#> $ COMP_AREA  <dbl> 1200513.096, 23096.094, 148147.128, 4603209.423, 48...
#> $ MEAS_AREA  <dbl> 1207000, 0, 0, 0, 0, 0, 0, 136700, 0, 0, 0, 18260, ...
#> $ UFI        <chr> "cad013844403", "cad013933348", "cad013933302", "ca...
#> $ FMP        <chr> "cad000029000", "cad000029000", "cad000029000", "ca...
#> $ CREATED_ON <chr> "2015-08-25 14:31:57", "2016-04-07 10:22:12", "2016...
#> $ LIST_GUID  <chr> "{d1b80f74-2873-46d1-a6ed-d1d27a45bd6e}", "{3cb147d...
#> $ SHAPE_AREA <dbl> 1200513.096, 23096.094, 148147.128, 4603209.423, 48...
#> $ SHAPE_LEN  <dbl> 4382.7258, 2365.0896, 14870.8324, 16288.4164, 14990...
#> $ bbox       <list> [<551456.8, 552738.1, 5413518.9, 5414799.6>, <5544...
```

Set up
------

I've kept a record of a minimal GDAL wrapper package here:

<https://github.com/mdsumner/gdalmin>

This must be run when your function definitions change:

``` r
tools::package_native_routine_registration_skeleton("../vapour", "src/init.c",character_only = FALSE)
```

Context
-------

My first real attempt at DBI abstraction is here, this is still an aspect that is desperately needed in R to help bring tidyverse attention to spatial:

<https://github.com/mdsumner/RGDALSQL>

Before that I had worked on getting sp and dplyr to at least work together <https://github.com/dis-organization/sp_dplyrexpt> and recently rgdal was updated to allow tibbles to be used, something that spbabel and spdplyr really needed to avoid friction.

Early exploration of allow non-geometry read with rgdal was tried here: <https://github.com/r-gris/gladr>

Big thanks to Edzer Pebesma and Roger Bivand and Tim Keitt for prior art that I crib and copy from. Jeroen Ooms helped the R community hugely by providing an automatable build process for libraries on Windows. Mark Padgham helped kick me over a huge obstacle in using C++ libraries with R. Simon Wotherspoon and Ben Raymond have endured my ravings about wanting this level of control for many years.

Code of conduct
===============

Please note that this project is released with a [Contributor Code of Conduct](CONDUCT.md). By participating in this project you agree to abide by its terms.
