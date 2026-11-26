# Raster ground control points

Return any ground control points for a raster data set, if they exist.

## Usage

``` r
vapour_raster_gcp(x, ...)
```

## Arguments

- x:

  data source string (i.e. file name or URL or database connection
  string)

- ...:

  ignored currently

## Value

list with

- `Pixel` the pixel coordinate

- `Line` the line coordinate

- `X` the X coordinate of the GCP

- `Y` the Y coordinate of the GCP

- `Z` the Z coordinate of the GCP (usually zero)

## Details

Pixel and Line coordinates do not correspond to cells in the underlying
raster grid, they refer to the index space of that array in 0, ncols and
0, nrows. They are usually a subsample of the grid and may not align
with the grid spacing itself (though they often do in satellite remote
sensing products).

The coordinate system of the GCPs is currently not read.

## Examples

``` r
## this file has no ground control points
## they are rare, and tend to be in large files
f <- system.file("extdata", "sst.tif", package = "vapour")
vapour_raster_gcp(f)
#> No GCP (ground control points) found.
#> $Pixel
#> NULL
#> 
#> $Line
#> NULL
#> 
#> $X
#> NULL
#> 
#> $Y
#> NULL
#> 
#> $Z
#> NULL
#> 
#> $CRS
#> NULL
#> 

## a very made-up example with no real use
f1 <- system.file("extdata/gcps", "volcano_gcp.tif", package = "vapour")
vapour_raster_gcp(f1)
#> $Pixel
#> [1]  0  5 20
#> 
#> $Line
#> [1]  0  5 15
#> 
#> $X
#> [1] 100 200 300
#> 
#> $Y
#> [1] 100 200 300
#> 
#> $Z
#> [1] 0 0 0
#> 
#> $CRS
#> [1] ""
#> 
```
