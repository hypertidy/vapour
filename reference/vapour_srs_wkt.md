# PROJ4 string to WKT

Convert a projstring to Well Known Text.

## Usage

``` r
vapour_srs_wkt(crs)
```

## Arguments

- crs:

  projection string, see Details.

## Value

WKT2 projection string

## Details

The function is vectorized, though in practice it is almost always used
on a single character string.

Note that no sanitizing is done on inputs, we literally just
'OGRSpatialReference.SetFromUserInput(crs)' and give the output as WKT.
If it's an error in GDAL it's an error in R.

Common inputs are WKT variants, 'AUTH:CODE's e.g. 'EPSG:3031', the
'OGC:CRS84' for long,lat WGS84, 'ESRI:code' and other authority
variants, and datum names such as 'WGS84','NAD27' recognized by PROJ
itself.

See help for 'SetFromUserInput' in 'OGRSpatialReference', and
'proj_create_crs_to_crs'.

[c.proj_create_crs_to_crs](https://proj.org/development/reference/functions.html#c.proj_create_crs_to_crs)

[c.proj_create](https://proj.org/development/reference/functions.html#c.proj_create)

[SetFromUserInput](https://gdal.org/en/stable/doxygen/classOGRSpatialReference.html)

## Examples

``` r
vapour_srs_wkt("+proj=laea +datum=WGS84")
#> [1] "PROJCRS[\"unknown\",\n    BASEGEOGCRS[\"unknown\",\n        DATUM[\"World Geodetic System 1984\",\n            ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n                LENGTHUNIT[\"metre\",1]],\n            ID[\"EPSG\",6326]],\n        PRIMEM[\"Greenwich\",0,\n            ANGLEUNIT[\"degree\",0.0174532925199433],\n            ID[\"EPSG\",8901]]],\n    CONVERSION[\"unknown\",\n        METHOD[\"Lambert Azimuthal Equal Area\",\n            ID[\"EPSG\",9820]],\n        PARAMETER[\"Latitude of natural origin\",0,\n            ANGLEUNIT[\"degree\",0.0174532925199433],\n            ID[\"EPSG\",8801]],\n        PARAMETER[\"Longitude of natural origin\",0,\n            ANGLEUNIT[\"degree\",0.0174532925199433],\n            ID[\"EPSG\",8802]],\n        PARAMETER[\"False easting\",0,\n            LENGTHUNIT[\"metre\",1],\n            ID[\"EPSG\",8806]],\n        PARAMETER[\"False northing\",0,\n            LENGTHUNIT[\"metre\",1],\n            ID[\"EPSG\",8807]]],\n    CS[Cartesian,2],\n        AXIS[\"(E)\",east,\n            ORDER[1],\n            LENGTHUNIT[\"metre\",1,\n                ID[\"EPSG\",9001]]],\n        AXIS[\"(N)\",north,\n            ORDER[2],\n            LENGTHUNIT[\"metre\",1,\n                ID[\"EPSG\",9001]]]]"
```
