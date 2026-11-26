# type safe(r) raster warp

These wrappers around
[`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md)
guarantee single vector output of the nominated type.

## Usage

``` r
vapour_warp_raster_raw(
  x,
  bands = NULL,
  extent = NULL,
  dimension = NULL,
  projection = "",
  set_na = TRUE,
  source_projection = NULL,
  source_extent = 0,
  resample = "near",
  silent = TRUE,
  ...,
  warp_options = "",
  transformation_options = "",
  open_options = "",
  options = ""
)

vapour_warp_raster_int(
  x,
  bands = NULL,
  extent = NULL,
  dimension = NULL,
  projection = "",
  set_na = TRUE,
  source_projection = NULL,
  source_extent = 0,
  resample = "near",
  silent = TRUE,
  ...,
  warp_options = "",
  transformation_options = "",
  open_options = "",
  options = ""
)

vapour_warp_raster_dbl(
  x,
  bands = NULL,
  extent = NULL,
  dimension = NULL,
  projection = "",
  set_na = TRUE,
  source_projection = NULL,
  source_extent = 0,
  resample = "near",
  silent = TRUE,
  ...,
  warp_options = "",
  transformation_options = "",
  open_options = "",
  options = ""
)

vapour_warp_raster_chr(
  x,
  bands = NULL,
  extent = NULL,
  dimension = NULL,
  projection = "",
  set_na = TRUE,
  source_projection = NULL,
  source_extent = 0,
  resample = "near",
  silent = TRUE,
  ...,
  warp_options = "",
  transformation_options = "",
  open_options = "",
  options = ""
)

vapour_warp_raster_hex(
  x,
  bands = NULL,
  extent = NULL,
  dimension = NULL,
  projection = "",
  set_na = TRUE,
  source_projection = NULL,
  source_extent = 0,
  resample = "near",
  silent = TRUE,
  ...,
  warp_options = "",
  transformation_options = "",
  open_options = "",
  options = ""
)
```

## Arguments

- x:

  vector of data source names (file name or URL or database connection
  string)

- bands:

  index of band/s to read (1-based), may be repeated or in any order, or
  `NULL` for all bands (the default)

- extent:

  extent of the target warped raster 'c(xmin, xmax, ymin, ymax)'

- dimension:

  dimensions in pixels of the warped raster (x, y)

- projection:

  projection of warped raster (in Well-Known-Text, or any projection
  string accepted by GDAL)

- set_na:

  NOT IMPLEMENTED logical, should 'NODATA' values be set to `NA`

- source_projection:

  optional, override or augment the projection of the source (in
  Well-Known-Text, or any projection string accepted by GDAL)

- source_extent:

  extent of the source raster, used to override/augment incorrect source
  metadata

- resample:

  resampling method used (see details in
  [vapour_read_raster](https://hypertidy.github.io/vapour/reference/vapour_read_raster.md))

- silent:

  `TRUE` by default, set to `FALSE` to report messages

- ...:

  unused

- warp_options:

  character vector of options, as in gdalwarp -wo - see Details

- transformation_options:

  character vector of options, as in gdalwarp -to see Details

- open_options:

  character vector of options, as in gdalwarp -oo - see Details

- options:

  character vectors of options as per the gdalwarp command line

## Value

atomic vector of the nominated type raw, int, dbl, or character (hex)

## Details

\_hex and \_chr are aliases of each other.

## Examples

``` r
b <- 4e5
f <- system.file("extdata", "sst.tif", package = "vapour")
prj <- "+proj=aeqd +lon_0=147 +lat_0=-42"
bytes <- vapour_warp_raster_raw(f, extent = c(-b, b, -b, b),
                             dimension = c(18, 2),
                             bands = 1, 
                             projection = prj)
# not useful given source type floating point, but works
str(bytes)
#>  raw [1:36] ff ff ff ff ...
```
