## vapour 0.9.2

* Removed hardcoded -lsqlite3 from configure.ac, as per helpful CRAN email. 

New submission after confirmation on mac builder,  same pkg-config --libs as expected from CRAN email, use LDFLAGS in load tests. 

Macbuilder results are here: https://mac.r-project.org/macbuilder/results/1668480796-f750737d607cc85b/

Thanks! 


## Test environments

* win-builder (devel and release)
* Linux ubuntu 4.2.1

## R CMD check results

0 errors | 0 warnings | 1 note

There is a note about the size of installed directories  because 
 of the GDAL and PROJ folders. 


