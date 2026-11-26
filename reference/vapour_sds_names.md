# GDAL raster subdatasets (variables)

A **subdataset** is a collection abstraction for a number of
**variables** within a single GDAL source. If there's only one variable
the datasource and the variable have the same data source string. If
there is more than one the subdatasets have the form
**DRIVER:"datasourcename":varname**. Each subdataset name can stand in
place of a data source name that has only one variable, so we always
treat a source as a subdataset, even if there's only one.

## Usage

``` r
vapour_sds_names(x, open_options = NULL)
```

## Arguments

- x:

  a data source string, filename, database connection string, or other
  URL

- open_options:

  open options for the driver, a character vector of `"NAME=VALUE"`
  strings (as `-oo` in the GDAL command line tools), `NULL` for none

## Value

character vector of subdataset names, or just the source itself if no
SDS are present

## Details

Returns a character vector of `subdatasets`. In the case of a normal
data source, with no subdatasets the value is simply the `datasource`.

If the raw SDS names contain spaces these are replaced by '%20' escape
strings. A specific example is
"WCS:https://elevation.nationalmap.gov:443" with request
"arcgis/services/3DEPElevation/ImageServer/WCSServer?version=2.0.1&coverage=DEP3Elevation_Hillshade
Gray". This function will return "..DEP3Elevation_Hillshade%20Gray". See
[wiki
post](https://github.com/hypertidy/vapour/wiki/Examples-of-subdatasets)
for more details.

## Examples

``` r
f <- system.file("extdata/gdal", "sds.nc", package = "vapour")
## protect from error with netcdf problems
result <- try(vapour_sds_names(f), silent = TRUE)
if (!inherits(result, "try-error")) {
  print(result)
}
#> [1] "NETCDF:\"/home/runner/work/_temp/Library/vapour/extdata/gdal/sds.nc\":vv" 
#> [2] "NETCDF:\"/home/runner/work/_temp/Library/vapour/extdata/gdal/sds.nc\":vv2"
vapour_sds_names(system.file("extdata", "sst.tif", package = "vapour"))
#> [1] "/home/runner/work/_temp/Library/vapour/extdata/sst.tif"
```
