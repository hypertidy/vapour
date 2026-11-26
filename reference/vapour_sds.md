# Subdatasets of raster sources

A data frame of the subdatasets of one or more raster sources, one row
for each subdataset. This is the vectorised form of
[`vapour_sds_names()`](https://hypertidy.github.io/vapour/reference/vapour_sds_names.md),
and it also gives GDAL's description of each subdataset.

## Usage

``` r
vapour_sds(x, open_options = NULL)
```

## Arguments

- x:

  one or more data source strings, filename, database connection string,
  or other URL

- open_options:

  open options for the driver, a character vector of `"NAME=VALUE"`
  strings (as `-oo` in the GDAL command line tools), `NULL` for none.
  They apply to every source in `x`

## Value

data frame with columns

- dsn:

  the source, as given in `x`

- sds:

  the number of the subdataset within its source, counted from 1 (as
  used by the `sds` argument of other functions)

- subdataset:

  the data source string of the subdataset

- description:

  GDAL's description of the subdataset, usually its dimensions, name and
  type

## Details

A source with no subdatasets gives one row, in which `subdataset` is the
source itself and `description` is `NA`. So every source has at least
one row, and every value of `subdataset` is a data source that can be
opened.

## See also

[`vapour_sds_names()`](https://hypertidy.github.io/vapour/reference/vapour_sds_names.md)
[`vapour_raster_info()`](https://hypertidy.github.io/vapour/reference/vapour_raster_info.md)

## Examples

``` r
tif <- system.file("extdata", "sst.tif", package = "vapour")
vapour_sds(tif)
#>                                                      dsn sds
#> 1 /home/runner/work/_temp/Library/vapour/extdata/sst.tif   1
#>                                               subdataset description
#> 1 /home/runner/work/_temp/Library/vapour/extdata/sst.tif        <NA>

f <- system.file("extdata/gdal", "sds.nc", package = "vapour")
## protect from error with netcdf problems
result <- try(vapour_sds(c(f, tif)), silent = TRUE)
if (!inherits(result, "try-error")) {
  print(result)
}
#>                                                          dsn sds
#> 1 /home/runner/work/_temp/Library/vapour/extdata/gdal/sds.nc   1
#> 2 /home/runner/work/_temp/Library/vapour/extdata/gdal/sds.nc   2
#> 3     /home/runner/work/_temp/Library/vapour/extdata/sst.tif   1
#>                                                                subdataset
#> 1  NETCDF:"/home/runner/work/_temp/Library/vapour/extdata/gdal/sds.nc":vv
#> 2 NETCDF:"/home/runner/work/_temp/Library/vapour/extdata/gdal/sds.nc":vv2
#> 3                  /home/runner/work/_temp/Library/vapour/extdata/sst.tif
#>                           description
#> 1  [61x87] vv (64-bit floating-point)
#> 2 [61x87] vv2 (64-bit floating-point)
#> 3                                <NA>
```
