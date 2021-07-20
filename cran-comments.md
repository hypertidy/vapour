## vapour 0.7.5

Fix for CRAN errors, with unsanitized type input (integer input for extent in warper, found on 
 clang-UBSAN tests and reported by CRAN 2021-07-04 and 2021-07-18). 

Thank you. 


## Test environments

* Linux Ubuntu 18.04 R 4.1.0
* win-builder (devel and release)


## R CMD check results

0 errors | 0 warnings | 1 note

There is a note about the size of installed directories on Windows and MacOS because 
 of the GDAL and PROJ folders. 



