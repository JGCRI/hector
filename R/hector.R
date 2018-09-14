#' @useDynLib hector
#' @importFrom Rcpp sourceCpp
NULL

#' Run a test of basic hector functionality
#' @keywords internal
runtest <- function()
{
    core <- newcore('inst/input/hector_rcp45.ini')
    run(core)
    shutdown(core)
}


#### Documentation objects
#' Hector logging levels
#'
#' These functions return constants that can be used in
#' \code{\link{newcore}} to set the minimum logging level
#'
#' @name loglevels
NULL

#### Utility functions
### The makeup of an hcore object is
###   hcore[[1]] : index
###   hcore[[2]] : start date
###   hcore[[3]] : end date
###   hcore[[4]] : config file name
###   hcore[[5]] : logical: is active?

#' Utility functions for Hector instances
#'
#' @param core Hector instance to operate on
#' @name hectorutil
NULL

#' \strong{isactive}: Indicate whether a Hector instance is active
#' @rdname hectorutil
#' @export
isactive <- function(core)
{
    ## This is not super robust, since it's always possible that a user will
    ## shut down a core without recording the fact.
    ## TODO: replace this with a function that checks the core registry, which
    ## is always accurate.
    core[[5]]
}

#' \strong{startdate}: Report the start date for a Hector instance
#' @rdname hectorutil
#' @export
startdate <- function(core)
{
    core[[2]]
}

#' \strong{enddate}: Report the end date for a Hector instance
#' @rdname hectorutil
#' @export
enddate <- function(core)
{
    core[[3]]
}



#### Methods for hcore objects

#' @rdname hectorutil
#' @export
format.hcore <- function(core)
{
    if(!isactive(core)) {
        'Hector core (INACTIVE)'
    }
    else {
        cdate <- getdate(core)
        sprintf('Hector core\nStart date:\t%d\nEnd date:\t%d\nCurrent date:\t%d\nInput file:\t%s',
                as.integer(core[[2]]), as.integer(core[[3]]), as.integer(cdate),
                core[[4]])
    }
}

#' @rdname hectorutil
#' @export
print.hcore <- function(core)
{
    cat(format(core))
}

