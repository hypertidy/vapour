library(R6)
GDALDataset <- R6Class("GDALDataset",
                  public = list(
                    ptr = NULL,
                    meta = NULL,
                    initialize = function(ptr = new("externalptr"), meta = list()) {
                      self$ptr <- ptr
                      self$meta <- meta
                      self$greet()
                    
                    },
                    
                    check_ptr = function() {
                      isnull <- function(pointer){
                        a <- attributes(pointer)
                        attributes(pointer) <- NULL
                        out <- identical(pointer, new("externalptr"))
                        attributes(pointer) <- a
                        return(out)
                      }
                      if (isnull(self$ptr)) stop("pointer is stale")
                    },
                    set_ptr = function(val) {
                      self$ptr <- val
                    },
                    greet = function() {
                      self$check_ptr()
                      cat(paste0(c(self$size(), self$geotransform()), collapse = ","), "\n")
                    },
                    size = function() {
                      self$check_ptr()
                      vapour:::gh_GDALGetRasterSize(self$ptr)
                    },
                      geotransform = function() {
                        self$check_ptr()
                        vapour:::gh_GDALGetGeoTransform(self$ptr)
         
                    }
                  )
)

f <- "inst/extdata/volcano.tif"
ann <- GDALDataset$new(vapour:::gh_GDALOpenEx(f), list())


ann$size()
ann$geotransform()
