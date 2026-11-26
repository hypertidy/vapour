# GDAL version and drivers.

Return information about the GDAL library in use.

## Usage

``` r
vapour_gdal_version()

vapour_proj_version()

vapour_all_drivers()

vapour_driver(dsource)
```

## Arguments

- dsource:

  data source string (i.e. file name or URL or database connection
  string)

## Value

please see Details, character vectors or lists of character vectors

## Details

`vapour_gdal_version` returns the version of GDAL as a string. This
corresponds to the "–version" as described for "GDALVersionInfo". [GDAL
documentation](https://gdal.org/).

`vapour_all_drivers` returns the names and capabilities of all available
drivers, in a list. This contains:

- `driver` the driver (short) name

- `name` the (long) description name

- `vector` logical vector indicating a vector driver

- `raster` logical vector indicating a raster driver

- `create` driver can create (note vapour provides no write capacity)

- `copy` driver can copy (note vapour provides no write capacity)

- `virtual` driver has virtual capabilities ('vsi')

- `sql_dialects` the SQL dialects a vector driver supports, space
  separated with the default first (`NA` for other drivers, and before
  GDAL 3.6)

`vapour_driver()` returns the short name of the driver, e.g. 'GPKG' or
'GTiff', to get the long name and other properties use
`vapour_all_drivers()` and match on 'driver'.

## Examples

``` r
vapour_gdal_version()
#> [1] "GDAL 3.12.2 \"Chicoutimi\", released 2026/02/03"

drv <- vapour_all_drivers()

f <- system.file("extdata/sst_c.fgb", package = "vapour")
vapour_driver(f)
#> [1] "FlatGeobuf"

as.data.frame(drv)[match(vapour_driver(f), drv$driver), ]
#>         driver       name vector raster create  copy virtual  sql_dialects
#> 166 FlatGeobuf FlatGeobuf   TRUE  FALSE   TRUE FALSE    TRUE OGRSQL SQLITE
```
