# Read GDAL layer names

Obtain the names of available layers from a GDAL vector source.

## Usage

``` r
vapour_layer_names(dsource, ..., open_options = NULL)
```

## Arguments

- dsource:

  data source name (path to file, connection string, URL)

- ...:

  arguments ignored for deprecated compatibility (no 'sql' argument any
  longer)

- open_options:

  open options for the driver, a character vector of `"NAME=VALUE"`
  strings (as `-oo` in the GDAL command line tools), `NULL` for none.
  The options a driver accepts are listed on its page at
  <https://gdal.org/en/stable/drivers/vector/index.html>

## Value

character vector of layer names

## Details

Some vector sources have multiple layers while many have only one.
Shapefiles for example have only one, and the single layer gets the file
name with no path and no extension. GDAL provides a quirk for shapefiles
in that a directory may act as a data source, and any shapefile in that
directory acts like a layer of that data source. This is a little like
the one-or-many sleight that exists for raster data sources with
subdatasets (there's no way to virtualize single rasters into a data
source with multiple subdatasets, oh except by using VRT....)

See
[vapour_sds_names](https://hypertidy.github.io/vapour/reference/vapour_sds_names.md)
for more on the multiple topic.

## Examples

``` r
file <- "list_locality_postcode_meander_valley.tab"
mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
vapour_layer_names(mvfile)
#> [1] "list_locality_postcode_meander_valley"
```
