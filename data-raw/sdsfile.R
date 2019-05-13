populateNC <- function(ncfile, data, dstart = 1, varname = "var", closefile = TRUE) {

  var.put.nc(ncfile, varname, data, c(rep(1, length(dim(data)))), c(dim(data)))


  if(closefile) close.nc(ncfile)
  ncfile
}

mkNC <- function(x, varname = "var", filename = tempfile(), missing = NULL) {
  ## x - named, list of dimension values
  require(RNetCDF)
  ncfile <- create.nc(filename)
  dnames <- names(x)


  for (i in seq_along(x)) {
    dim.def.nc(ncfile, dnames[i], length(x[[dnames[i]]]))
    var.def.nc(ncfile, dnames[i], "NC_DOUBLE", dnames[i])

    ## here we have POSIXct time, but we also have to define the epoch and stuff it in the attributes,
    ## which is easy but extremely boring
    dd <- x[[dnames[i]]]
    if (inherits(dd, "POSIXt")) dd <- as.numeric(dd)
    var.put.nc(ncfile, dnames[i], dd)
  }
  dims <- sapply(x, length)
  for (j in seq_along(varname)) var.def.nc(ncfile, varname[j], "NC_DOUBLE", dnames)
  if(!is.null(missing)) att.put.nc(ncfile, varname, "missing_value", "NC_DOUBLE", missing)
  att.put.nc(nc, "NC_GLOBAL", "title", "NC_CHAR", "Data from Foo")
  att.put.nc(nc, "NC_GLOBAL", "Conventions", "NC_CHAR", "PUBAR1.0")

  ncfile
}

dims <- list(x = 1:nrow(volcano), y = 1:ncol(volcano))
vv <- volcano
afile <- tempfile(fileext = ".nc")
nc <- mkNC(dims, varname = c("vv", "vv2"), filename = afile)
populateNC(nc, vv, varname = "vv", closefile = FALSE)
populateNC(nc, vv*2, varname = "vv2", closefile = TRUE)


RNetCDF::print.nc(RNetCDF::open.nc(afile))
# dimensions:
#   x = 87 ;
# y = 61 ;
# variables:
#   double x(x) ;
# double y(y) ;
# double vv(x, y) ;

file.copy(afile, file.path("inst/extdata/gdal/sds.nc"))
