
Hello, 

vapour 0.2.0, there are some problem tests on OSX on CRAN that have 
been disabled, they rely on unnecessary numeric detail. 

Also a problematic declaration (a direct CRAN report) has been fixed, 
see below. 

Thank you


## Test environments

* Linux Ubuntu 18.04 install, R 3.6.0
* ubuntu 14.04 (on travis-ci), R 3.6.0
* win-builder (devel)

## R CMD check results

0 errors | 0 warnings | 1 note

There is a note about the size of installed directories because of the GDAL 
 tools process:


> * checking installed package size ... NOTE
>   installed size is 79.0Mb
>  sub-directories of 1Mb or more:
>     gdal   3.9Mb
>     libs  68.8Mb
>     proj   5.0Mb
    

## CRAN direct report 2018-08-15

> We see (gcc 8)

> sds_io.cpp: In function ‘Rcpp::CharacterVector sds_info_cpp(const char*)’:
> sds_io.cpp:48:28: warning: comparison of integer expressions of 
> different signedness: ‘size_t’ {aka ‘long unsigned int’} and ‘int’ 
> [-Wsign-compare]
>       for (size_t ii = 0; ii < dscount; ii++) {
  
ii has been set to int. 


      
