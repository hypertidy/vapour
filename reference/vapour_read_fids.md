# Read feature names

Obtains the internal 'Feature ID (FID)' for a data source.

## Usage

``` r
vapour_read_fids_ia(
  dsource,
  layer = 0L,
  sql = "",
  extent = NA,
  ia = NULL,
  open_options = NULL
)

vapour_read_fids_ij(
  dsource,
  layer = 0L,
  sql = "",
  extent = NA,
  ij = NULL,
  open_options = NULL
)

vapour_read_fids(
  dsource,
  layer = 0L,
  sql = "",
  limit_n = NULL,
  skip_n = 0,
  extent = NA,
  open_options = NULL
)

vapour_read_names(
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

## Value

numeric vector of feature ids

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

`vapour_read_fids_ia` gives the feature ids at chosen positions (`ia`,
counted from 0, in any order and with repeats) and `vapour_read_fids_ij`
those in an inclusive range of positions (`ij`). A position that is not
in the layer gives `NA`, with a warning. These turn positions into the
values that the `fa` argument of
[`vapour_read_geometry_fa()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
and
[`vapour_read_fields_fa()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md)
takes.

## Examples

``` r
file <- "list_locality_postcode_meander_valley.tab"
mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
range(fids <- vapour_read_names(mvfile))
#> [1]  1 58
length(fids)
#> [1] 58
vapour_read_fids_ia(mvfile, ia = c(0, 5, 2))
#> [1] 1 6 3
vapour_read_fids_ij(mvfile, ij = c(0, 2))
#> [1] 1 2 3
```
