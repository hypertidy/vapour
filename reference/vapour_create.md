# Create raster file

This is in an incomplete interface to raster writing, for exploring.

## Usage

``` r
vapour_create_options(driver = "GTiff")

vapour_create(
  filename,
  driver = "GTiff",
  extent = c(-180, 180, -90, 90),
  dimension = c(2048, 1024),
  projection = "EPSG:4326",
  n_bands = 1L,
  overwrite = FALSE,
  datatype = "Float32",
  options = vapour_create_options(driver)
)
```

## Arguments

- driver:

  GDAL driver to use (GTiff is default, and recommended)

- filename:

  filename/path to create

- extent:

  xmin,xmax,ymin,ymax 4-element vector

- dimension:

  dimension of the output, X \* Y

- projection:

  projection of the output, best to use a full WKT but any string
  accepted

- n_bands:

  number of bands in the output, default is 1

- overwrite:

  not TRUE by default

- datatype:

  the name of a GDAL datatype ('Float32', 'Int64', etc)

- options:

  character vector of creation of options for the driver in use
  `c('COMPRESS=DEFLATE')` note how these are constructed (no '-co'
  element)

## Value

the file path that was created

## Details

If GeoTIFF is used (`driver = "GTiff"`, recommended) then the output is
tiled 512x512, and has DEFLATE compression, and is sparse when created
(no values are initiated, so the file is tiny).

Note that there is no restriction on where you can read or write from,
the responsibility is yours. There is no auto driver detection done for
the file format, it's up to you to set the file extension *and* the
driver.

File is created using CreateCopy from a VRT in memory. This is so that
we can instantiate COG layer with 'driver = "COG"'. Please note that
performance is best for GTiff itself, with 'SPARSE_OK=YES'. We don't yet
know how to instantiate a large COG with overviews.

There are default creation options set for COG and GTiff drivers, see
'vapour_create_options(driver "GTiff")' for what those are.

## Examples

``` r
tfile <- tempfile(fileext = ".tif")
if (!file.exists(tfile)) {
 vapour_create(tfile, extent = c(-1, 1, -1, 1) * 1e6, 
                     dimension = c(128, 128), 
                     projection = "+proj=laea")
 file.remove(tfile)
}
#> [1] TRUE
```
