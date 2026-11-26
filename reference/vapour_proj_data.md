# Report on the PROJ data directory in use

Returns the PROJ search paths that GDAL's PROJ contexts are using,
whether the copy of PROJ data bundled with vapour (if any) is active,
and the metadata of the proj.db that resolves first on that path.

## Usage

``` r
vapour_proj_data()
```

## Value

a list with elements 'search_paths', 'bundled_active', 'proj_version',
'proj_db', and 'message' (the startup message explaining why a bundled
copy was ignored, or NULL)

## Examples

``` r
vapour_proj_data()
#> $search_paths
#> [1] "/home/runner/.local/share/proj" "/usr/share/proj"               
#> 
#> $bundled_active
#> [1] FALSE
#> 
#> $proj_version
#> [1] "9.7.1"
#> 
#> $proj_db
#>                                                                              path 
#>                                                         "/usr/share/proj/proj.db" 
#>                                                     DATABASE.LAYOUT.VERSION.MAJOR 
#>                                                                               "1" 
#>                                                     DATABASE.LAYOUT.VERSION.MINOR 
#>                                                                               "6" 
#>                                                                         EPSG.DATE 
#>                                                                      "2025-10-03" 
#>                                                                      EPSG.VERSION 
#>                                                                         "v12.029" 
#>                                                                         ESRI.DATE 
#>                                                                      "2025-05-11" 
#>                                                                      ESRI.VERSION 
#>                                                                  "ArcGIS Pro 3.5" 
#>                                                                         IGNF.DATE 
#>                                                                      "2019-05-24" 
#>                                                                       IGNF.SOURCE 
#> "https://raw.githubusercontent.com/rouault/proj-resources/master/IGNF.v3.1.0.xml" 
#>                                                                      IGNF.VERSION 
#>                                                                           "3.1.0" 
#>                                                                          NKG.DATE 
#>                                                                      "2025-02-13" 
#>                                                                        NKG.SOURCE 
#>                          "https://github.com/NordicGeodesy/NordicTransformations" 
#>                                                                       NKG.VERSION 
#>                                                                           "1.0.w" 
#>                                                                      PROJ.VERSION 
#>                                                                           "9.7.1" 
#>                                                                 PROJ_DATA.VERSION 
#>                                                                            "1.24" 
#> 
#> $message
#> NULL
#> 
```
