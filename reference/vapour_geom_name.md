# Read geometry column name

There might be one or more geometry column names, or it might be an
empty string.

## Usage

``` r
vapour_geom_name(dsource, layer = 0L, sql = "", open_options = NULL)
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

- open_options:

  open options for the driver, a character vector of `"NAME=VALUE"`
  strings (as `-oo` in the GDAL command line tools), `NULL` for none.
  The options a driver accepts are listed on its page at
  <https://gdal.org/en/stable/drivers/vector/index.html>

## Value

character vector of geometry column name/s

## Details

It might be "", or "geom", or "*ogr_geometry*" - the last is a default
name given when SQL is executed by GDAL but there was no geometry name,
and 'SELECT \* ' or equivalent was used.

This feature is required by the DBI backend work in RGDALSQL, so that
when `SELECT * ` is used we can give a reasonable name to the geometry
column which is obtained separately.

## Examples

``` r
file <- system.file("extdata/tab/list_locality_postcode_meander_valley.tab", package = "vapour")
vapour_geom_name(file)  ## empty string
#> [1] ""
```
