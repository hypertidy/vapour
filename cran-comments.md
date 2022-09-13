## vapour 0.9.0

Resubmission, failures previous picked up in UBSAN incorrect use of NaN for raw/integer. I think I've fixed it by removing the offending code
which couldn't have worked and is only relevant when a user specifies an output type - so they get what they ask for. 

Thank you! 

## Test environments

* win-builder (devel)
* Linux ubuntu 4.2.1
* Rhub:  macos-highsierra-release-cran

## R CMD check results

0 errors | 0 warnings | 1 note

There is a note about the size of installed directories  because 
 of the GDAL and PROJ folders. 

## Reverse dependenices

The single revdep 'lazyraster' passes check with this version. 


