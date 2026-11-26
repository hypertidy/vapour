# General raster read and convert

The warper is used to convert source/s to an output file or to data in
memory.

## Usage

``` r
gdal_raster_data(
  dsn,
  target_crs = NULL,
  target_dim = NULL,
  target_ext = NULL,
  target_res = NULL,
  resample = "near",
  bands = 1L,
  band_output_type = NULL,
  options = character(),
  include_meta = TRUE,
  unscale = TRUE
)

gdal_raster_dsn(
  dsn,
  target_crs = NULL,
  target_dim = NULL,
  target_ext = NULL,
  target_res = NULL,
  resample = "near",
  bands = NULL,
  band_output_type = NULL,
  options = character(),
  out_dsn = tempfile(fileext = ".tif"),
  include_meta = TRUE
)

gdal_raster_image(
  dsn,
  target_crs = NULL,
  target_dim = NULL,
  target_ext = NULL,
  target_res = NULL,
  resample = "near",
  bands = NULL,
  band_output_type = NULL,
  options = character(),
  include_meta = TRUE
)

gdal_raster_nara(
  dsn,
  target_crs = NULL,
  target_dim = NULL,
  target_ext = NULL,
  target_res = NULL,
  resample = "near",
  bands = NULL,
  band_output_type = NULL,
  options = character(),
  include_meta = TRUE
)
```

## Arguments

- dsn:

  data sources, files, urls, db strings, vrt, etc

- target_crs:

  projection of the target grid

- target_dim:

  dimension of the target grid

- target_ext:

  extent of the target grid

- target_res:

  resolution of the target grid

- resample:

  resampling algorithm used

- bands:

  band or bands to include (1-based), may be repeated or in any order,
  or `NULL` for all bands. The default is the first band only, see
  Details for the image functions

- band_output_type:

  specify the band type, see
  [vapour_read_raster](https://hypertidy.github.io/vapour/reference/vapour_read_raster.md)

- options:

  general options passed to gdal warper

- include_meta:

  metadata is attached, turn off by setting this to `FALSE`

- unscale:

  default is `TRUE` so values are converted by the band's scale and
  offset to floating point, set to `FALSE` to get the stored values. An
  integer `band_output_type` also gives the stored values, see
  [vapour_read_raster](https://hypertidy.github.io/vapour/reference/vapour_read_raster.md)

- out_dsn:

  file name for output "\_dsn"

## Value

pixel values in a list vector per band, or a list of file paths

## Details

Two functions 'gdal_raster_data' and 'gdal_raster_dsn' act like the
gdalwarp command line tool, a convenience third function
'gdal_raster_image()' works especially for image data.

For `gdal_raster_image()` and `gdal_raster_nara()` the default
`bands = NULL` means the first bands up to four, which is what an image
can use. `gdal_raster_dsn()` writes every band, use
[`vapour_vrt()`](https://hypertidy.github.io/vapour/reference/vapour_vrt.md)
on the source to choose bands for it.

## Examples

``` r
dsn <- system.file("extdata/sst.tif", package = "vapour")
## do nothing, get native
X <- gdal_raster_data(dsn)

## set resolution (or dimension, extent, crs, or combination thereof - GDAL 
## will report/resolve incompatible opts)
X1 <- gdal_raster_data(dsn,  target_res = 1)

## add a cutline, and cut to it using gdal warp args

if (interactive()) {
 cutline <- tempfile(fileext = ".csv")
 wkt <- "POLYGON ((142 -41, 149 -41, 146 -58, 142 -41))"
 write.csv(data.frame(id = 1, WKT = wkt), cutline, row.names = FALSE)
 X1c <- gdal_raster_data(dsn,  target_res = .5, 
     options = c("-cutline",cutline, "-crop_to_cutline"))
 file.remove(cutline)
}

## warp whole grid to given res
X2 <- gdal_raster_data(dsn,  target_res = 25000, target_crs = "EPSG:32755")

## specify exactly (as per vapour originally)
X3 <- gdal_raster_data(dsn,  target_ext = c(-1, 1, -1, 1) * 8e6, 
 target_dim = c(512, 678), target_crs = "+proj=stere +lon_0=147 +lat_0=-90")
 
X4 <- gdal_raster_dsn(dsn, out_dsn = tempfile(fileext = ".tif"))
```
