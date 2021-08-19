## vapour 0.7.9

Fix for CRAN errors, with broken configure. 

* had modified sf/configure.ac to remove GEOS, but had remove key AC_SUBST along with that (GDAL_DEP_LIBS was lost, amongst other things)
* put PROJ_LIBS after PKG_LIBS, as per CRAN advice
* CRAN notes AC_SUBST should only be called once at the end for clarity, but I've only restored what sf does until I can understand more

Please note that Jeroen Ooms already applied a PR for UCRT, so there's a warning currently on  
 r-devel-windows-x86_64-gcc10-UCRT. 

Thanks very much again, appreciate the patience and detailed feedback.  

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



