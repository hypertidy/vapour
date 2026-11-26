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


## GDAL open options: NULL, character() or "" for none, otherwise "NAME=VALUE" strings
.check_open_options <- function(x) {
  if (is.null(x) || length(x) < 1L) return(character())
  if (!is.character(x)) stop("'open_options' must be a character vector of 'NAME=VALUE' strings, or NULL")
  if (anyNA(x)) stop("'open_options' must not contain missing values")
  x <- x[nzchar(x)]
  bad <- !grepl("^[^=]+=", x)
  if (any(bad)) stop(sprintf("'open_options' must be 'NAME=VALUE' strings, not: %s", paste(sQuote(x[bad], FALSE), collapse = ", ")))
  x
}

## vapour opens every source itself, so "-oo" given among the options of a GDAL
## utility would never reach the open. Any "-oo", "NAME=VALUE" pairs found in
## 'options' are moved to the open options.
.split_open_options <- function(options, open_options = NULL) {
  open_options <- .check_open_options(open_options)
  if (is.null(options)) options <- character()
  idx <- which(options == "-oo")
  idx <- idx[idx < length(options)]
  if (length(idx) > 0L) {
    open_options <- c(open_options, .check_open_options(options[idx + 1L]))
    options <- options[-c(idx, idx + 1L)]
  }
  list(options = options, open_options = open_options)
}

## Evaluate 'expr' and take the warnings that carry GDAL messages out of the
## condition stream, so the caller can fold them into an error or signal them
## again. Returns the value and the messages.
.with_gdal_said <- function(expr) {
  said <- character()
  value <- withCallingHandlers(expr, warning = function(w) {
    msg <- conditionMessage(w)
    if (grepl("^GDAL (error|warning) ", msg)) {
      said <<- c(said, msg)
      invokeRestart("muffleWarning")
    }
  })
  list(value = value, said = said)
}
## ": <what GDAL said>" for the end of an error message, or ""
.gdal_reason <- function(said) {
  if (length(said) < 1L) return("")
  paste0(": ", paste(unique(sub("^GDAL (error|warning) [0-9]+: ", "", said)), collapse = "; "))
}
