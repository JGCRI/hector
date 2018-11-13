# General support functions for Hector's outputplotter routines

library(readr)  # currently 0.1.0

SEPARATOR <- "-------------------------"

# -----------------------------------------------------------------------------
# Time-stamped output function
printlog <- function(msg="", ..., ts=TRUE, cr=TRUE) {
  if(ts) cat(date(), " ")
  cat(msg, ...)
  if(cr) cat("\n")
} # printlog

printlog_warn <- function( ... ) { printlog( "*** WARNING ***", ... ) }

# -----------------------------------------------------------------------------
# Print dimensions of data frame
print_dims <- function(d, dname=deparse(substitute(d))) {
  stopifnot(is.data.frame(d))
  printlog(dname, "rows =", nrow(d), "cols =", ncol(d))
} # print_dims

# -----------------------------------------------------------------------------
# Return matrix of memory consumption
object_sizes <- function() {
  rev(sort(sapply(ls(envir=.GlobalEnv), function(object.name) 
    object.size(get(object.name)))))
} # object_sizes

# -----------------------------------------------------------------------------
# Open a (possibly compressed) csv file and return data
# Note this masks readr::read_csv, which we're using
read_csv <- function(fn, datadir=".", ...) {
  if(is.null(datadir)) {  # NULL signifies absolute path
    fqfn <- fn 
  } else {
    fqfn <- file.path(datadir, fn)      
  }
  printlog("Opening", fqfn)
  if(grepl(".gz$", fqfn)) {
    fqfn <- gzfile(fqfn)
  } else if(grepl(".zip$", fqfn)) {
    fqfn <- unz(fqfn)
  }
  invisible(readr::read_csv(fqfn, ...))
} # read_csv


