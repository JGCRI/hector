#### Code for interacting with the hector messaging system


default_fetchvars <- c(ATMOSPHERIC_CO2, RF_TOTAL, RF_CO2, GLOBAL_TEMP)

#' Fetch results from a running Hector core
#'
#' This function will fetch results and return them as a data frame.  The core
#' must be active in order for this to work; attempting to fetch results from a
#' core that has been shut down will cause an error.
#'
#' The variables to fetch should be given as a sequence of character strings
#' corresponding to the capabilities declared by hector components.  These
#' strings should be generated using the capability identifier functions (see
#' below for a list of documentation pages for these).
#'
#' The list of variables to fetch if you don't specify \code{vars} is stored in
#' the \code{hector.default.fetchvars} option.  If this option is also unset, then
#' the default variable list is CO2 concentration, total radiative forcing, CO2
#' forcing, and global mean temperature.
#'
#' @seealso \link{concentrations}, \link{emissions}, \link{forcings},
#' \link{carboncycle}, \link{haloemiss}, \link{haloforcings}, \link{methane},
#' \link{ocean}, \link{so2}, \link{temperature}, \link{parameters}
#'
#' @param core Hector core object
#' @param dates Vector of dates to fetch.  This will be automatically trimmed to
#' dates that are between the start date and the latest date currently run.
#' @param vars List (or vector) of capability strings defining the variables to
#' be fetched in the result.
#' @param scenario Optional scenario name.  If not specified, the name element
#' of the Hector core object will be used.
#' @family main user interface functions
#' @export
fetchvars <- function(core, dates, vars=NULL, scenario=NULL)
{
    if(is.null(vars)) {
        vars <- getOption('hector.default.fetchvars',
                          default=sapply(default_fetchvars, function(f){f()}))
    }

    if(is.null(scenario)) {
        scenario <- core$name
    }

    strt <- startdate(core)
    end <- getdate(core)

    valid <- dates >= strt & dates <= end
    dates <- dates[valid]

    rslt <- do.call(rbind,
                    lapply(vars, function(v) {
                               sendmessage(core, GETDATA(), v, dates, NA, '')
                           }))
    ## Fix the variable name for the adjusted halocarbon forcings so that they are
    ## consistent with other forcings.
    rslt$variable <- sub(paste0('^',RFADJ_PREFIX()), RF_PREFIX(), rslt$variable)
    cols <- names(rslt)
    rslt$scenario <- scenario
    ## reorder the columns to put the scenario name first
    rslt[,c('scenario', cols)]
}


#' Set values for a Hector variable
#'
#' This function selects a variable by its capability name and sets the
#' requested values at the requested dates.  The units must also be supplied as
#' a single string (heterogeneous units are not supported).  These are checked
#' against the expected unit in the code, and an error is signaled if they don't
#' match (i.e., there is no attempt to convert units).
#'
#' @param core Hector core object
#' @param dates Vector of dates
#' @param var Capability string for the variable to set
#' @param values Values to set.  Must be either a single value or a vector the
#' same length as dates.
#' @param unit Unit string.  Can be set to NA for unitless variables.
#' @family main user interface functions
#' @export
setvar <- function(core, dates, var, values, unit)
{
    unit[is.na(unit)] <- '(unitless)'
    var_split <- strsplit(var, BIOME_SPLIT_CHAR(), fixed = TRUE)[[1]]
    if (length(var_split) > 2) {
        stop("Invalid input variable: '", var, "'")
    } else if (length(var_split) == 2) {
        biome <- var_split[[1]]
        biome_list <- get_biome_list(core)
        if (!biome %in% biome_list) {
            stop("Biome '", biome, "' missing from biome list. ",
                 "Use `hector::create_biome(\"", biome, "\")` to create it.")
        }
    }
    sendmessage(core, SETDATA(), var, dates, values, unit)

    if(any(dates <= getdate(core)) || any(is.na(dates))) {
        rdate <- min(dates) -1
        if(is.na(rdate))
            rdate <- 0

        if(core$clean)
            core$reset_date <- rdate
        else
            core$reset_date <- min(rdate, core$reset_date)

        core$clean <- FALSE
    }

    invisible(NULL)
}
