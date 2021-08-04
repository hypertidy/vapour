## vapour 0.7.7

Fix for CRAN errors, with required version of minimum version of GDAL now 2.3.0. 

There was a change in 0.7.6 to use `OGRGeometry::importFromWkt()` from GDAL, with syntax
requirement for GDAL 2.3.0 as a minimum. This triggered the error on Solaris. 

There was unused code in src/vapour.cpp (GetPointsInternal, MAX_INT) that triggered warnings on fedora devel clang about 
unused variable and function. 

I understand better now ... I have checked with rhub on 

* fedora-clang-devel      (~= CRAN with config flags: r-devel-linux-x86_64-fedora-clang )
* fedora-gcc-devel        (~= CRAN with config flags: r-devel-linux-x86_64-fedora-gcc)
* `rhub::check_for_cran()` (multiple platforms). 

Please note that Jeroen Ooms already applied a PR for UCRT, so there's a warning currently on  r-devel-windows-x86_64-gcc10-UCRT. 

I appreciate your patience, and pointing out problems - thank you!

## Test environments

* Linux Ubuntu 20.04 R 4.1.0
* win-builder (devel)


## R CMD check results

0 errors | 1 warning | 1 note

There is a note about the size of installed directories on Windows and MacOS because 
 of the GDAL and PROJ folders. 

There is a warning on Windows: 

r-devel-windows-x86_64-gcc10-UCRT:  WARNING: failed to apply patch patches/CRAN/vapour.diff

## Reverse dependencies

{lazyraster} passes check with this version



