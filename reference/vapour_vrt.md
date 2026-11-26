# Virtual raster

Simple VRT creation of a GDAL virtual raster. The data source string is
**augmented** by input of other optional arguments. That means it
**overrides** their values provided by the source data, or stands in
place of this information if it is missing.

## Usage

``` r
vapour_vrt(
  x,
  extent = NULL,
  projection = NULL,
  sds = 1L,
  bands = NULL,
  geolocation = NULL,
  ...,
  relative_to_vrt = FALSE,
  nomd = FALSE,
  overview = -1L,
  options = character()
)
```

## Arguments

- x:

  data source name, filepath, url, database connection string, or VRT
  text

- extent:

  (optional) numeric extent, xmin,xmax,ymin,ymax

- projection:

  (optional) character string, projection string ("auth:code", proj4, or
  WKT, or anything understood by PROJ, see Details)

- sds:

  which subdataset to select from a source with more than one

- bands:

  (optional) which band/s to include from the source

- geolocation:

  vector of 2 dsn to longitude, latitude geolocation array sources

- ...:

  ignored

- relative_to_vrt:

  default `FALSE`, if `TRUE` input strings that identify as files on the
  system are left as-is (by default they are made absolute at the R
  level)

- nomd:

  if `TRUE` the Metadata tag is removed from the resulting VRT (it can
  be quite substantial)

- overview:

  pick an integer overview from the source (0L is highest resolution,
  default -1L does nothing)

- options:

  pass in options to the VRT creation, like 'c("-expand", "rgb", "-ot",
  "Byte"('

## Value

VRT character string (for use by GDAL-capable tools, i.e. reading
raster)

## Details

Create a GDAL data source string (to be used like a filename) with
various helpers. VRT stands for 'ViRTual'. A VRT string then acts as a
representative of a data source for further use (to read or warp it).

An input string will be converted to a single subdataset, use 'sds'
argument to select.

If 'extent', 'projection' is provided this is applied to override the
source's extent and/or projection. (These might be invalid, or missing,
so we facilitate correcting this).

If 'bands' is provided this is used to select a set of bands (numbered
from 1), which might be repeated, or in any order and contain
repetitions.

`vapour_vrt()` is vectorized, it will return multiple VRT strings for
multiple inputs in a "length \> 1" character vector. These are all
independent, this is different to the function
[`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md)
where multiple inputs are merged (possibly by sequential overlapping).

If `geolocation` is set the 'GeoTransform' element is forcibly removed
from the vrt output, in order to avoid
https://github.com/hypertidy/vapour/issues/210 (there might be a better
fix).

## Rationale

For a raster, the basic essentials we can specify or modify for a source
are

1.  the source, 2) the extent, 3) the projection 4) what subdataset
    (these are variables from NetCDF and the like that contain multiple
    datasets) and 5) which band/s to provided. For extent and projection
    we are simply providing or correcting complete information about how
    to interpret the georeferencing, with subdatasets and bands this is
    more like a query of which ones we want. If we only wanted band 5,
    then the output data would have one band only (and we we read it we
    need `band = 1`).

We don't provide ability override the dimension, but that is possible as
well. More features may come with a 'VRTBuilder' interface.

## Projections

Common inputs for `projection` are WKT variants, "AUTH:CODE"s e.g.
"EPSG:3031", the "OGC:CRS84" for long,lat WGS84, "ESRI:code" and other
authority variants, and datum names such as 'WGS84','NAD27' recognized
by PROJ itself.

See the following links to GDAL and PROJ documentation:

[PROJ documentation:
c.proj_create_crs_to_crs](https://proj.org/development/reference/functions.html#c.proj_create_crs_to_crs)

[PROJ documentation:
c.proj_create](https://proj.org/development/reference/functions.html#c.proj_create)

[GDAL documentation:
SetFromUserInput](https://gdal.org/en/stable/doxygen/classOGRSpatialReference.html)

## Examples

``` r
tif <- system.file("extdata", "sst.tif", package = "vapour")
vapour_vrt(tif)
#> [1] "<VRTDataset rasterXSize=\"143\" rasterYSize=\"286\">\n  <SRS dataAxisToSRSAxisMapping=\"2,1\">GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AXIS[\"Latitude\",NORTH],AXIS[\"Longitude\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]</SRS>\n  <GeoTransform>  1.4000000000000000e+02,  6.9999999999999937e-02,  0.0000000000000000e+00, -3.9997222067892665e+01,  0.0000000000000000e+00, -7.0003889104950298e-02</GeoTransform>\n  <Metadata>\n    <MDI key=\"AREA_OR_POINT\">Area</MDI>\n  </Metadata>\n  <Metadata domain=\"IMAGE_STRUCTURE\">\n    <MDI key=\"INTERLEAVE\">BAND</MDI>\n    <MDI key=\"COMPRESSION\">LZW</MDI>\n  </Metadata>\n  <VRTRasterBand dataType=\"Float32\" band=\"1\" blockXSize=\"143\" blockYSize=\"14\">\n    <Metadata>\n      <MDI key=\"STATISTICS_MAXIMUM\">289.859</MDI>\n      <MDI key=\"STATISTICS_MEAN\">279.8360951673</MDI>\n      <MDI key=\"STATISTICS_MINIMUM\">271.35</MDI>\n      <MDI key=\"STATISTICS_STDDEV\">4.9558170498586</MDI>\n    </Metadata>\n    <NoDataValue>-3.39999995214436425e+38</NoDataValue>\n    <ColorInterp>Gray</ColorInterp>\n    <SimpleSource>\n      <SourceFilename relativeToVRT=\"0\">/home/runner/work/_temp/Library/vapour/extdata/sst.tif</SourceFilename>\n      <SourceBand>1</SourceBand>\n      <SourceProperties RasterXSize=\"143\" RasterYSize=\"286\" DataType=\"Float32\" BlockXSize=\"143\" BlockYSize=\"14\" />\n      <SrcRect xOff=\"0\" yOff=\"0\" xSize=\"143\" ySize=\"286\" />\n      <DstRect xOff=\"0\" yOff=\"0\" xSize=\"143\" ySize=\"286\" />\n    </SimpleSource>\n  </VRTRasterBand>\n</VRTDataset>\n"

vapour_vrt(tif, bands = c(1, 1))
#> [1] "<VRTDataset rasterXSize=\"143\" rasterYSize=\"286\">\n  <SRS dataAxisToSRSAxisMapping=\"2,1\">GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AXIS[\"Latitude\",NORTH],AXIS[\"Longitude\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]</SRS>\n  <GeoTransform>  1.4000000000000000e+02,  6.9999999999999937e-02,  0.0000000000000000e+00, -3.9997222067892665e+01,  0.0000000000000000e+00, -7.0003889104950298e-02</GeoTransform>\n  <Metadata>\n    <MDI key=\"AREA_OR_POINT\">Area</MDI>\n  </Metadata>\n  <Metadata domain=\"IMAGE_STRUCTURE\">\n    <MDI key=\"INTERLEAVE\">BAND</MDI>\n    <MDI key=\"COMPRESSION\">LZW</MDI>\n  </Metadata>\n  <VRTRasterBand dataType=\"Float32\" band=\"1\" blockXSize=\"143\" blockYSize=\"14\">\n    <Metadata>\n      <MDI key=\"STATISTICS_MAXIMUM\">289.859</MDI>\n      <MDI key=\"STATISTICS_MEAN\">279.8360951673</MDI>\n      <MDI key=\"STATISTICS_MINIMUM\">271.35</MDI>\n      <MDI key=\"STATISTICS_STDDEV\">4.9558170498586</MDI>\n    </Metadata>\n    <NoDataValue>-3.39999995214436425e+38</NoDataValue>\n    <ColorInterp>Gray</ColorInterp>\n    <SimpleSource>\n      <SourceFilename relativeToVRT=\"0\">/home/runner/work/_temp/Library/vapour/extdata/sst.tif</SourceFilename>\n      <SourceBand>1</SourceBand>\n      <SourceProperties RasterXSize=\"143\" RasterYSize=\"286\" DataType=\"Float32\" BlockXSize=\"143\" BlockYSize=\"14\" />\n      <SrcRect xOff=\"0\" yOff=\"0\" xSize=\"143\" ySize=\"286\" />\n      <DstRect xOff=\"0\" yOff=\"0\" xSize=\"143\" ySize=\"286\" />\n    </SimpleSource>\n  </VRTRasterBand>\n  <VRTRasterBand dataType=\"Float32\" band=\"2\" blockXSize=\"143\" blockYSize=\"14\">\n    <Metadata>\n      <MDI key=\"STATISTICS_MAXIMUM\">289.859</MDI>\n      <MDI key=\"STATISTICS_MEAN\">279.8360951673</MDI>\n      <MDI key=\"STATISTICS_MINIMUM\">271.35</MDI>\n      <MDI key=\"STATISTICS_STDDEV\">4.9558170498586</MDI>\n    </Metadata>\n    <NoDataValue>-3.39999995214436425e+38</NoDataValue>\n    <ColorInterp>Gray</ColorInterp>\n    <SimpleSource>\n      <SourceFilename relativeToVRT=\"0\">/home/runner/work/_temp/Library/vapour/extdata/sst.tif</SourceFilename>\n      <SourceBand>1</SourceBand>\n      <SourceProperties RasterXSize=\"143\" RasterYSize=\"286\" DataType=\"Float32\" BlockXSize=\"143\" BlockYSize=\"14\" />\n      <SrcRect xOff=\"0\" yOff=\"0\" xSize=\"143\" ySize=\"286\" />\n      <DstRect xOff=\"0\" yOff=\"0\" xSize=\"143\" ySize=\"286\" />\n    </SimpleSource>\n  </VRTRasterBand>\n</VRTDataset>\n"
```
