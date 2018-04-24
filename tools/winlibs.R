if(getRversion() < "3.3.0") {
  stop("Your version of R is too old. This package requires R-3.3.0 or newer on Windows.")
}



# For details see: https://github.com/rwinlib/gdal2
VERSION <- commandArgs(TRUE)
if(!file.exists(sprintf("../windows/gdal2-%s/include/gdal/gdal.h", VERSION))){
  if(getRversion() < "3.3.0") setInternet2()
  download.file(sprintf("https://github.com/rwinlib/gdal2/archive/v%s.zip", VERSION), "lib.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("lib.zip", exdir = "../windows")
  unlink("lib.zip")
}

# Download gdal-2.2.0 from rwinlib
# if(!file.exists("../windows/gdal2-2.2.3/include/gdal/gdal.h")) {
#   download.file("https://github.com/rwinlib/gdal2/archive/v2.2.3.zip", "lib.zip", quiet = TRUE)
#   dir.create("../windows", showWarnings = FALSE)
#   unzip("lib.zip", exdir = "../windows")
#   unlink("lib.zip")
# }
