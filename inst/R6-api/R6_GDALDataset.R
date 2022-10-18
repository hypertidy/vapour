library(R6)
GDALDataset <- R6Class("GDALDataset",
                  public = list(
                    ptr = NULL,
                    dsn = NULL, 
                    meta = NULL,
                    initialize = function(dsn = character(), meta = list()) {
                      ## a bit weird but easy for now
                      if (inherits(dsn, "externalptr")) {
                        self$ptr <- dsn
                        self$dsn <- vapour:::gh_GDALGetDescription(self$ptr)
                      } else {
                        self$ptr <- vapour:::gh_GDALOpenEx(dsn)
                        self$dsn <- dsn  ## our description might get lost (?)
                      }
                      
                      self$meta <- meta
                      self$info()
                    },
                    check_ptr = function() {
                      isnull <- function(pointer){
                        a <- attributes(pointer)
                        attributes(pointer) <- NULL
                        out <- identical(pointer, new("externalptr"))
                        attributes(pointer) <- a
                        return(out)
                      }
                      
                      ## if we check and the pointer is nil, open it up again
                      if (isnull(self$ptr)) {
                        if (self$dsn == "") {
                          stop("cannot refresh GDALDataset, no DSN/description/file-list")
                        }
                        message(sprintf("refreshing external pointer from 'dsn': %s ", 
                                        self$dsn))
                        self$ptr <- vapour:::gh_GDALOpenEx(self$dsn)
                      }
                        
                    },
                    Read = function(dimension= NULL, extent = NULL, resample = "nearestneighbour") {
                      if (!is.null(extent)) {
                        message("extent argument not yet supported")
                      }
                      if (is.null(dimension)) {
                        ## FIXME: modify to aspect ratio of source
                        dimension <- c(1024, 1024)
                      }
                      srcdim <- self$dimension()
                      window <- c(0L, 0L, 
                                  as.integer(c(srcdim[1L], srcdim[2L], dimension[1L], dimension[2L])))
                      matrix(vapour:::gh_GDALRasterio(self$ptr, window, resample), dimension[2L], byrow = TRUE)
                                            
                    },
                     set_ptr = function(val) {
                      self$ptr <- val
                    },
                    info = function() {
                      self$check_ptr()
                      lst <- self$dsnlist()
                      if (length(lst) < 1 || nchar(lst) < 1) {
                        lst <- "<placeholder: try `$description` for input DSN string>"
                      } else {
                        lst <- str(lst,  nchar.max = 378)
                      }
                      cat(sprintf("DSN       : %s\n", lst))
                      dimension <- self$dimension()
                      cat(sprintf("dimension : %i, %i (ncol, nrow)\n", 
                                  dimension[1L], dimension[2L]))
                      ex <- self$extent()
                      cat(sprintf("extent    : %.1f,%.1f,%.1f,%.1f (xmin, xmax, ymin, ymax) \n", ex[1L], ex[2L], ex[3L], ex[4L]))
                      
                    },
                    dimension = function() {
                      self$check_ptr()
                      vapour:::gh_GDALGetRasterSize(self$ptr)
                    },
                      geotransform = function() {
                        self$check_ptr()
                        vapour:::gh_GDALGetGeoTransform(self$ptr)
         
                      },
                    extent = function() {
                      x <- self$geotransform()
                      dim <- self$dimension()
                      xx <- c(x[1], x[1] + dim[1] * x[2])
                      yy <- c(x[4] + dim[2] * x[6], x[4])
                      c(xx, yy)
                    },
                    dsnlist = function() {
                      vapour:::gh_GDALGetFileList(self$ptr)
                    },
                    filelist = function() {
                      self$dsnlist()
                    },
                    description = function() {
                      desc <- vapour:::gh_GDALGetDescription(self$ptr)
                      if (nchar(desc) > 0) desc else self$dsn
                    }
                  )
)
f <- raadfiles::oisst_daily_files()$fullname[1]

sds <- vapour_vrt(f, sds = "sst", projection = "OGC:CRS84")

#f <- "inst/extdata/volcano.tif"
ann <- GDALDataset$new(sds, list())

dsn <- "NETCDF:/rdsi/PUBLIC/raad/data/www.ncei.noaa.gov/data/sea-surface-temperature-optimum-interpolation/v2.1/access/avhrr/198109/oisst-avhrr-v02r01.19810901.nc:sst"
ann$dimension()
ann$geotransform()
library(vapour)

elev <- "/vsicurl/https://github.com/rspatial/terra/raw/master/inst/ex/elev.tif"
d <- GDALDataset$new(elev)


ximage::ximage(d$Read(d$dimension() * 6, resample = "lanczos"), extent = d$extent(), zlim = c(0, 550))
