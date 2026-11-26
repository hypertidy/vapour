# Set and query GDAL configuration options

These functions can get and set configuration options for GDAL, for fine
control over specific GDAL behaviours.

## Usage

``` r
vapour_set_config(option, value)

vapour_get_config(option)
```

## Arguments

- option:

  GDAL config name (see Details), character string

- value:

  value for config option, character string

## Value

character string for `vapour_get_config`, integer 1 for successful
`vapour_set_config()`

## Details

Configuration options may also be set as environment variables.

See [GDAL config
options](https://gdal.org/en/stable/user/configoptions.html) for details
on available options.

## Examples

``` r
if (FALSE) { # \dontrun{
(orig <- vapour_get_config("GDAL_CACHEMAX"))
vapour_set_config("GDAL_CACHEMAX", "64")
vapour_get_config("GDAL_CACHEMAX")
vapour_set_config("GDAL_CACHEMAX", orig)
} # }
```
