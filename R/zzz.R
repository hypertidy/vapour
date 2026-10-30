## 2020-05-08 GDAL up/down stuff cribbed from sf
## TODO: see mdsumner/dirigible, we'll have a gdalheaders package
.vapour_cache <- new.env(FALSE, parent=globalenv())


.onLoad = function(libname, pkgname) {
  vapour_load_gdal()
}

.onUnload = function(libname, pkgname) {
  vapour_unload_gdal()
}

.onAttach = function(libname, pkgname) {
  ## provided by R. Bivand #183
  rver <- version_gdal_cpp()
  if (strsplit(strsplit(rver, ",")[[1]][1], " ")[[1]][2] == "3.6.0") {
    warning("{vapour} package: GDAL 3.6.0 is in use but has been officially retracted; check here for whether its use might affect your work:\nhttps://github.com/OSGeo/gdal/blob/v3.6.1/NEWS.md\n Need help? Contact the maintainer of {vapour}.")
  }
  msg <- get0(".vapour.proj_data_message", envir = .vapour_cache, ifnotfound = NULL)
  if (!is.null(msg)) packageStartupMessage(msg)
}

vapour_getenv_sql_dialect <- function() {
  Sys.getenv("vapour.sql.dialect")
}

vapour_load_gdal <- function() {
  ## bundled data directories exist only on
  ## - windows because src/Makevars.ucrt copies them from Rtools
  ## - macos because CRAN mac binary libs, and configure --with-data-copy=yes --with-proj-data=/usr/local/share/proj
  ## - linux only if someone opted in with --configure-args='--with-data-copy=yes'
  sql <- Sys.getenv("vapour.sql.dialect")
  if (is.null(sql)) Sys.setenv(vapour_sql_dialect = "")

  ## register drivers first: the PROJ checks below need the SQLite
  ## driver (to read proj.db metadata) and OSR
  out <- register_gdal_cpp()

  ## GDAL data is deliberately left alone: the bundled copy is not
  ## activated. Pointing GDAL_DATA at it broke OSM driver lookup of
  ## osmconf.ini on Windows (win-builder and GitHub CI, 0.20.0 development),
  ## whereas GDAL's own default resolves correctly.

  ## PROJ data, only if the files are in the package, and only if the
  ## bundled proj.db is actually usable by the libproj we are linked to
  prj <- system.file("proj", package = "vapour")
  if (nzchar(prj) && file.exists(file.path(prj, "proj.db"))) {
    vapour_use_bundled_proj_data(prj)
  }

  out
}

vapour_use_bundled_proj_data <- function(prj) {
  ## PROJ validates the database layout version lazily, on first database
  ## access, so setting the search path succeeds even when the bundled
  ## proj.db is incompatible with the libproj in use (e.g. a copy taken
  ## from an older system PROJ, then the system PROJ was upgraded). Point
  ## at the bundled copy, force a lookup, and back out if it fails.
  set_proj_search_paths_cpp(prj)
  ok <- probe_proj_db_cpp()
  if (isTRUE(ok)) {
    assign(".vapour.proj_data_active", TRUE, envir = .vapour_cache)
    return(invisible(TRUE))
  }
  ## revert to PROJ/GDAL defaults and leave a note for .onAttach
  set_proj_search_paths_cpp(character())
  assign(".vapour.proj_data_active", FALSE, envir = .vapour_cache)
  md <- proj_db_metadata_cpp(file.path(prj, "proj.db"))
  built_by <- if ("PROJ.VERSION" %in% names(md)) md[["PROJ.VERSION"]] else "unknown"
  layout <- if (all(c("DATABASE.LAYOUT.VERSION.MAJOR", "DATABASE.LAYOUT.VERSION.MINOR") %in% names(md))) {
    paste0(md[["DATABASE.LAYOUT.VERSION.MAJOR"]], ".", md[["DATABASE.LAYOUT.VERSION.MINOR"]])
  } else {
    "unknown"
  }
  msg <- sprintf(paste0(
    "{vapour}: the PROJ data bundled with this package (proj.db built by PROJ %s, ",
    "database layout %s) is not usable with the PROJ library in use (%s); ",
    "ignoring the bundled copy and using the PROJ defaults instead. ",
    "If this was installed with --with-data-copy, reinstall without it."),
    built_by, layout, vapour_proj_version())
  ## .onLoad can't packageStartupMessage(), .onAttach picks this up. A
  ## message rather than a warning: the fallback works, and a load-time
  ## warning would turn a misconfigured install into an R CMD check WARNING.
  ## vapour_proj_data()$message carries it for requireNamespace() users.
  assign(".vapour.proj_data_message", msg, envir = .vapour_cache)
  invisible(FALSE)
}

vapour_unload_gdal <- function() {
  if (isTRUE(get0(".vapour.proj_data_active", envir = .vapour_cache, ifnotfound = FALSE))) {
    set_proj_search_paths_cpp(character())
  }
}

#' Report on the PROJ data directory in use
#'
#' Returns the PROJ search paths that GDAL's PROJ contexts are using,
#' whether the copy of PROJ data bundled with vapour (if any) is active,
#' and the metadata of the proj.db that resolves first on that path.
#'
#' @return a list with elements 'search_paths', 'bundled_active', 'proj_version', 'proj_db',
#' and 'message' (the startup message explaining why a bundled copy was ignored, or NULL)
#' @export
#' @examples
#' vapour_proj_data()
vapour_proj_data <- function() {
  paths <- get_proj_search_paths_cpp()
  db <- NULL
  for (p in paths) {
    f <- file.path(p, "proj.db")
    if (file.exists(f)) {
      db <- c(path = f, proj_db_metadata_cpp(f))
      break
    }
  }
  list(search_paths = paths,
       bundled_active = isTRUE(get0(".vapour.proj_data_active", envir = .vapour_cache, ifnotfound = FALSE)),
       proj_version = vapour_proj_version(),
       proj_db = db,
       message = get0(".vapour.proj_data_message", envir = .vapour_cache, ifnotfound = NULL))
}
