# Raster information

Return the basic structural metadata of a raster source understood by
GDAL. Subdatasets may be specified by number, starting at 1. See
[vapour_sds_names](https://hypertidy.github.io/vapour/reference/vapour_sds_names.md)
for more.

## Usage

``` r
vapour_raster_info(x, ..., sds = NULL, min_max = FALSE)
```

## Arguments

- x:

  data source string (i.e. file name or URL or database connection
  string)

- ...:

  currently unused

- sds:

  a subdataset number, if necessary

- min_max:

  logical, control computing min and max values in source ('FALSE' by
  default)

## Value

list with vectors 'geotransform', 'dimXY', 'minmax', 'block',
'projection', 'bands', 'projstring', 'nodata_value', 'overviews',
'filelist' see sections in Details for more on each element

## Details

The structural metadata are

- extent:

  the extent of the data, xmin, xmax, ymin, ymax - these are the lower
  left and upper right corners of pixels

- geotransform:

  the affine transform

- dimension:

  dimensions x-y, columns\*rows

- minmax:

  numeric values of the computed min and max from the first band
  (optional)

- block:

  dimensions x-y of internal tiling scheme

- projection:

  text version of map projection parameter string

- bands:

  number of bands in the dataset

- projstring:

  the proj string version of 'projection'

- nodata_value:

  not implemented

- overviews:

  the number and size of any available overviews

- filelist:

  the list of files involved (may be none, and so will be a single NA
  character value)

- datatype:

  the band type name, in GDAL form 'Byte', 'Int16', 'Float32', etc.

- subdatasets:

  any subdataset DSNs is present, otherwise `NULL`

- corners:

  corner coordinates of the data, for non-zero skew geotransforms a
  2-column matrix with rows upperLeft, lowerLeft, lowerRight,
  upperRight, and center

Note that the geotransform is a kind of obscure combination of the
extent and dimension, I don't find it useful and modern GDAL is moving
away from needing it so much. Extent is more sensible and used in many
places in a straightforward way.

On access vapour functions will report on the existence of subdatasets
while defaulting to the first subdataset found.

## Subdatasets

Some sources provide multiple data sets, where a dataset is described by
a 2- (or more) dimensional grid whose structure is described by the
metadata described above. Note that *subdataset* is a different concept
to *band or dimension*. Sources that may have multiple data sets are
HDF4/HDF5 and NetCDF, and they are loosely analogous to the concept of
*layer* in GDAL vector data. Variables are usually seen as distinct data
but in GDAL and related 2D-interpretations this concept is leveraged as
a 3rd dimension (and higher). In a GeoTIFF a third dimension might be
implicit across bands, i.e. to express time varying data and so each
band is not properly a variable. Similarly in NetCDF, the data may be
any dimensional but there's only an implicit link for other variables
that exist in that same dimensional space. When using GDAL you are
always traversing this confusing realm.

If subdatasets are present but not specified the first is queried. The
choice of subdataset is analogous to the way that the `raster` package
behaves, and uses the argument `varname`. Variables in NetCDF correspond
to subdatasets, but a single data set might have multiple variables in
different bands or in dimensions, so this guide does not hold across
various systems.

## The Geo Transform

From <https://gdal.org/en/stable/user/raster_data_model.html>.

The affine transform consists of six coefficients returned by
`GDALDataset::GetGeoTransform()` which map pixel/line coordinates into
georeferenced space using the following relationship:

`Xgeo = GT(0) + Xpixel*GT(1) + Yline*GT(2)`

`Ygeo = GT(3) + Xpixel*GT(4) + Yline*GT(5)`

They are

- GT0, xmin:

  the x position of the lower left corner of the lower left pixel

- GT1, xres:

  the scale of the x-axis, the width of the pixel in x-units

- GT2, yskew:

  y component of the pixel width

- GT3, ymax:

  the y position of the upper left corner of the upper left pixel

- GT4, xskew:

  x component of the pixel height

- GT5, yres:

  the scale of the y-axis, the height of the pixel in *negative* y-units

Please note that these coefficients are equivalent to the contents of a
*world file* but that the order is not the same and the world file uses
cell centre convention rather than edge.
<https://en.wikipedia.org/wiki/World_file>

Usually the skew components are zero, and so only four coefficients are
relevant and correspond to the offset and scale used to position the
raster - in combination with the number of rows and columns of data they
provide the spatial extent and the pixel size in each direction. Very
rarely an actual affine raster will be used with this *rotation*
specified within the transform coefficients.

Calculation of 'minmax' can take a significant amount of time, so it's
not done by default. Use `min_max = TRUE` to do it. (It does perform
well, but may be prohibitive for very large or remote sources.)

## Overviews

If there are no overviews this element will simply be a single-element
vector of value 0. If there are overviews, the first value will give the
number of overviews and their dimensions will be listed as pairs of x,y
values.

## See also

vapour_sds_names

## Examples

``` r
f <- system.file("extdata", "sst.tif", package = "vapour")
vapour_raster_info(f)
#> $geotransform
#> [1] 140.00000000   0.07000000   0.00000000 -39.99722207   0.00000000
#> [6]  -0.07000389
#> 
#> $dimension
#> [1] 143 286
#> 
#> $dimXY
#> [1] 143 286
#> 
#> $minmax
#> [1] 271.350 289.859
#> 
#> $block
#> [1] 143  14
#> 
#> $projection
#> [1] "GEOGCRS[\"WGS 84\",\n    ENSEMBLE[\"World Geodetic System 1984 ensemble\",\n        MEMBER[\"World Geodetic System 1984 (Transit)\"],\n        MEMBER[\"World Geodetic System 1984 (G730)\"],\n        MEMBER[\"World Geodetic System 1984 (G873)\"],\n        MEMBER[\"World Geodetic System 1984 (G1150)\"],\n        MEMBER[\"World Geodetic System 1984 (G1674)\"],\n        MEMBER[\"World Geodetic System 1984 (G1762)\"],\n        MEMBER[\"World Geodetic System 1984 (G2139)\"],\n        MEMBER[\"World Geodetic System 1984 (G2296)\"],\n        ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n            LENGTHUNIT[\"metre\",1]],\n        ENSEMBLEACCURACY[2.0]],\n    PRIMEM[\"Greenwich\",0,\n        ANGLEUNIT[\"degree\",0.0174532925199433]],\n    CS[ellipsoidal,2],\n        AXIS[\"geodetic latitude (Lat)\",north,\n            ORDER[1],\n            ANGLEUNIT[\"degree\",0.0174532925199433]],\n        AXIS[\"geodetic longitude (Lon)\",east,\n            ORDER[2],\n            ANGLEUNIT[\"degree\",0.0174532925199433]],\n    USAGE[\n        SCOPE[\"Horizontal component of 3D system.\"],\n        AREA[\"World.\"],\n        BBOX[-90,-180,90,180]],\n    ID[\"EPSG\",4326]]"
#> 
#> $bands
#> [1] 1
#> 
#> $projstring
#> [1] "+proj=longlat +datum=WGS84 +no_defs"
#> 
#> $nodata_value
#> [1] -3.4e+38
#> 
#> $overviews
#> NULL
#> 
#> $filelist
#> [1] "/home/runner/work/_temp/Library/vapour/extdata/sst.tif"
#> 
#> $datatype
#> [1] "Float32"
#> 
#> $extent
#> [1] 140.00000 150.01000 -60.01833 -39.99722
#> 
#> $subdatasets
#> NULL
#> 
#> $corners
#>               [,1]      [,2]
#> upperLeft  140.000 -39.99722
#> lowerLeft  140.000 -60.01833
#> lowerRight 150.010 -60.01833
#> upperRight 150.010 -39.99722
#> center     145.005 -50.00778
#> 
```
