## vapour 0.9.3

* Patched CRAN supplied diff for Makevars.ucrt. 
* vapour doesn't write to files, so the GDAL 3.6.0 retraction is not relevant so we've avoided messaging
 from .onLoad as discussed by R Bivand. 
 
Thanks! 


## Test environments

* win-builder (devel and release)
* Linux ubuntu

## R CMD check results

0 errors | 0 warnings | 1 note

There is a note about the size of installed directories  because 
 of the GDAL and PROJ folders. 


