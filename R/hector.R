#' @useDynLib hector
#' @importFrom Rcpp sourceCpp
NULL

#' Run a single scenario
#'
#' Run the scenario defined by the input file and return a data frame containing
#' results for the default variable list.  The default variable list can be
#' changed by setting the \code{hector.default.fetchvars} option, as described
#' in \code{\link{fetchvars}}.
#'
#' @param infile INI-format file containing the scenario definition
#' @return Data frame containing Hector output for default variables
#' @export
runscenario <- function(infile)
{
    core <- newcore(infile)
    run(core)
    d <- fetchvars(core, seq(startdate(core), enddate(core)))
    shutdown(core)
    d
}

#### Utility functions
### The makeup of an hcore object is
###   hcore[[1]] : index
###   hcore[[2]] : start date
###   hcore[[3]] : end date
###   hcore[[4]] : config file name
###   hcore[[5]] : logical: is active?

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
format.hcore <- function(x, ...)
{
    if(!isactive(x)) {
        'Hector core (INACTIVE)'
    }
    else {
        cdate <- getdate(x)
        sprintf('Hector core\nStart date:\t%d\nEnd date:\t%d\nCurrent date:\t%d\nInput file:\t%s',
                as.integer(x[[2]]), as.integer(x[[3]]), as.integer(cdate),
                x[[4]])
    }
}

#' @rdname hectorutil
#' @export
print.hcore <- function(x, ...)
{
    cat(format(x, ...))
}

