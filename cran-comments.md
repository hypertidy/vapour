## vapour 0.8.5

Fix for CRAN warnings due to outdated autoconf syntax. 

Important bug fixes for raster scaling in the warper. 

## Test environments

* Linux Ubuntu 20.04 R 4.1.0
* win-builder (devel)
* macos-builder: https://mac.r-project.org/macbuilder/results/1633481011-e9c706e020cb5c7e/
* mac-os CRAN-alike on github actions
* fedora-clang-devel      (~= CRAN with config flags: r-devel-linux-x86_64-fedora-clang )
* fedora-gcc-devel        (~= CRAN with config flags: r-devel-linux-x86_64-fedora-gcc)
* `rhub::check_for_cran()` (multiple platforms). 
* ubuntu 18.04    GDAL: 2.2.2, PROJ: 4.8.0

## R CMD check results

0 errors | 0 warnings | 1 note

There is a note about the size of installed directories  because 
 of the GDAL and PROJ folders. 


## Reverse dependencies

{lazyraster} passes check with this version



