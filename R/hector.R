#' @useDynLib hector
#' @importFrom Rcpp sourceCpp
NULL

#' @details
#' This package allows you to run the Hector Simple Climate Model (SCM) from R
#' and receive the results back as R data frames.
#'
#' @section Getting started:
#' To run a scenario, you will need a scenario input file.  Several examples are
#' included in the \code{input} directory, which you can find with the
#' expression \code{system.file('input', package='hector')}.  The input files
#' have names like  \code{'hector_sspNNN.ini'}, where \code{NNN} is one of
#' \code{119}, \code{126}, \code{245}, \code{370}, \code{434}, \code{460}, or \code{ssp585}.
#'
#' There is a quickstart wrapper called \code{\link{runscenario}}, which takes
#' an input file as an argument, runs the scenario, and returns the results in a
#' data frame.  Which variables you get out are controlled by an option setting.
#' Setting this option is explained in the manual page for
#' \code{\link{fetchvars}}.
#'
#' @section Advanced usage:
#' You can get a lot more control over your hector run if you create a hector
#' instance (sometimes called a "core" elsewhere in this documentation) using
#' the \code{\link{newcore}} function.  Once you have that you can run Hector up
#' to specific dates, fetch results, set parameters or input data, rewind to an
#' earlier date so you can rerun (e.g., with changed emissions or parameters).
#' When you are finished with a hector instance, you should call
#' \code{\link{shutdown}} on it to release the resources it has allocated.
#'
#' @seealso \code{\link{runscenario}}, \code{\link{newcore}},
#' \code{\link{shutdown}}, \code{\link{fetchvars}}
#'
#' @references
#' Hartin, C. A., Patel, P., Schwarber, A., Link, R. P., and Bond-Lamberty,
#' B. P.: A simple object-oriented and open-source model for scientific and
#' policy analyses of the global climate system - Hector v1.0, Geosci. Model
#' Dev., 8, 939-955, \url{https://doi.org/10.5194/gmd-8-939-2015}, 2015.
#'
#' Hartin, C. A., Bond-Lamberty, B., Patel, P., and Mundra, A.: Ocean
#' acidification over the next three centuries using a simple global climate
#' carbon-cycle model: projections and sensitivities, Biogeosciences, 13,
#' 4329-4342, \url{https://doi.org/10.5194/bg-13-4329-2016}, 2016.
#' @keywords internal
"_PACKAGE"

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
runscenario <- function(infile) {
    core <- newcore(infile)
    run(core)
    d <- fetchvars(core, seq(startdate(core), enddate(core)))
    shutdown(core)
    d
}


#### Hector core constructor
#' Create and initialize a new hector instance
#'
#' The object returned is a handle to the newly created instance.  It will be required as an
#' argument for all functions that operate on the instance.  Creating multiple instances
#' simultaneously is supported.
#'
#' @include aadoc.R
#' @param inifile (String) name of the hector input file.
#' @param loglevel (int) minimum message level to output in logs (see \code{\link{loglevels}}).
#' @param suppresslogging (bool) If true, suppress all logging (loglevel is ignored in this case).
#' @param name (string) An optional name to identify the core.
#' @return handle for the Hector instance.
#' @family main user interface functions
#' @export
newcore <- function(inifile, loglevel = 0, suppresslogging = TRUE,
                    name = "Unnamed Hector core") {
    hcore <- newcore_impl(normalizePath(inifile), loglevel, suppresslogging, name)
    class(hcore) <- c("hcore", class(hcore))
    reg.finalizer(hcore, hector::shutdown)
    hcore
}

#' Retrieve the tracking data for a Hector instance
#'
#' @param core Handle to the Hector instance.
#' @importFrom utils read.csv
#' @return A \code{\link{data.frame}} with the tracking data. Columns include
#' \code{year} (integer), \code{component} (character), \code{pool_name} (character),
#' \code{pool_value} (double), \code{pool_units} (character),
#' \code{source_name} (character), and \code{source_fraction} (double). The
#' fractions will always sum to 1 for a given pool and year.
#' @note The \code{pool_name}, \code{pool_value}, and \code{pool_units} names
#' differ from those used in the model's standard output stream (\code{variable},
#' \code{value}, and \code{units} respectively).
#' @family main user interface functions
#' @export
get_tracking_data <- function(core) {
    td <- get_tracking_data_impl(core)
    if (td != "") {
        read.csv(textConnection(td), stringsAsFactors = FALSE)
    } else {
        data.frame()  # throw error instead?
    }
}

#### Utility functions
### The elements of an hcore object are
###   coreidx : index
###   strtdate : start date
###   enddate : end date
###   inifile : config file name

#' \strong{isactive}: Indicate whether a Hector instance is active
#' @rdname hectorutil
#' @export
isactive <- function(core) {
    if (!inherits(core, "hcore")) {
        stop("Object supplied is not an hcore class instance.")
    }
    chk_core_valid(core)
}

#' \strong{startdate}: Report the start date for a Hector instance
#' @rdname hectorutil
#' @export
startdate <- function(core) {
    if (!inherits(core, "hcore")) {
        stop("Object supplied is not an hcore class instance.")
    }
    core$strtdate
}

#' \strong{enddate}: Report the end date for a Hector instance
#' @rdname hectorutil
#' @export
enddate <- function(core) {
    if (!inherits(core, "hcore")) {
        stop("Object supplied is not an hcore class instance.")
    }
    core$enddate
}

#' \strong{getname}: Report the name for a Hector instance
#' @rdname hectorutil
#' @export
getname <- function(core) {
    core$name
}


#### Methods for hcore objects

#' Hector instance object methods
#'
#' @param x Hector core object
#' @param ... Additional arguments
#' @name methods
NULL

#' @describeIn methods Format method
#' @export
format.hcore <- function(x, ...) {
    if (!isactive(x)) {
        "Hector core (INACTIVE)"
    } else {
        cdate <- getdate(x)
        sprintf(
            paste0("Hector core:\t%s\nStart date:\t%d\nEnd date:\t%d\n",
                   "Current date:\t%d\nInput file:\t%s\n"),
            x$name,
            as.integer(x$strtdate), as.integer(x$enddate), as.integer(cdate),
            x$inifile
        )
    }
}

#' @describeIn methods Print method
#' @export
print.hcore <- function(x, ...) {
    cat(format(x, ...))
}
