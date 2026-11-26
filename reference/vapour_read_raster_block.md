# Read or write raster block

Read a 'block' from raster.

## Usage

``` r
vapour_read_raster_block(
  dsource,
  offset,
  dimension,
  band = 1L,
  band_output_type = "",
  unscale = TRUE,
  nara = FALSE,
  open_options = NULL
)
```

## Arguments

- dsource:

  file name to read from, or write to

- offset:

  position x,y to start writing (0-based, y-top)

- dimension:

  window size to read from, or write to

- band:

  which band to read (1-based)

- band_output_type:

  numeric type of band to apply (else the native type if ”) can be one
  of 'Byte', 'Int32', or 'Float64'

- unscale:

  default is `TRUE` so native values will be converted by offset and
  scale to floating point

- nara:

  if 'TRUE' return in nativeRaster format

- open_options:

  open options for the driver, a character vector of `"NAME=VALUE"`
  strings (e.g. `"IGNORE_COG_LAYOUT_BREAK=YES"` to write to a COG),
  `NULL` for none

## Value

a list with a vector of data from the band read

## Examples

``` r
f <- system.file("extdata", "sst.tif", package = "vapour")
v <- vapour_read_raster_block(f, c(0L, 0L), dimension = c(2L, 3L), band = 1L)
```
