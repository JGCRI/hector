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
#' @family fetchvars_all helper function
cum_vars_input <- function(core, dates, lib_funcs) {

    # Use regex to find the indices of EMISSIONS_* function names
    func_idx <- grep("^EMISSIONS_+", lib_funcs)

    # Get the variable closures and execute them to get the capabililty strings
    var_funcs <- sapply(lib_funcs[func_idx], get)

    # Don't really know what this does exactly but it makes the sendmessage()
    # call work
    vars_em <- getOption('hector.vars.emissions',
                         default=sapply(var_funcs, function(f){f()}))

    # Repeat the above process for PREINDUSTRIAL_* functions
    func_idx <- grep("^PREINDUSTRIAL_+", lib_funcs)
    var_funcs <- sapply(lib_funcs[func_idx], get)
    vars_conc <- getOption('hector.vars.preindustrial',
                           default=sapply(var_funcs, function(f){f()}))


    # Get model output for variables that use date arg
    rslt_em <- do.call(rbind,
                       lapply(vars_em, function(v) {
                           sendmessage(core, GETDATA(), v, dates, NA, '')
                       }))

    # Get model output for variables that DO NOT use date arg
    rslt_conc <- do.call(rbind,
                         lapply(vars_conc, function(v) {
                             sendmessage(core, GETDATA(), v, NA, NA, '')
                         }))

    rslt_tot <- rbind(rslt_em, rslt_conc)

    invisible(rslt_em)
}


#' Cumulate Hector parameters
#'
#' This function cumulates all available Hector parameters, such as BETA, ESC,
#' and Volcanic Scale
#'
#' @param core Hector object
#' @return rslt_tot Dataframe containing the parameters/variables
#' @family fetchvars_all helper function
cum_vars_params <- function(core) {

    # These variables don't follow a common naming rule (i.e., EMISSIONS_*),
    # so the best option is to hardcode
    vars <- c(AERO_SCALE(), BETA(), DIFFUSIVITY(), ECS(), F_NPPV(), F_NPPD(),
              F_LITTERD(), F_LUCV(), Q10_RH(), VOLCANIC_SCALE(), WARMINGFACTOR()
    )

    # Get the data for the given list of variables
    rslt_tot <- do.call(rbind,
                        lapply(vars, function(v) {
                            sendmessage(core, GETDATA(), v, NA, NA, '')
                        }))

    invisible(rslt_tot)
}


#' Cumulate Hector output variables
#'
#' This function cumulates all available output variables from an active Hector
#' core (e.g.,  Concentrations, Forcings, etc.)
#'
#' @param core Hector core object
#' @param dates Vector of dates
#' @return rslt_tot Dataframe containing the variables
#' @family fetchvars_all helper function
cum_vars_output <- function(core, dates, lib_funcs) {

    # Atmospheric concentrationvars. Take date arg
    func_idx <- grep("^ATMOSPHERIC_+", lib_funcs)
    var_funcs <- sapply(lib_funcs[func_idx], get)
    vars_conc_d <- getOption('hector.vars.atmospheric',
                             default=sapply(var_funcs, function(f){f()}))


    # Various concenctration, temperature, & flux parameters that
    # DO NOT take a date arg
    func_idx <- grep("^OCEAN_C(_\\w{2})?$|^ATM_OCEAN_+|\\w{1,5}_[HL]L$", lib_funcs)
    var_funcs <- sapply(lib_funcs[func_idx], get)
    vars_ocn_nd <- getOption('hector.vars.ocean',
                             default=sapply(var_funcs, function(f){f()}))

    # Various CFLUX and Natural emission variables. DO NOT take date arg
    func_idx <- grep("^\\w{4,5}_CFLUX$|^NATURAL_+", lib_funcs)
    var_funcs <- sapply(lib_funcs[func_idx], get)
    vars_cflux_nd <- getOption('hector.vars.cflux',
                               default=sapply(var_funcs, function(f){f()}))

    # Temperature and flux variabels. Take date arg
    func_idx <- grep("+_TEMP(\\w{2})?$|+_FLUX$", lib_funcs)
    var_funcs <- sapply(lib_funcs[func_idx], get)
    vars_temps_d <- getOption('hector.vars.oceantemps',
                              default=sapply(var_funcs, function(f){f()}))

    # Radiative forcing variables. Date arg OK
    func_idx <- grep("PREINDUSTRIAL_+", lib_funcs)
    var_funcs <- sapply(lib_funcs[func_idx], get)
    vars_rf <- getOption('hector.vars.radforcing',
                         default=sapply(var_funcs, function(f){f()}))

    # Concat the lists of vars that do & do not use date arg, respectively
    vars_d <- c(vars_conc_d, vars_temps_d)
    vars_nd <- c(vars_ocn_nd, vars_cflux_nd)

    # Get variables that DO NOT use date arg
    rslt_nd <- do.call(rbind,
                       lapply(vars_nd, function(v) {
                           sendmessage(core, GETDATA(), v, NA, NA, '')
                       }))

    # Get variables that use date arg
    rslt_d <- do.call(rbind,
                      lapply(vars_d, function(v) {
                          sendmessage(core, GETDATA(), v, dates, NA, '')
                      }))

    # Combine the results of the sendmessage calls into one dataframe
    rslt_tot <- rbind(rslt_nd, rslt_d)

    invisible(rslt_tot)
}



#' Fetch all the available Hector variables
#'
#' This function is similar to fetchvars, except it fetches all available
#' Hector variables, including exogenous inputs (e.g., emissions & pre-industrial
#' concentrations), parameters (e.g., alpha, beta, ECS), and output
#' variables (e.g., concentrations & forcings).
#'
#' @param core Hector core object
#' @param dates ector of dates, optional
#' @param scenario Scenario name, optional str
#' @param write If TRUE, write the resulting dataframe to a csv file, optional bool
#' @param outpath Absolute path of the output csv. Must be given if 'write' is true
#' @return vars_all    Dataframe containing all Hector variables
#' @family main user interface functions
#' @export
fetchvars_all <- function(core, dates=NULL, scenario=NULL, write=F, outpath=NULL) {

    if (is.null(scenario)) {
        scenario <- core$name
    }

    # Load the list of available functions in the hector R library.
    # Will be passed to helper funcs to match capability strings using regex
    lib_funcs <- lsf.str("package:hector")

    strt <- startdate(core)
    end <- getdate(core)

    if (is.null(dates)) {
        dates <- strt:end
    } else {
        valid <- dates >= strt & dates <= end
        dates <- dates[valid]
    }

    # Get variables from the helper functions, combine results into one dataframe
    vars_inpt  <- cum_vars_input(core, dates, lib_funcs)
    vars_param <- cum_vars_params(core)
    vars_outpt <- cum_vars_output(core, dates, lib_funcs)

    vars_all <- rbind(vars_inpt, vars_param, vars_outpt)

    # If 'write' is TRUE, reshape the resulting data frame to a wide format
    # and write to csv
    if (write) {

        # Pivot the dataframe "wide" so years run along the x axis
        # Catch warning from reshape()
        suppressWarnings(vars_all <- reshape(vars_all, direction="wide",
                                     idvar=c("variable", "units"), timevar="year"))

        # Clean up the column names (value.YYYY --> YYYY)
        col_names <- colnames(vars_all)[-1:-2]
        col_names <- sapply(col_names, function(x) sub("value.", "", x), USE.NAMES=F)
        col_names <- col_names[-length(col_names)]

        col_names <- c("variable", "units", "initial value", col_names)

        num_cols <- length(col_names)

        # Move last column into 3rd col position
        vars_all <- vars_all[, c(1, 2, num_cols, 4:num_cols-1)]

        # Set the column names for the re-ordered dataframe
        colnames(vars_all) <- col_names

        # Check if the absolute path of the output csv file is valid
        if (!is.null(outpath)) {
            write.table(vars_all, outpath, col.names=col_names, row.names=F, sep=',')
            cat("Output written to ", outpath, sep="")
        } else {
            # Write to a default location in the Hector R library output directory
            base_path <- find.package("hector")
            f_name <- "fetchvars_all.csv"
            abs_path <- file.path(base_path, "output", f_name)

            write.table(vars_all, abs_path, col.names=col_names, row.names=F, sep=',')

            cat("Output written to ", abs_path, sep="")
        }
    }

    invisible(vars_all)
}
