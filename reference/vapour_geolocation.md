# Retrieve geolocation information for a dataset

Value is a named vector in a list.

## Usage

``` r
vapour_geolocation(x, sds = NULL)
```

## Arguments

- x:

  data source string (i.e. file name or URL or database connection
  string)

- sds:

  a subdataset number, if necessary

## Value

list with a single character vector

## Details

If no geolocation exist the return value is an empty list.

## Examples

``` r
drivers <- vapour_all_drivers()
ok <- drivers$raster[ drivers$driver == "netCDF"]
if (isTRUE(ok) && interactive()) {
 vapour_geolocation(system.file("extdata/gdal/geos_rad.nc", package = "vapour"), 0L)
}
```
