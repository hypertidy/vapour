## vapour <next>

Bug fix, warper wasn't providing correct scaling of values. 

(There is an error on CRAN but due to an Rcpp problem affecting many packages also. )

## Test environments

* Linux Ubuntu 20.04 R 4.1.0
* win-builder (devel)
* mac-os CRAN-alike on github actions
* fedora-clang-devel      (~= CRAN with config flags: r-devel-linux-x86_64-fedora-clang )
* fedora-gcc-devel        (~= CRAN with config flags: r-devel-linux-x86_64-fedora-gcc)
* `rhub::check_for_cran()` (multiple platforms). 
* ubuntu 18.04    GDAL: 2.2.3, PROJ: 4.9.3


## R CMD check results

0 errors | 1 warning | 1 note

There is a note about the size of installed directories on Windows and MacOS because 
 of the GDAL and PROJ folders. 

There is a warning on Windows: 

r-devel-windows-x86_64-gcc10-UCRT:  WARNING: failed to apply patch patches/CRAN/vapour.diff

## Reverse dependencies

{lazyraster} passes check with this version



