#### Documentation objects
#' Hector logging levels
#'
#' These functions return constants that can be used in
#' \code{\link{newcore}} to set the minimum logging level
#'
#' @name loglevels
NULL


#' Utility functions for Hector instances
#'
#' @param core Hector instance to operate on
#' @name hectorutil
NULL


#' Message type identifiers
#'
#' These identifiers indicate the type of a message so that the core can route
#' it to the correct method in the recipient.
#'
#' @section Note:
#' Because these identifiers are provided as \code{#define} macros in the hector code,
#' these identifiers are provided in the R interface as function.  Therefore,
#' these objects must be called to use them; \emph{e.g.}, \code{GETDATA()}
#' instead of the more natural looking \code{GETDATA}.
#'
#' @name msgtype
NULL


#' Identifiers for capabilities in the Hector forcing component
#'
#' These identifiers specify forcing values that can be provided by hector via
#' the forcing component.  All of the values corresponding to these identifiers
#' are read-only (\emph{i.e.}, they can only appear in \code{\link{GETDATA}}
#' messages.)
#'
#' @inheritSection msgtype Note
#'
#' @name forcings
#' @seealso \link{haloforcings} for forcings from halocarbons.
#' @family capability identifiers
NULL


#' Identifiers for variables in the Hector carbon cycle component
#'
#' These identifiers correspond to variables that can be read and/or set in the
#' carbon cycle
#'
#' @section Output variables:
#' These variables can be read using the \code{\link{GETDATA}} message type:
#' \describe{
#' \item{ATMOSPHERIC_CO2}{Atmospheric CO2 concentration}
#' \item{ATMOSPHERIC_C}{Atmospheric carbon pool}
#' \item{LAND_CFLUX}{Land carbon flux}
#' \item{FFI_EMISSIONS}{Fossil fuel and industrial emissions}
#' \item{LUC_EMISSIONS}{Land use change emissions}
#' }
#' @section Input variables:
#' \describe{
#' \item{FFI_EMISSIONS}{Fossil fuel and industrial emissions (\code{"Pg C/yr"})}
#' \item{LUC_EMISSIONS}{Land use change emissions (\code{"Pg C/yr"})}
#' }
#' @inheritSection msgtype Note
#' @name carboncycle
#' @seealso \link{concentrations} for other gas concentrations and
#' \link{emissions} for other gas emissions
#' @family capability identifiers
NULL

#' Identifiers for halocarbon forcings
#'
#' These identifiers specify forcing values that can be provided by hector via
#' one of the myriad halocarbon components.  All of the values corresponding to
#' these identifiers are read-only (\emph{i.e.}, they can only appear in
#' \code{\link{GETDATA}} messages.)  The forcings returned are the
#' \emph{relative} forcings, with the base year (typically 1750) values
#' subtracted off.
#'
#' @inheritSection msgtype Note
#'
#' @name haloforcings
#' @seealso \link{forcings} for forcings from other sources.
#' @family capability identifiers
NULL


#' Identifiers for halocarbon emissions
#'
#' These identifiers correspond to emissions values for halocarbons.  They are
#' (for now) write only, meaning they can only appear in \code{\link{SETDATA}}
#' messages.  In all cases, the expected input units are gigagrams
#' (\code{"Gg"}).
#'
#' @inheritSection msgtype Note
#'
#' @name haloemiss
#' @family capability identifiers
NULL


#' Identifiers for quantities in the methane component
#'
#' These identifiers correspond to variables that can be read and/or set in the
#' methane component.
#'
#' @section Output variables:
#' These variables can be read from the methane component.
#' \describe{
#' \item{ATMOSPHERIC_CH4}{Atmospheric methane concentration}
#' \item{PREINDUSTRIAL_CH4}{Preindustrial methane concentration}
#' }
#'
#'
#' @section Input variables:
#' These variables can be set in the methane component.  The expected units string is given after
#' each description.
#' \describe{
#' \item{EMISSIONS_CH4}{Methane emissions (\code{"Tg CH4"})}
#' \item{PREINDUSTRIAL_CH4}{Preindustrial methane concentration (\code{"ppbv CH4"})}
#' \item{NATURAL_CH4}{Natural methane emissions (\code{"Tg CH4"})}
#' \item{LIFETIME_SOIL}{Time scale for methane loss into soil (\code{"Years"})}
#' \item{LIFETIME_STRAT}{Time scale for methane loss into stratosphere (\code{"Years"})}
#' }
#'
#' @inheritSection msgtype Note
#'
#' @name methane
#' @family capability identifiers
NULL

#' Identifiers for quantities in the SO2 component
#'
#' These identifiers correspond to variables that can be read and/or set in the
#' SO2 component
#'
#' @section Output variables:
#' These variables can be read using the \code{\link{GETDATA}} message type.
#' \describe{
#' \item{NATURAL_SO2}{Natural SO2 emissions}
#' \item{Y2000_SO2}{Year 2000 SO2 emissions}
#' \item{EMISIONS_SO2}{Anthropogenic SO2 emissions}
#' \item{VOLCANIC_SO2}{Forcing due to volcanic SO2 emissions}
#' }
#'
#' @section Input variables:
#' These variables can be set using the \code{\link{SETDATA}} message type.
#' \describe{
#' \item{EMISSIONS_SO2}{Anthropogenic SO2 emissions (\code{"Gg S"})}
#' \item{VOLCANIC_SO2}{Forcing due to volcanic SO2 emissions (\code{"W/m2"})}
#' }
#'
#' @inheritSection msgtype Note
#'
#' @name so2
#' @family capability identifiers
NULL

#' Identifiers for quantities in the ocean component
#'
#' These identifiers correspond to variables that can be read from the ocean
#' component using the \code{\link{GETDATA}} message type.
#'
#' @inheritSection msgtype Note
#'
#' @section To Do:
#' Some of these could be made writeable as well as readable.  Notably, TT, TU,
#' TWI, and TID are only set from the input, not calculated.
#' @name ocean
#' @family capability identifiers
NULL

#' Identifiers for miscellaneous concentrations not elsewhere described
#'
#' All of these variables can be read using the \code{\link{GETDATA}} message
#' type.
#'
#' @inheritSection msgtype Note
#' @name concentrations
#' @family capability identifiers
NULL

#' Identifiers for miscellaneous emissions not elsewhere described
#'
#' All of these variables can be set using the \code{\link{SETDATA}} message
#' type.
#'
#' @inheritSection msgtype Note
#' @name emissions
#' @family capability identifiers
NULL

#' Identifiers for model parameters
#'
#' These identifiers correspond to settable parameters that change the model
#' behavior and are subject to uncertainty.  All of these can be set using the
#' \code{\link{SETDATA}} message type.  Changing any of these parameters will
#' typically invalidate the hector core's internal state; therefore, after
#' setting one or more of these values you should call \code{\link{reset}} before
#' attempting to run the model again.  This will rerun the spinup and produce a
#' new internally consistent state.  Attempting to run the model without resetting
#' first will usually produce an error (often with a message about failing to conserve
#' mass).
#'
#' @inheritSection msgtype Note
#' @name parameters
#' @family capability identifiers
NULL

#' Identifiers for variables associated with the temperature component
#'
#' All of these variables can be read using the \code{\link{GETDATA}} message
#' type.
#'
#' @inheritSection msgtype Note
#' @name temperature
#' @family capability identifiers
NULL
