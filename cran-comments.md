## vapour 0.7.7

Fix for CRAN errors, with required version of minimum version of GDAL now 2.3.0. 

There was a change in 0.7.6 to use `OGRGeometry::importFromWkt()` from GDAL, with syntax
requirement for 2.3.0. This triggered the error on Solaris. 

There was unused code in src/vapour.cpp that triggered warnings on fedora devel clang about 
unused variable and function. 

Thank you!

## Test environments

* Linux Ubuntu 18.04 R 4.1.0
* win-builder (devel and release)


## R CMD check results

0 errors | 0 warnings | 1 note

There is a note about the size of installed directories on Windows and MacOS because 
 of the GDAL and PROJ folders. 

## Reverse dependencies

{lazyraster} passes check with this version



