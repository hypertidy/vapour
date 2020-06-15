## 2020-05-08 GDAL up/down stuff cribbed from sf
## TODO: GDALAllRegister, OGRRegisterAll, OSRRegisterAll
## do here once as neede
.vapour_cache <- new.env(FALSE, parent=globalenv())


.onLoad = function(libname, pkgname) {
  vapour_load_gdal()
}

.onUnload = function(libname, pkgname) {
  vapour_unload_gdal()
}

.onAttach = function(libname, pkgname) {

}

vapour_load_gdal <- function() {
  ## only on windows because tools/winlibs.R
  if (file.exists(system.file("proj/nad.lst", package = "vapour")[1L])) {
    #prj = system.file("proj", package = "sf")[1]
    #if (! CPL_set_data_dir(prj)) { # if TRUE, uses C API to set path, leaving PROJ_LIB alone
    #  assign(".vapour.PROJ_LIB", Sys.getenv("PROJ_LIB"), envir=.vapour_cache)
    #  Sys.setenv("PROJ_LIB" = prj)
    #}
    #CPL_use_proj4_init_rules(1L)
    assign(".vapour.GDAL_DATA", Sys.getenv("GDAL_DATA"), envir=.vapour_cache)
    gdl = system.file("gdal", package = "vapour")[1]
    Sys.setenv("GDAL_DATA" = gdl)
    # nocov end
  }
  #.gdal_init()
  #register_all_s3_methods() # dynamically registers non-imported pkgs (tidyverse)
}
# todo
vapour_unload_gdal <- function() {
  #CPL_gdal_cleanup_all()
  if (file.exists(system.file("proj/nad.lst", package = "vapour")[1L])) {
    #if (! CPL_set_data_dir(system.file("proj", package = "sf")[1])) # set back:
    #  Sys.setenv("PROJ_LIB"=get(".sf.PROJ_LIB", envir=.sf_cache))
    #
    Sys.setenv("GDAL_DATA"=get(".vapour.GDAL_DATA", envir=.vapour_cache))
 }
}
