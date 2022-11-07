#' Get the hector function associated with a specific string
#'
#' This function provides the Hector function that returns a the Hector string
#' @param strstr String name to find the Hector function for
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
