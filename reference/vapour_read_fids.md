# Read feature names

Obtains the internal 'Feature ID (FID)' for a data source.

## Usage

``` r
vapour_read_fids(
  dsource,
  layer = 0L,
  sql = "",
  limit_n = NULL,
  skip_n = 0,
  extent = NA
)

vapour_read_names(
  dsource,
  layer = 0L,
  sql = "",
  limit_n = NULL,
  skip_n = 0,
  extent = NA
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

- limit_n:

  an arbitrary limit to the number of features scanned

- skip_n:

  an arbitrary number of features to skip

- extent:

  apply an arbitrary extent, only when 'sql' used (must be 'ex = c(xmin,
  xmax, ymin, ymax)' but sp bbox, sf bbox, and raster extent also
  accepted)

## Value

character vector of geometry id 'names'

## Details

This may be virtual (created by GDAL for the SQL interface) and may be
0- or 1- based. Some drivers have actual names, and they are persistent
and arbitrary. Please use with caution, this function can return the
current FIDs, but there's no guarantee of what it represents for
subsequent access.

An earlier version used 'OGRSQL' to obtain these names, which was slow
for some drivers and also clashed with independent use of the `sql`
argument.

`vapour_read_names()` is an older name, aliased to `vapour_read_fids()`.

## Examples

``` r
file <- "list_locality_postcode_meander_valley.tab"
mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
range(fids <- vapour_read_names(mvfile))
#> [1]  1 58
length(fids)
#> [1] 58
```
