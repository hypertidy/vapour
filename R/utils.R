.check_dsn_single <- function(x) {
  mess <- "%s\n 'x' must be a valid character vector (of length 1) for a GDAL-accessible source (file, '/vsi*/' url, database string, etc)"
  if (is.null(x)) stop(sprintf(mess, "NULL: "))
  if (length(x) < 1L) stop(sprintf(mess, "Missing (empty): "))
  if (is.na(x[1L])) stop(sprintf(mess, "Missing (NA): "))
  if (length(x) > 1L) stop(sprintf(mess, "Longer than 1 element:"))
  if (!is.character(x)) stop(sprintf(mess, "Not a character string:"))
  if (nchar(x) < 1) stop(sprintf(mess, "Not a valid character string:"))
  if (file.exists(x)) {
    ## we only want to normalize out the "~"
    if (grepl("^~", x)) {
       x <- normalizePath(x)
    }
  }
  x
}
.check_dsn_multiple <- function(x) {
  mess <- "%s\n 'x' must be a valid character vector (NAs not allowed) for a GDAL-accessible source (file, '/vsi*/' url, database string, etc)"
  if (is.null(x)) stop(sprintf(mess, "NULL: "))
  if (length(x) < 1L) stop(sprintf(mess, "Missing (empty): "))
  if (anyNA(x[1L])) stop(sprintf(mess, "Missing (NA): "))
  ## if (length(x) > 1L) stop(sprintf(mess, "Longer than 1 element:"))
  if (!is.character(x)) stop(sprintf(mess, "Not a character vector:"))
  if (any(nchar(x) < 1)) stop(sprintf(mess, "Not a valid character vector:"))
  tildes <- unlist(lapply(x, function(.x) grepl("~", .x)))
  normthem <- tildes & file.exists(x)
  x <- ifelse(normthem, normalizePath(x), x)
  x
}


.check_dsn_multiple_naok <- function(x) {
  mess <- "%s\n 'x' must be a valid character vector (NAs are allowed) for a GDAL-accessible source (file, '/vsi*/' url, database string, etc)"
  if (is.null(x)) stop(sprintf(mess, "NULL: "))
  if (length(x) < 1L) stop(sprintf(mess, "Missing (empty): "))
  ## if (anyNA(x[1L])) stop(sprintf(mess, "Missing (NA): "))
  ## if (length(x) > 1L) stop(sprintf(mess, "Longer than 1 element:"))
  if (!is.character(x)) stop(sprintf(mess, "Not a character vector:"))
  if (any(nchar(x) < 1)) stop(sprintf(mess, "Not a valid character vector:"))
  
  x
}
