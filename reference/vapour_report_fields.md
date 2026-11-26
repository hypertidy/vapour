# Read feature field types.

Obtains the internal type-constant name for the data attributes in a
source.

## Usage

``` r
vapour_report_fields(dsource, layer = 0L, sql = "", open_options = NULL)

vapour_report_attributes(dsource, layer = 0L, sql = "", open_options = NULL)
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

named character vector of the GDAL types for each field

## Details

Use this to compare the interpreted versions converted into R types by
`vapour_read_fields`.

This and
[`vapour_read_fields()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md)
are aliased to older versions named 'vapour_report_attributes()' and
'vapour_read_attributes()', but "field" is a clearer and more sensible
name (in our opinion).

These are defined for the enum OGRFieldType in GDAL itself.
<https://gdal.org/en/stable/doxygen/ogr__core_8h.html>

## Examples

``` r
file <- "list_locality_postcode_meander_valley.tab"
mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
vapour_report_fields(mvfile)
#>     LOCAL_ID         NAME     POSTCODE     PLAN_REF     GAZ_DATE   NOM_REG_NO 
#>    "OFTReal"  "OFTString" "OFTInteger"  "OFTString"    "OFTDate"  "OFTString" 
#>          UFI   CREATED_ON    LIST_GUID   SHAPE_AREA    SHAPE_LEN 
#>  "OFTString"  "OFTString"  "OFTString"    "OFTReal"    "OFTReal" 

## modified by sql argument
vapour_report_fields(mvfile,
  sql = "SELECT POSTCODE, NAME FROM list_locality_postcode_meander_valley")
#>     POSTCODE         NAME 
#> "OFTInteger"  "OFTString" 
```
