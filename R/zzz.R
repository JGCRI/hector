#### package options, etc.

.onLoad <- function(libname, pkgname)
{
    opts <- options()
    opts.hector <- list(
        hector.default.fetchvars =
          c(ATMOSPHERIC_CO2(), RF_TOTAL(), RF_CO2(),
            GLOBAL_TEMP())
        )
    setopts <- !(names(opts.hector) %in% names(opts))
    if(any(setopts)) {
        options(opts.hector[setopts])
    }

    invisible()
}

