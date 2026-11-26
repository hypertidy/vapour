# Feature access with vapour

vapour reads vector *features* at a low level: geometry comes back as
raw binary (WKB) or as text, and attributes are read separately, leaving
you to combine them however you like. Part of that control is choosing
*which* features to read, rather than always pulling an entire layer
into memory.

This vignette shows the four feature-access patterns vapour supports,
using two example data sources that ship with the package.

``` r

library(vapour)

## a MapInfo TAB file of polygons (localities)
mvfile <- system.file("extdata/tab/list_locality_postcode_meander_valley.tab",
                      package = "vapour")

## a shapefile of points
pfile <- system.file("extdata/point.shp", package = "vapour")
```

Every feature has a positional *index* (0-based, in layer order) and a
feature *identifier* (FID) assigned by the driver. These are not always
the same number. The count of features in the layer is:

``` r

n <- length(vapour_read_names(mvfile))
n
#> [1] 58
```

## All features

Read every feature. The geometry readers return one element per feature,
which you pair with the FIDs or attributes as needed.

``` r

g <- vapour_read_geometry(mvfile)   # a list of raw WKB, one per feature
length(g)
#> [1] 58
g[[1]][1:8]                         # first bytes of the first geometry
#> [1] 01 03 00 00 00 01 00 00

## the matching feature identifiers
head(vapour_read_names(mvfile))
#> [1] 1 2 3 4 5 6
```

Extents and geometry types can be read the same way without decoding the
full geometry:

``` r

str(vapour_read_extent(mvfile)[1:2])
#> List of 2
#>  $ : num [1:4] 448353 457706 5386606 5397352
#>  $ : num [1:4] 453544 459318 5403972 5412505
table(vapour_read_type(mvfile))
#> 
#>  3  6 
#> 49  9
```

## An index range, i to j

[`vapour_read_geometry_ij()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
reads a contiguous block of features *by position*, from `i` to `j`
inclusive (both increasing). Use `c(0, n - 1)` for all of them.

``` r

ij <- vapour_read_geometry_ij(mvfile, ij = c(0, 2))   # the first three features
length(ij)
#> [1] 3
```

## Arbitrary indices

[`vapour_read_geometry_ia()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
reads features at *arbitrary* positions. Order is respected and
duplicates are allowed, so you can reorder or repeat features.

``` r

ia <- vapour_read_geometry_ia(mvfile, ia = c(2, 0, 0))
length(ia)
#> [1] 3
```

## By feature identifier (FID)

Positional index and FID are not always aligned, so to select by FID use
a SQL query against the data source. SQL also lets you subset geometry
and attributes together with the same filter.

``` r

sql <- "SELECT * FROM list_locality_postcode_meander_valley WHERE FID IN (2, 5, 9)"
wkt <- vapour_read_geometry_text(mvfile, sql = sql, textformat = "wkt")
length(wkt)
#> [1] 3
substr(wkt[[1]], 1, 50)
#> [1] "MULTIPOLYGON (((458251.36 5410533.49,458192.26 541"
```

## Geometry as text

Any reader can return text instead of WKB – GeoJSON (the default), WKT,
GML or KML – which is convenient for inspection.

``` r

substr(vapour_read_geometry_text(pfile, textformat = "wkt")[[1]], 1, 40)
#> [1] "POINT (0.623376188334078 0.3800980376545"
```

See
[`?vapour_read_geometry`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
for the full set of readers and the arguments they share (`layer`,
`sql`, `extent`, `limit_n`, `skip_n`).
