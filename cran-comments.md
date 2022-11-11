## vapour 0.9.1

New version to address Additional Issues on CRAN. I've tested for lossy casts on clang with -Wconversion. 

There are warnings from clang about 

"enumeration values 'OFTWideString' and 'OFTWideStringList' not handled in switch [-Wswitch]
    switch (poFieldDefn->GetType()) {

but these are deprecated values in GDAL. 

Other clang warnings are from Rcpp itself when run with -Wconversion. 

Thank you! 

## Test environments

* win-builder (devel and release)
* Linux ubuntu 4.2.1

## R CMD check results

0 errors | 0 warnings | 1 note

There is a note about the size of installed directories  because 
 of the GDAL and PROJ folders. 


