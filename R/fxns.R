#' Get the hector function associated with a specific string
#'
#' This function provides the Hector function that returns a the Hector string
#' @param str String name to find the Hector function for
#' @return Hector function name
#' @export
#' @seealso getunits
#' @examples
#' getfxn("beta")
#' getfxn("q10_rh")
getfxn <- function(str) {
  rows <- match(str, hector::fxntable$string)
  rslt <- hector::fxntable$fxn[rows]
  if (any(is.na(rows))) {
    warning(
      "Functions for the following are not found: ",
      paste(str[is.na(rslt)], collapse = ", ")
    )
  }
  as.character(rslt)
}


#' Get all of the possible Hector output variables
#'
#' This function returns all of the possible hector output variables
#' that can be accessed with \code{\link{fetchvars}}.
#' @return Character vector of variable names.
#' @export
#' @family outputs
#' @examples
#' \dontrun{
#' ini <- system.file(package = "hector", "input/hector_ssp245.ini")
#' hc <- newcore(ini)
#' run(hc)
#' out <- fetchvars(core = hc, dates = 1900:2100, vars = ALL_VARS())
#' print(out)
#' }
ALL_VARS <- function() {
  all_vars
}

#' Get all of the possible Hector halocarbon concentrations
#'
#' This function returns a vector of the possible hector halocarbon concentrations
#' that can be accessed with \code{\link{fetchvars}}.
#' @return Character vector of variable names.
#' @export
#' @family outputs
#' @examples
#' \dontrun{
#' ini <- system.file(package = "hector", "input/hector_ssp245.ini")
#' hc <- newcore(ini)
#' run(hc)
#' out <- fetchvars(core = hc, dates = 1900:2100, vars = ALL_HALOCARBON_CONCENTRATIONS())
#' print(out)
#' }
ALL_HALOCARBON_CONCENTRATIONS <- function() {
    halo_conc
}


#' Get all of the possible Hector halocarbon emissions
#'
#' This function returns a vector of the possible hector halocarbon emissions
#' that can be accessed with \code{\link{fetchvars}} or set with \code{\link{setvar}}.
#' @return Character vector of variable names.
#' @export
#' @family haloemiss
#' @examples
#' \dontrun{
#' ini <- system.file(package = "hector", "input/hector_ssp245.ini")
#' hc <- newcore(ini)
#' run(hc)
#' out <- fetchvars(core = hc, dates = 1900:2100, vars = ALL_HALOCARBON_EMISSIONS())
#' print(out)
#' }
ALL_HALOCARBON_EMISSIONS <- function() {
    halo_emiss
}


#' Get all of the possible Hector halocarbon constraints
#'
#' This function returns a vector of the possible hector halocarbon constraints
#' that can be accessed with \code{\link{fetchvars}} or set with \code{\link{setvar}}.
#' @return Character vector of variable names.
#' @export
#' @family outputs, inputs
#' @examples
#' \dontrun{
#' ini <- system.file(package = "hector", "input/hector_ssp245.ini")
#' hc <- newcore(ini)
#' run(hc)
#' out <- fetchvars(core = hc, dates = 1900:2100, vars = ALL_HALOCARBON_CONSTRAINTS())
#' print(out)
#' }
ALL_HALOCARBON_CONSTRAINTS <- function() {
    halo_constraints
}

#' Get all of the possible Hector halocarbon radiative forcings
#'
#' This function returns a vector of the possible hector halocarbon forcings
#' that can be accessed with \code{\link{fetchvars}}.
#' @return Character vector of variable names.
#' @export
#' @family outputs
#' @examples
#' \dontrun{
#' ini <- system.file(package = "hector", "input/hector_ssp245.ini")
#' hc <- newcore(ini)
#' run(hc)
#' out <- fetchvars(core = hc, dates = 1900:2100, vars = ALL_HALOCARBON_RF())
#' print(out)
#' }
ALL_HALOCARBON_RF <- function() {
    halo_rf
}
