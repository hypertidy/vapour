## vapour 0.5.5

Release to fix error on CRAN results (unsupported NetCDF by GDAL lib), and to ensure proj/ and gdal/ files are 
able to be copied to the MacOS tarball by CRAN request. 

## Test environments

* Linux Ubuntu 18.04 R 4.0.0
* MacOS (gh actions), R 4.0.0
* win-builder (devel and release)


## R CMD check results

0 errors | 0 warnings | 1 note

There is a note about the size of installed directories on Windows and MacOS because 
 of the GDAL and PROJ folders. 



