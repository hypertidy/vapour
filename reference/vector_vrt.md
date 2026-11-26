# Vector VRT

Just a simple text generator to generate the VRT for a vector layer,
First layer is chosen if not otherwise specified.

## Usage

``` r
vector_vrt(x, layer = 1L, projection = NULL, sql = NULL, a_srs = NULL)
```

## Arguments

- x:

  data source name

- layer:

  layer index (1-based) or name

- projection:

  crs of the output

- sql:

  SQL for ExecuteSQL to define the layer

- a_srs:

  set the source crs

## Value

single element character vector

## Details

Using 'sql' overrides the 'layer', and using 'projection' results in the
geometries being transformed. No check is made of the layer source
projection.

Use 'a_srs' to ensure the source has a source crs (that might be the
only thing you use this for, even if not reprojecting).

It's expected that if you use this with a source without a source
projection, you'll get "Failed to import source SRS", so use argument
"a_srs" to set it if needed (or many other GDAL other facilities that do
this).

## Examples

``` r
file <- "list_locality_postcode_meander_valley.tab"
## A MapInfo TAB file with polygons
mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
vector_vrt(mvfile, sql = "SELECT * FROM list_locality_postcode_meander_valley LIMIT 5 OFFSET 4")
#> [1] "<OGRVRTDataSource>\n     <OGRVRTLayer name=\"list_locality_postcode_meander_valley\">\n    <SrcDataSource>/home/runner/work/_temp/Library/vapour/extdata/tab/list_locality_postcode_meander_valley.tab</SrcDataSource>\n    <SrcSQL>SELECT * FROM list_locality_postcode_meander_valley LIMIT 5 OFFSET 4</SrcSQL> \n    </OGRVRTLayer>\n </OGRVRTDataSource>"

## read this with vapour_read_geometry() and it will be projected to VicGrid
vector_vrt(mvfile, projection = "EPSG:3111")
#> [1] "<OGRVRTDataSource>\n  <OGRVRTWarpedLayer>\n     <OGRVRTLayer name=\"list_locality_postcode_meander_valley\">\n    <SrcDataSource>/home/runner/work/_temp/Library/vapour/extdata/tab/list_locality_postcode_meander_valley.tab</SrcDataSource>\n    <SrcLayer>list_locality_postcode_meander_valley</SrcLayer> \n    </OGRVRTLayer>\n    <TargetSRS>EPSG:3111</TargetSRS>\n    </OGRVRTWarpedLayer>\n </OGRVRTDataSource>"
```
