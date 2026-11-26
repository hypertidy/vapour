# Read GDAL layer info

Read GDAL layer information for a vector data source.

## Usage

``` r
vapour_layer_info(
  dsource,
  layer = 0L,
  sql = "",
  extent = NA,
  count = TRUE,
  ...
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

- count:

  logical to control if count calculated and returned, TRUE by default
  (set to FALSE to avoid the extra calculation and missing value is the
  result)

- ...:

  unused, reserved for future use

## Value

list with a list of character vectors of projection metadata, see
details

## Details

Set `extent` and/or `count` to `FALSE` to avoid calculating them if not
needed, it might take some time.

The layer information elements are

- dsn:

  the data source name

- driver:

  the short name of the driver used

- layer:

  the name of the layer queried

- layer_names:

  the name/s of all available layers (see
  [vapour_layer_names](https://hypertidy.github.io/vapour/reference/vapour_layer_names.md))

- fields:

  a named vector of field types (see
  [vapour_report_fields](https://hypertidy.github.io/vapour/reference/vapour_report_fields.md))

- count:

  the number of features in this data source (can be turned off to avoid
  the extra work `count`)

- extent:

  the extent of all features xmin, xmax, ymin, ymax (can be turned off
  to avoid the extra work `extent`)

- projection:

  a list of formats of the projection metadata, see next

- filelist:

  the files that make up the data source (may be none)

`$projection` is a list with elements `Wkt`, `Proj4` and `EPSG`. Use
`$projection$Wkt` as most authoritative, but we don't enter into the
discussion or limit what might be done with this (that's up to you).
`EPSG` is the integer EPSG code when the projection carries one or GDAL
can identify one for it, otherwise `NA`.

To get the geometry type/s of a layer see
[`vapour_read_type()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md).

## See also

vapour_geom_name vapour_layer_names vapour_report_fields
vapour_read_fields vapour_driver vapour_read_names

## Examples

``` r
file <- "list_locality_postcode_meander_valley.tab"
## A MapInfo TAB file with polygons
mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
info <- vapour_layer_info(mvfile)
names(info$projection)
#> [1] "Wkt"   "Proj4" "EPSG" 

## info depends on the query/spatial-filter
vapour_layer_info(mvfile, extent = c(412000,  420000, 5352612.8, 5425154.3), 
  sql = "SELECT * FROM list_locality_postcode_meander_valley")$count
#> [1] 3
```
