<!-- README.md is generated from README.Rmd. Please edit that file -->
    ## 
    ## Attaching package: 'dplyr'

    ## The following objects are masked from 'package:stats':
    ## 
    ##     filter, lag

    ## The following objects are masked from 'package:base':
    ## 
    ##     intersect, setdiff, setequal, union

vapour
======

The goal of vapour is to learn C++ enough to help create a **GDAL API** package for R so that R package developers have a common foundation to extend. A common foundation is required so that general tools can be developed from a general resource, and so specific goals and choices made for other projects can be maintained separately. A parallel goal is to be freed from the powerful but sometimes limiting high-level data models of GDAL itself, specifically these are *simple features* and *affine-based regular rasters composed of 2D slices*. GDAL will possibly remove these limitations over time but still there will always be value in having modularity in an ecosystem of tools.

Currently all it does is read the geometry as text and / or read the attribute table from a vector source (only integer, double and character types are supported.) This is inspired by and draws heavily on [the sf package, simple features for R](https://github.com/r-spatial/sf).

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

A low-level function will return a character vector of JSON, GML, or KML.

``` r
to_format(pfile)[5:6]  ## format = "json"
#> [1] "{ \"type\": \"Point\", \"coordinates\": [ 0.89612962375395, 0.577139189234003 ] }" 
#> [2] "{ \"type\": \"Point\", \"coordinates\": [ 0.261427939636633, 0.330394758377224 ] }"

sfile <- system.file("shape/nc.shp", package="sf")

to_format(sfile, format = "gml")[99:100]
#> [1] "<gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-77.9607315063477,34.1892433166504 -77.9658660888672,34.2422866821289 -77.9752807617188,34.2433624267578 -77.9831466674805,34.2616806030273 -78.0002212524414,34.2678833007812 -77.9953918457031,34.2827987670898 -78.0070190429688,34.2848167419434 -78.0113067626953,34.312614440918 -78.0259246826172,34.3287696838379 -77.9866790771484,34.339916229248 -77.9944534301758,34.3623161315918 -77.9790725708008,34.3756866455078 -77.9498138427734,34.3660850524902 -77.9439392089844,34.3564376831055 -77.9217834472656,34.3733139038086 -77.888069152832,34.364070892334 -77.8283843994141,34.3879699707031 -77.8091430664062,34.359432220459 -77.7505264282227,34.305046081543 -77.864387512207,34.1927375793457 -77.894401550293,34.0691795349121 -77.9267578125,34.0620346069336 -77.9607315063477,34.1892433166504</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon>"                                                                                                                                             
#> [2] "<gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-78.6557159423828,33.948673248291 -78.6347198486328,33.9779777526855 -78.6302719116211,34.0102005004883 -78.5877838134766,34.0306053161621 -78.5634307861328,34.0589447021484 -78.5442810058594,34.134162902832 -78.5272369384766,34.154857635498 -78.4927444458008,34.158504486084 -78.4254302978516,34.1380653381348 -78.3611221313477,34.1867218017578 -78.3735733032227,34.2023506164551 -78.2610626220703,34.2152633666992 -78.15478515625,34.3622436523438 -78.130241394043,34.3641242980957 -78.0259246826172,34.3287696838379 -78.0113067626953,34.312614440918 -78.0070190429688,34.2848167419434 -77.9953918457031,34.2827987670898 -78.0002212524414,34.2678833007812 -77.9831466674805,34.2616806030273 -77.9752807617188,34.2433624267578 -77.9658660888672,34.2422866821289 -77.9607315063477,34.1892433166504 -77.9585266113281,33.9925804138184 -78.0348052978516,33.9142913818359 -78.579719543457,33.8819923400879 -78.6557159423828,33.948673248291</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon>"

to_format(sfile, format = "kml")[1:2]
#> [1] "<Polygon><outerBoundaryIs><LinearRing><coordinates>-81.4727554321289,36.2343559265137 -81.5408401489258,36.2725067138672 -81.5619812011719,36.2735939025879 -81.6330642700195,36.3406867980957 -81.7410736083984,36.3917846679688 -81.6982803344727,36.4717788696289 -81.7027969360352,36.5193405151367 -81.6699981689453,36.5896492004395 -81.3452987670898,36.5728645324707 -81.347541809082,36.537914276123 -81.3247756958008,36.5136795043945 -81.3133239746094,36.4806976318359 -81.2662353515625,36.4372062683105 -81.2628402709961,36.4050407409668 -81.2406921386719,36.3794174194336 -81.2398910522461,36.365364074707 -81.2642440795898,36.3524131774902 -81.3289947509766,36.3635025024414 -81.3613739013672,36.3531608581543 -81.3656921386719,36.3390502929688 -81.354133605957,36.2997169494629 -81.3674545288086,36.2786979675293 -81.4063873291016,36.2850532531738 -81.4123306274414,36.2672920227051 -81.431037902832,36.2607192993164 -81.4528884887695,36.2395858764648 -81.4727554321289,36.2343559265137</coordinates></LinearRing></outerBoundaryIs></Polygon>"
#> [2] "<Polygon><outerBoundaryIs><LinearRing><coordinates>-81.2398910522461,36.365364074707 -81.2406921386719,36.3794174194336 -81.2628402709961,36.4050407409668 -81.2662353515625,36.4372062683105 -81.3133239746094,36.4806976318359 -81.3247756958008,36.5136795043945 -81.347541809082,36.537914276123 -81.3452987670898,36.5728645324707 -80.9034423828125,36.5652122497559 -80.9335479736328,36.4983139038086 -80.9657745361328,36.4672203063965 -80.9496688842773,36.4147338867188 -80.9563903808594,36.4037971496582 -80.9779510498047,36.3913764953613 -80.9828414916992,36.3718338012695 -81.0027770996094,36.3666801452637 -81.0246429443359,36.3778343200684 -81.0428009033203,36.4103355407715 -81.0842514038086,36.4299201965332 -81.0985641479492,36.43115234375 -81.1133117675781,36.4228515625 -81.1293792724609,36.4263305664062 -81.1383972167969,36.4176254272461 -81.1533660888672,36.4247398376465 -81.1766738891602,36.4154434204102 -81.2398910522461,36.365364074707</coordinates></LinearRing></outerBoundaryIs></Polygon>"
```

We can combine these together to get a custom data set.

``` r
library(dplyr)
dat <- read_gdal_table(sfile) %>% dplyr::mutate(kml = to_format(sfile, format = "kml"))
glimpse(dat)
#> Observations: 100
#> Variables: 15
#> $ AREA      <dbl> 0.114, 0.061, 0.143, 0.070, 0.153, 0.097, 0.062, 0.0...
#> $ PERIMETER <dbl> 1.442, 1.231, 1.630, 2.968, 2.206, 1.670, 1.547, 1.2...
#> $ CNTY_     <dbl> 1825, 1827, 1828, 1831, 1832, 1833, 1834, 1835, 1836...
#> $ CNTY_ID   <dbl> 1825, 1827, 1828, 1831, 1832, 1833, 1834, 1835, 1836...
#> $ NAME      <chr> "Ashe", "Alleghany", "Surry", "Currituck", "Northamp...
#> $ FIPS      <chr> "37009", "37005", "37171", "37053", "37131", "37091"...
#> $ FIPSNO    <dbl> 37009, 37005, 37171, 37053, 37131, 37091, 37029, 370...
#> $ CRESS_ID  <int> 5, 3, 86, 27, 66, 46, 15, 37, 93, 85, 17, 79, 39, 73...
#> $ BIR74     <dbl> 1091, 487, 3188, 508, 1421, 1452, 286, 420, 968, 161...
#> $ SID74     <dbl> 1, 0, 5, 1, 9, 7, 0, 0, 4, 1, 2, 16, 4, 4, 4, 18, 3,...
#> $ NWBIR74   <dbl> 10, 10, 208, 123, 1066, 954, 115, 254, 748, 160, 550...
#> $ BIR79     <dbl> 1364, 542, 3616, 830, 1606, 1838, 350, 594, 1190, 20...
#> $ SID79     <dbl> 0, 3, 6, 2, 3, 5, 2, 2, 2, 5, 2, 5, 4, 4, 6, 17, 4, ...
#> $ NWBIR79   <dbl> 19, 12, 260, 145, 1197, 1237, 139, 371, 844, 176, 59...
#> $ kml       <chr> "<Polygon><outerBoundaryIs><LinearRing><coordinates>...
```

Set up
------

I've kept a record of a minimal GDAL wrapper package here:

<https://github.com/mdsumner/gdalmin>

This must be run when your function definitions change:

``` r
tools::package_native_routine_registration_skeleton("../vapour", "src/init.c",character_only = FALSE)
```

Code of conduct
===============

Please note that this project is released with a [Contributor Code of Conduct](CONDUCT.md). By participating in this project you agree to abide by its terms.
