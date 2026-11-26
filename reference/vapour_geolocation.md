# Retrieve geolocation information for a dataset

Value is a named vector in a list.

## Usage

``` r
vapour_geolocation(x, sds = NULL, open_options = NULL)
```

## Arguments

- x:

  data source string (i.e. file name or URL or database connection
  string)

- sds:

  a subdataset number, if necessary

- open_options:

  open options for the driver, a character vector of `"NAME=VALUE"`
  strings (as `-oo` in the GDAL command line tools), `NULL` for none.
  The options a driver accepts are listed on its page at
  <https://gdal.org/en/stable/drivers/raster/index.html>

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
