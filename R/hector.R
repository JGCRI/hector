#' @useDynLib hector
#' @importFrom Rcpp sourceCpp
NULL

#' Run a test of basic hector functionality
#' @keywords internal
runtest <- function()
{
    core <- newcore('inst/input/hector_rcp45.ini')
    run(core)
    shutdown(core)
}
