#' Get the hector unit strings for one or more variables
#'
#' This function provides units only for variables that can be set in the model
#' using \code{\link{setvar}}.  Variables read from the model with
#' \code{\link{fetchvars}} will have a units column in the data frame they are
#' returned in.
#' @param vars Capability strings for variables to get units for.
#' @return Character vector of unit strings. Variables not found will have \code{NA} entries.
#' @export
getunits <- function(vars)
{
    ## NB the unitstable data structure is included as internal package data.
    rows <- match(vars, unitstable$variable)
    units <- unitstable$units[rows]
    if(any(is.na(units))) {
        warning('Units entries for the following variables not found: ',
                paste(vars[is.na(units)], collapse=', '))
    }
    as.character(units)
}
