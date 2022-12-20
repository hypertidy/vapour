# vapour_reproj_xy <- function(x, target, ..., source = NULL) {
#   
#   x <- as.matrix(x)
#   
#   if (is.null(source)) {
#     source <- "OGC:CRS84"
#     warning("assuming input is longlat, set 'source' explicitly as needed")
#   }
#   if (!ncol(x) == 2L) stop("2 columns for vapour_reproj_xy")
#   if (!is.numeric(x)) stop("2 column matrix must be numeric for vapour_reproj_xy")
#   do.call(cbind, proj_reproj_xy(x[,1], x[,2], target = target, source = source))
# }
# 
# 
# vapour_reproj_list <- function(x, target, ..., source = NULL) {
#   
#   x <- as.matrix(x)
#   
#   if (is.null(source)) {
#     source <- "OGC:CRS84"
#     warning("assuming input is longlat, set 'source' explicitly as needed")
#   }
#   if (!ncol(x) == 2L) stop("2 columns for vapour_reproj_xy")
#   if (!is.numeric(x)) stop("2 column matrix must be numeric for vapour_reproj_xy")
#   do.call(cbind, proj_reproj_xy(x[,1], x[,2], target = target, source = source))
# }
# 
