# Read GDAL feature geometry

Read GDAL geometry as binary blob, text, or numeric extent.

## Usage

``` r
vapour_read_geometry_ia(
  dsource,
  layer = 0L,
  sql = "",
  extent = NA,
  ia = NULL,
  open_options = NULL
)

vapour_read_geometry_ij(
  dsource,
  layer = 0L,
  sql = "",
  extent = NA,
  ij = NULL,
  open_options = NULL
)

vapour_read_geometry(
  dsource,
  layer = 0L,
  sql = "",
  limit_n = NULL,
  skip_n = 0,
  extent = NA,
  open_options = NULL
)

vapour_read_geometry_text(
  dsource,
  layer = 0L,
  sql = "",
  textformat = "json",
  limit_n = NULL,
  skip_n = 0L,
  extent = NA_real_,
  open_options = NULL
)

vapour_read_extent(
  dsource,
  layer = 0L,
  sql = "",
  limit_n = NULL,
  skip_n = 0,
  extent = NA,
  open_options = NULL
)

vapour_feature_extent(
  dsource,
  layer = 0L,
  sql = "",
  limit_n = NULL,
  skip_n = 0,
  extent = NA,
  open_options = NULL
)

vapour_read_type(
  dsource,
  layer = 0L,
  sql = "",
  limit_n = NULL,
  skip_n = 0,
  extent = NA,
  open_options = NULL
)
```

## Arguments

- dsource:

  data source name (path to file, connection string, URL)

- layer:

  integer of layer to work with, defaults to the first (0) or the name
  of the layer

- sql:

  if not empty this is executed against the data source (layer will be
  ignored)

- extent:

  apply an arbitrary extent, only when 'sql' used (must be 'ex = c(xmin,
  xmax, ymin, ymax)' but sp bbox, sf bbox, and raster extent also
  accepted)

- ia:

  an arbitrary index, integer vector with values between 0 and one less
  the number of features, duplicates allowed and arbitrary order is ok

- open_options:

  open options for the driver, a character vector of `"NAME=VALUE"`
  strings (as `-oo` in the GDAL command line tools), `NULL` for none.
  The options a driver accepts are listed on its page at
  <https://gdal.org/en/stable/drivers/vector/index.html>

- ij:

  a range index, integer vector of length two with values between 0 and
  one less the number of features, this range of geometries is returned

- limit_n:

  an arbitrary limit to the number of features scanned

- skip_n:

  an arbitrary number of features to skip

- textformat:

  indicate text output format, available are "json" (default), "gml",
  "kml", "wkt"

## Value

for `vapour_read_geometry()`, `vapour_read_geometry_ia()` and
`vapour_read_geometry_ij()` a raw vector of geometry, for
`vapour_read_extent()` a list of numeric vectors each with
'xmin,xmax,ymin,ymax' respectively for each geometry, for
`vapour_feature_extent()` a numeric matrix with columns 'xmin', 'xmax',
'ymin', 'ymax' and one row for each geometry, for `vapour_read_type()` a
character vector. See Details for more information.

## Details

`vapour_read_geometry` will read features as binary WKB,
`vapour_read_geometry_text` as various text formats (geo-json, wkt, kml,
gml),

`vapour_read_extent` a numeric extent which is the native bounding box,
the four numbers (in this order) `xmin, xmax, ymin, ymax`.
`vapour_feature_extent` gives the same extents as a matrix, one row for
each feature, with those four as named columns. A missing geometry is a
row of `NA`. For each function an optional SQL string will be evaluated
against the data source before reading.

`vapour_read_geometry_ia` will read features by *arbitrary index*, so
any integer between 0 and one less than the number of features. These
may be duplicated. If 'ia' is greater than the highest index NULL is
returned, but if less than 0 the function will error.

`vapour_read_geometry_ij` will read features by *index range*, so two
numbers to read every feature between those limits inclusively. 'i' and
'j' must be increasing.

`vapour_read_type` will read the (wkb) type of the geometry as an
integer. These are `0` unknown, `1` Point, `2` LineString, `3` Polygon,
`4` MultiPoint, `5` MultiLineString, `6` MultiPolygon, `7`
GeometryCollection, and the other more exotic types listed in
"api/vector_c_api.html" from the GDAL home page (as at October 2020). A
missing value 'NA' indicates an empty geometry.

Note that `limit_n` and `skip_n` interact with the affect of `sql`,
first the query is executed on the data source, then while looping
through available features `skip_n` features are ignored, and then a
feature-count begins and the loop is stopped if `limit_n` is reached.

Note that `extent` applies to the 'SpatialFilter' of 'ExecuteSQL':
https://gdal.org/user/ogr_sql_dialect.html#executesql.

## Examples

``` r
file <- "list_locality_postcode_meander_valley.tab"
## A MapInfo TAB file with polygons
mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
## A shapefile with points
pfile <- system.file("extdata/point.shp", package = "vapour")

## raw binary WKB points in a list
ptgeom <- vapour_read_geometry(pfile)
## create a filter query to ensure data read is small
SQL <- "SELECT FID FROM list_locality_postcode_meander_valley WHERE FID < 3"
## polygons in raw binary (WKB)
plgeom <- vapour_read_geometry_text(mvfile, sql = SQL)
## polygons in raw text (GeoJSON)
txtjson <- vapour_read_geometry_text(mvfile, sql = SQL)

## polygon extents in a list xmin, xmax, ymin, ymax
exgeom <- vapour_read_extent(mvfile)

## the same extents as a matrix, one row for each polygon
exmat <- vapour_feature_extent(mvfile)
head(exmat)
#>          xmin     xmax    ymin    ymax
#> [1,] 448353.3 457705.8 5386606 5397352
#> [2,] 453543.6 459318.0 5403972 5412505
#> [3,] 454839.8 461042.0 5411562 5417892
#> [4,] 461504.9 476213.0 5410911 5424854
#> [5,] 471572.5 483157.2 5417110 5424645
#> [6,] 491638.3 494048.2 5417262 5419331

## points in raw text (GeoJSON)
txtpointjson <- vapour_read_geometry_text(pfile)
## points in raw text (WKT)
txtpointwkt <- vapour_read_geometry_text(pfile, textformat = "wkt")
```
