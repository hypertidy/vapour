# Is the CRS string representative of angular coordinates

Returns `TRUE` if this is longitude latitude data. Missing, malformed,
zero-length values are disallowed.

## Usage

``` r
vapour_crs_is_lonlat(crs)
```

## Arguments

- crs:

  character string of length 1

## Value

logical value `TRUE` for lonlat, `FALSE` otherwise

## Examples

``` r
vapour_gdal_version() ## versions to catch problems with string input
#> [1] "GDAL 3.12.2 \"Chicoutimi\", released 2026/02/03"
vapour_proj_version()
#> [1] "9.7.1"
vapour_crs_is_lonlat("+proj=aeqd +lon_0=147 +lat_0=-42")
#> [1] FALSE
vapour_crs_is_lonlat("EPSG:4326")
#> [1] TRUE
vapour_srs_wkt("+proj=laea")
#> [1] "PROJCRS[\"unknown\",\n    BASEGEOGCRS[\"unknown\",\n        DATUM[\"World Geodetic System 1984\",\n            ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n                LENGTHUNIT[\"metre\",1]],\n            ID[\"EPSG\",6326]],\n        PRIMEM[\"Greenwich\",0,\n            ANGLEUNIT[\"degree\",0.0174532925199433],\n            ID[\"EPSG\",8901]]],\n    CONVERSION[\"unknown\",\n        METHOD[\"Lambert Azimuthal Equal Area\",\n            ID[\"EPSG\",9820]],\n        PARAMETER[\"Latitude of natural origin\",0,\n            ANGLEUNIT[\"degree\",0.0174532925199433],\n            ID[\"EPSG\",8801]],\n        PARAMETER[\"Longitude of natural origin\",0,\n            ANGLEUNIT[\"degree\",0.0174532925199433],\n            ID[\"EPSG\",8802]],\n        PARAMETER[\"False easting\",0,\n            LENGTHUNIT[\"metre\",1],\n            ID[\"EPSG\",8806]],\n        PARAMETER[\"False northing\",0,\n            LENGTHUNIT[\"metre\",1],\n            ID[\"EPSG\",8807]]],\n    CS[Cartesian,2],\n        AXIS[\"(E)\",east,\n            ORDER[1],\n            LENGTHUNIT[\"metre\",1,\n                ID[\"EPSG\",9001]]],\n        AXIS[\"(N)\",north,\n            ORDER[2],\n            LENGTHUNIT[\"metre\",1,\n                ID[\"EPSG\",9001]]]]"
vapour_crs_is_lonlat("+proj=laea +type=crs")
#> [1] FALSE
vapour_crs_is_lonlat("OGC:CRS84")
#> [1] TRUE
vapour_crs_is_lonlat("WGS84")
#> [1] TRUE
vapour_crs_is_lonlat("NAD27")
#> [1] TRUE
vapour_crs_is_lonlat("EPSG:3031")
#> [1] FALSE
```
