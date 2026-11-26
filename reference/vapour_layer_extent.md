# Read layer extent

Extent of all features in entire layer, possibly after execution of sql
query and input extent filter.

## Usage

``` r
vapour_layer_extent(
  dsource,
  layer = 0L,
  sql = "",
  extent = 0,
  ...,
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

  optional extent (xmin,xmax,ymin,ymax)

- ...:

  unused

- open_options:

  open options for the driver, a character vector of `"NAME=VALUE"`
  strings (as `-oo` in the GDAL command line tools), `NULL` for none.
  The options a driver accepts are listed on its page at
  <https://gdal.org/en/stable/drivers/vector/index.html>

## Value

vector of numeric values xmin,xmax,ymin,ymax

## See also

vapour_read_extent vapour_layer_info

## Examples

``` r
file <- "list_locality_postcode_meander_valley.tab"
## A MapInfo TAB file with polygons
mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
vapour_layer_extent(mvfile)
#> [1]  412718.7  511687.5 5352612.8 5425154.3
```
