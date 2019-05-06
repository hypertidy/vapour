
Hello, 

Resubmission of 0.2.0: 

* current problem tests on CRAN should now pass
* fixed printf() from C++, replaced with Rprintf()
* size of installed directories is expected for Windows tools install of GDAL (afaik)

Thank you


## Test environments

* Linux Ubuntu 18.04 install, R 3.6.0
* ubuntu 14.04 (on travis-ci), R 3.6.0
* win-builder (devel)

## R CMD check results

0 errors | 0 warnings | 1 note

There is a note about the size of installed directories because of the GDAL 
 tools process. 

      
