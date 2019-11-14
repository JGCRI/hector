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


#' Cumulate exogenous Hector input variables
#'
#' This function cumulates all available exogenous input variables for the active
#' Hector core, such as emissions and pre-industrial species concentrations.
#'
#' @param core Hector core object
#' @param dates Vector of dates to fetch
#' @return rslt_tot Dataframe of variables
#' @family main user interface functions
#' @export
cum_vars_input <- function(core, dates) {
    vars_em <- c(
        hector::EMISSIONS_BC(),
        # hector::EMISSIONS_C2F6(),             # Caller is requesting unknown variable
        # hector::EMISSIONS_CCL4(),             # Caller is requesting unknown variable
        # hector::EMISSIONS_CF4(),              # Caller is requesting unknown variable
        # hector::EMISSIONS_CFC11(),            # Caller is requesting unknown variable
        # hector::EMISSIONS_CFC113(),           # Caller is requesting unknown variable
        # hector::EMISSIONS_CFC114(),           # Caller is requesting unknown variable
        # hector::EMISSIONS_CFC115(),           # Caller is requesting unknown variable
        # hector::EMISSIONS_CFC12(),            # Caller is requesting unknown variable
        # hector::EMISSIONS_CH3BR(),            # Caller is requesting unknown variable
        # hector::EMISSIONS_CH3CCL3(),          # Caller is requesting unknown variable
        # hector::EMISSIONS_CH3CL(),            # Caller is requesting unknown variable
        hector::EMISSIONS_CH4(),
        # hector::EMISSIONS_CO(),               # Caller is requesting unknown variable
        # hector::EMISSIONS_HALON1211(),        # Caller is requesting unknown variable
        # hector::EMISSIONS_HALON1301(),        # Caller is requesting unknown variable
        # hector::EMISSIONS_HALON2402(),        # Caller is requesting unknown variable
        # hector::EMISSIONS_HCF141B(),          # Caller is requesting unknown variable
        # hector::EMISSIONS_HCF142B(),          # Caller is requesting unknown variable
        # hector::EMISSIONS_HCF22(),            # Caller is requesting unknown variable
        # hector::EMISSIONS_HFC125(),           # Caller is requesting unknown variable
        # hector::EMISSIONS_HFC134A(),          # Caller is requesting unknown variable
        # hector::EMISSIONS_HFC143A(),          # Caller is requesting unknown variable
        # hector::EMISSIONS_HFC227EA(),         # Caller is requesting unknown variable
        # hector::EMISSIONS_HFC23(),            # Caller is requesting unknown variable
        # hector::EMISSIONS_HFC245FA(),         # Caller is requesting unknown variable
        # hector::EMISSIONS_HFC32(),            # Caller is requesting unknown variable
        # hector::EMISSIONS_HFC4310(),          # Caller is requesting unknown variable
        # hector::EMISSIONS_N2O(),              # Caller is requesting unknown variable
        # hector::EMISSIONS_NMVOC(),            # Caller is requesting unknown variable
        # hector::EMISSIONS_NOX(),              # Caller is requesting unknown variable
        hector::EMISSIONS_OC(),
        # hector::EMISSIONS_SF6(),              # Caller is requesting unknown variable
        hector::EMISSIONS_SO2()
    )

    vars_conc <- c(
        hector::PREINDUSTRIAL_CH4(),          # Assertion failed: Date not allowed
        hector::PREINDUSTRIAL_CO2(),          # Assertion failed: Date not allowed
        hector::PREINDUSTRIAL_N2O()          # Assertion failed: Date not allowed
        # hector::PREINDUSTRIAL_O3()            # Assertion failed: Date not allowed
    )

    ### Call sendmessage() & pass dates param
    rslt_em <- do.call(rbind,
                       lapply(vars_em, function(v) {
                           sendmessage(core, GETDATA(), v, dates, NA, '')
                       }))

    ### Call sendmessage() without passing dates param, since doing so raises an error
    rslt_conc <- do.call(rbind,
                         lapply(vars_conc, function(v) {
                             sendmessage(core, GETDATA(), v, NA, NA, '')
                         }))

    rslt_tot <- rbind(rslt_em, rslt_conc)

    return(rslt_tot)
}


#' Cumulate Hector parameters
#'
#' This function cumulates all available Hector parameters, such as BETA, ESC,
#' and Volcanic Scale
#'
#' @param core Hector object
#' @return rslt_tot Dataframe containing the parameters/variables
#' @family main user interface functions
#' @export
cum_vars_params <- function(core) {
    vars <- c(
        hector::AERO_SCALE(),
        hector::BETA(),           # Assertion failed: Date not allowed
        hector::DIFFUSIVITY(),
        hector::ECS(),
        hector::F_NPPV(),         # Assertion failed: Date not allowed
        hector::F_NPPD(),         # Assertion failed: Date not allowed
        hector::F_LITTERD(),      # Assertion failed: Date not allowed
        hector::F_LUCV(),         # Assertion failed: Date not allowed
        hector::Q10_RH(),         # Assertion failed: Date not allowed
        hector::VOLCANIC_SCALE(),
        hector::WARMINGFACTOR()   # Assertion failed: Date not allowed
    )

    rslt_tot <- do.call(rbind,
                        lapply(vars, function(v) {
                            sendmessage(core, GETDATA(), v, NA, NA, '')
                        }))

    return(rslt_tot)
}


#' Cumulate Hector output variables
#'
#' This function cumulates all available output variables from an active Hector
#' core (e.g.,  Concentrations, Forcings, etc.)
#'
#' @param core Hector core object
#' @param dates Vector of dates
#' @return rslt_tot Dataframe containing the variables
#' @family main user interface functions
#' @export
cum_vars_output <- function(core, dates) {

    ### Concentration vars with date arg
    vars_conc_d <- c(
        # hector::ATM_OCEAN_FLUX_HL(),      # Assertion failed: Unknown model datum
        # hector::ATM_OCEAN_FLUX_LL(),      # Assertion failed: Unknown model datum
        hector::ATMOSPHERIC_C(),
        hector::ATMOSPHERIC_CH4(),
        hector::ATMOSPHERIC_CO2(),
        hector::ATMOSPHERIC_N2O(),
        hector::ATMOSPHERIC_O3()
        # hector::CO3_HL(),                 # Assertion failed: Unknown model datum
        # hector::CO3_LL(),                 # Assertion failed: Unknown model datum
        # hector::DIC_HL(),                 # Assertion failed: Unknown model datum
        # hector::DIC_LL(),                 # Assertion failed: Unknown model datum
    )

    vars_conc_nd <- c(
        hector::LAND_CFLUX(),             # Assertion failed: Date not allowed
        # hector::NATURAL_CH4(),            # Assertion failed: Unknown model datum
        hector::NATURAL_SO2(),            # Assertion failed: Date not allowed
        hector::OCEAN_C(),                # Assertion failed: Date data not available
        hector::OCEAN_C_DO(),             # Assertion failed: Date data not available
        hector::OCEAN_C_HL(),             # Assertion failed: Date data not available
        hector::OCEAN_C_IO(),             # Assertion failed: Date data not available
        hector::OCEAN_C_LL(),             # Assertion failed: Date data not available
        hector::OCEAN_CFLUX()            # Assertion failed: Date data not available
        # hector::PCO2_HL(),                # Assertion failed: Unknown model datum
        # hector::PCO2_LL(),                # Assertion failed: Unknown model datum
        # hector::PH_HL(),                  # Assertion failed: Unknown model datum
        # hector::PH_LL(),                  # Assertion failed: Unknown model datum
    )

    ### Date arg OK
    vars_temps <- c(
        hector::GLOBAL_TEMP(),
        hector::GLOBAL_TEMPEQ(),
        hector::HEAT_FLUX(),
        hector::LAND_AIR_TEMP(),
        hector::OCEAN_AIR_TEMP(),
        hector::OCEAN_SURFACE_TEMP()
    )

    ### Date arg OK
    vars_rf <- c(
        hector::RF_BC(),
        hector::RF_C2F6(),
        hector::RF_CCL4(),
        hector::RF_CF4(),
        hector::RF_CFC11(),
        hector::RF_CFC113(),
        hector::RF_CFC114(),
        hector::RF_CFC115(),
        hector::RF_CFC12(),
        hector::RF_CH3BR(),
        hector::RF_CH3CCL3(),
        hector::RF_CH3CL(),
        hector::RF_CH4(),
        hector::RF_CO2(),
        hector::RF_H2O(),
        hector::RF_HALON1211(),
        hector::RF_HALON1301(),
        hector::RF_HALON2402(),
        hector::RF_HCF141B(),
        hector::RF_HCF142B(),
        hector::RF_HCF22(),
        hector::RF_HFC125(),
        hector::RF_HFC134A(),
        hector::RF_HFC143A(),
        hector::RF_HFC227EA(),
        hector::RF_HFC23(),
        hector::RF_HFC245FA(),
        hector::RF_HFC32(),
        hector::RF_HFC4310(),
        hector::RF_N2O(),
        hector::RF_O3(),
        hector::RF_OC(),
        hector::RF_SF6(),
        hector::RF_SO2(),
        hector::RF_SO2D(),
        hector::RF_SO2I(),
        hector::RF_T_ALBEDO(),
        hector::RF_TOTAL(),
        hector::RF_VOL()
    )

    ### Concat the lists of vars that can use the dates param in sendmessage() to
    ### decrease the amount of calls made to do.call(...)
    vars_d <- c(vars_conc_d, vars_temps, vars_temps)

    rslt_nd <- do.call(rbind,
                       lapply(vars_conc_nd, function(v) {
                           sendmessage(core, GETDATA(), v, NA, NA, '')
                       }))


    rslt_d <- do.call(rbind,
                      lapply(vars_d, function(v) {
                          sendmessage(core, GETDATA(), v, dates, NA, '')
                      }))

    rslt_tot <- rbind(rslt_nd, rslt_d)

    return(rslt_tot)
}



#' Fetch all the available Hector variables
#'
#' This function is similar to fetchvars, except it fetches all available
#' Hector variables, including exogenous inputs (e.g., emissions & pre-industrial
#' concentrations), parameters (e.g., alpha, beta, ECS), and output
#' variables (e.g., concentrations & forcings).
#'
#' @param core         Hector core object
#' @param dates        Vector of dates, optional
#' @param scenario     Scenario name, optional str
#' @return vars_all    Dataframe containing all Hector variables
#' @family main user interface functions
#' @export
fetchvars_all <- function(core, dates=NULL, scenario=NULL) {

    if (is.null(scenario)) {
        scenario <- core$name
    }


    strt <- startdate(core)
    end <- getdate(core)

    if (is.null(dates)) {
        # Get vars for entire date range
        dates <- strt:end
    } else {
        # Use the date range specified in 'dates' param
        valid <- dates >= strt & dates <= end
        dates <- dates[valid]
    }

    vars_inpt  <- cum_vars_input(core, dates)
    vars_param <- cum_vars_params(core)
    vars_outpt <- cum_vars_output(core, dates)

    vars_all <- rbind(vars_inpt, vars_param, vars_outpt)

    return(vars_all)
}
