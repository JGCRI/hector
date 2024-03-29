% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/messages.R
\name{fetchvars}
\alias{fetchvars}
\title{Fetch results from a running Hector core}
\usage{
fetchvars(core, dates, vars = NULL, scenario = NULL)
}
\arguments{
\item{core}{Hector core object}

\item{dates}{Vector of dates to fetch; this will be automatically trimmed to
dates that are between the start date and the latest date currently run.
Set to \code{NA} to return data with no associated dates such as
model \link{parameters}.}

\item{vars}{List (or vector) of capability strings defining the variables to
be fetched in the result.}

\item{scenario}{Optional scenario name.  If not specified, the name element
of the Hector core object will be used.}
}
\description{
This function will fetch results and return them as a data frame.  The core
must be active in order for this to work; attempting to fetch results from a
core that has been shut down will cause an error.
}
\details{
The variables to fetch should be given as a sequence of character strings
corresponding to the capabilities declared by hector components.  These
strings should be generated using the capability identifier functions (see
below for a list of documentation pages for these).

The list of variables to fetch if you don't specify \code{vars} is stored in
the \code{hector.default.fetchvars} option.  If this option is also unset, then
the default variable list is CO2 concentration, total radiative forcing, CO2
forcing, and global mean temperature. To see a list of the potential \code{vars}
see \code{data(inputstable)} and \code{data(fxntable)}.
}
\examples{
\dontrun{
ini <- system.file(package = "hector", "input/hector_ssp245.ini")
hc <- newcore(ini)
run(hc)
out <- fetchvars(core = hc, dates = 1900:2100, vars = c(GLOBAL_TAS(), NPP()))
print(out)
}
}
\seealso{
\link{concentrations}, \link{emissions}, \link{forcings},
\link{carboncycle}, \link{haloemiss}, \link{haloforcings}, \link{methane},
\link{ocean}, \link{so2}, \link{temperature}, \link{parameters}

Other main user interface functions: 
\code{\link{get_tracking_data}()},
\code{\link{newcore}()},
\code{\link{reset}()},
\code{\link{run}()},
\code{\link{setvar}()},
\code{\link{shutdown}()}
}
\concept{main user interface functions}
