# Write data to a block *in an existing file*.

Be careful! The write function doesn't create a file, you have to use an
existing one. Don't write to a file you don't want to update by mistake.

## Usage

``` r
vapour_write_raster_block(
  dsource,
  data,
  offset,
  dimension,
  band = 1L,
  overwrite = FALSE,
  open_options = character()
)
```

## Arguments

- dsource:

  data source name

- data:

  data vector, length should match `prod(dimension)` or length 1 allowed

- offset:

  offset to start

- dimension:

  dimension to write

- band:

  which band to write to (1-based)

- overwrite:

  set to FALSE as a safety valve to not overwrite an existing file

- open_options:

  character vector of open options for the GDAL driver (e.g.
  `c("IGNORE_COG_LAYOUT_BREAK=YES")`)

## Value

a logical value indicating success (or failure) of the write

## Examples

``` r
f <- system.file("extdata", "sst.tif", package = "vapour")
v <- vapour_read_raster_block(f, c(0L, 0L), dimension = c(2L, 3L), band = 1L)
file.copy(f, tf <- tempfile(fileext = ".tif"))
#> [1] TRUE
try(vapour_write_raster_block(tf, data = v[[1]], offset = c(0L, 0L), 
               dimension = c(2L, 3L), band = 1L))
#> Error in vapour_write_raster_block(tf, data = v[[1]], offset = c(0L, 0L),  : 
#>   set 'overwrite' to TRUE if you really mean to write to file /tmp/Rtmp5dOcc9/file201bf56c1a3.tif
if (file.exists(tf)) file.remove(tf)
#> [1] TRUE
```
