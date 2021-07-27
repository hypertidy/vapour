## vapour 0.7.6

Fix for CRAN errors, with unsanitized type input (int type for double in warper nodata) found on  clang-UBSAN tests and 
re-reported by CRAN on 2021-07-25. 

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



