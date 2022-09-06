## vapour 0.8.82

This is a new submission as 0.8.81 was archived for new issues since 1 September 2022. 

I believe the problems currently (2022-09-06) that led to vapour being archived on 2022-09-04
are due purely to these obscure file format nuances. 

* Removed tests for obscure HDF5 and NetCDF files that are not supported on CRAN macos. 

## Test environments

* win-builder (devel)
* Linux ubuntu 4.2.1
* Github actions, mac, windows. 

## R CMD check results

0 errors | 0 warnings | 1 note

There is a note about the size of installed directories  because 
 of the GDAL and PROJ folders. 

## Reverse dependenices

The single revdep 'lazyraster' passes check with this version. 


