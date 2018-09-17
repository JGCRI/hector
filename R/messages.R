#### Code for interacting with the hector messaging system


default_fetchvars <- c(ATMOSPHERIC_CO2, RF_TOTAL, RF_CO2, GLOBAL_TEMP)

#' Fetch results from a running Hector core
#'
#' This function will fetch results and return them as a data frame.  The core
#' must be active in order for this to work; attempting to fetch results from a
#' core that has been shut down will cause an error.
#'
#' @param core Hector core object
#' @param dates Vector of dates to fetch.  This will be automatically trimmed to
#' dates that are between the start date and the latest date currently run.
#' @param vars List (or vector) of capability strings defining the variables to
#' be fetched in the result.  By default, the list in the
#' \code{hector.default.fetchvars} option is used.
#' @export
fetchvars <- function(core, dates, vars=NULL)
{
    if(is.null(vars)) {
        vars <- getOption('hector.default.fetchvars',
                          default=sapply(default_fetchvars, function(f){f()}))
    }

    strt <- startdate(core)
    end <- getdate(core)

    valid <- dates >= strt & dates <= end
    dates <- dates[valid]

    do.call(rbind,
            lapply(vars, function(v) {
                       sendmessage(core, GETDATA(), v, dates, NA, '')
                   }))
}
