## vapour 0.9.2

* Removed hardcoded -lsqlite3 from configure.ac, as per helpful CRAN email. 

New submission after confirmation on mac builder, confirmed that it works, same pkg-config --libs as expected from CRAN email. 

Thanks! 

## vapour 0.9.1

New version to address Additional Issues on CRAN. I've tested for lossy casts on clang with -Wconversion. 

Thank you! 

## Test environments

* win-builder (devel and release)
* Linux ubuntu 4.2.1

## R CMD check results

0 errors | 0 warnings | 1 note

There is a note about the size of installed directories  because 
 of the GDAL and PROJ folders. 


