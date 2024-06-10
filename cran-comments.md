## vapour 0.10.0

To fix errors on CRAN. 

vapour is now passing ubuntu-clang and clang-asan via rhub and not segfaulting on the systems reported on 
CRAN results. 

There are valgrind messages but I think these are internal to GDAL from CSLAddString, CPLRealloc, GDALInfo and others and I will investigate further. 

## Test environments

* win-builder (devel)
* macos on rhub
* Linux ubuntu-clang, clang-asan on rhub. 

## R CMD check results

0 errors | 0 warnings | 1 note

There is a note about the size of installed directories  because 
 of the GDAL and PROJ folders. 


