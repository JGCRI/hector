#' Create new biome
#'
#' @param core Hector core
#' @param biome Name of new biome
#' @param veg_c0 Initial vegetation C pool
#' @param detritus_c0 Initial detritus C pool
#' @param soil_c0 Initial soil C pool
#' @param permafrost_c0 Initial permafrost C pool
#' @param npp_flux0 Initial net primary productivity
#' @param warmingfactor Temperature multiplier (default =
#'   `1.0`)
#' @param beta CO2 fertilization effect (default = `0.36`)
#' @param q10_rh Q10 of heterotrophic respiration (default = `2.0`)
#' @param f_nppv Fraction of NPP to vegetation (default = `0.35`)
#' @param f_nppd Fraction of NPP to detritus (default = `0.60`)
#' @param f_litterd Fraction of litter flux to detritus (default = `0.98`)
#' @return Hector core, invisibly
#' @author Alexey Shiklomanov
#' @export
create_biome <- function(core, biome,
                         veg_c0, detritus_c0, soil_c0, permafrost_c0,
                         npp_flux0,
                         warmingfactor, beta, q10_rh,
                         f_nppv, f_nppd, f_litterd) {
    create_biome_impl(core, biome)
    setvar(core, 0, VEG_C(biome), veg_c0, "Pg C")
    setvar(core, 0, DETRITUS_C(biome), detritus_c0, "Pg C")
    setvar(core, 0, SOIL_C(biome), soil_c0, "Pg C")
    setvar(core, 0, PERMAFROST_C(biome), permafrost_c0, "Pg C")
    setvar(core, NA, NPP_FLUX0(biome), npp_flux0, "Pg C/yr")
    setvar(core, NA, WARMINGFACTOR(biome), warmingfactor, NA)
    setvar(core, NA, BETA(biome), beta, NA)
    setvar(core, NA, Q10_RH(biome), q10_rh, NA)
    setvar(core, NA, F_NPPV(biome), f_nppv, NA)
    setvar(core, NA, F_NPPD(biome), f_nppd, NA)
    setvar(core, NA, F_LITTERD(biome), f_litterd, NA)
    invisible(core)
}

#' Create new biomes by splitting up an existing biome
#'
#' Distributes vegetation, detritus, and soil C, and initial NPP flux
#' from an existing biome across multiple new biomes.
#'
#' @param core Hector core
#' @param old_biome Name of biome that will be split up
#' @param new_biomes Names of biomes to be created
#' @param fveg_c Fractions of vegetation C distributed to each biome
#'   in \code{new_biomes}; must be the same length as it. Default
#'   is to split vegetation C evenly between all biomes.
#' @param fdetritus_c Fractions of detritus C distributed to each
#'   biome. Defaults to the same value as \code{fveg_c}.
#' @param fsoil_c Fractions of soil C distributed to each biome.
#'   Defaults to the same value as \code{fveg_c}.
#' @param fpermafrost_c Fraction of permafrost C distributed to each
#'   biome. Defaults to the same value as \code{fveg_c}.
#' @param fnpp_flux0 Fraction of initial NPP flux distributed to each
#'   biome. Defaults to the same value as \code{fveg_c}.
#' @param ... Additional biome parameters passed to \code{\link{create_biome}}.
#' @export
split_biome <- function(core,
                        old_biome,
                        new_biomes,
                        fveg_c = rep(1 / length(new_biomes), length(new_biomes)),
                        fdetritus_c = fveg_c,
                        fsoil_c = fveg_c,
                        fpermafrost_c = fveg_c,
                        fnpp_flux0 = fveg_c,
                        ...) {
    stopifnot(
        length(old_biome) == 1,
        old_biome %in% get_biome_list(core),
        !any(new_biomes %in% get_biome_list(core)),
        length(fveg_c) == length(new_biomes),
        length(fdetritus_c) == length(new_biomes),
        length(fsoil_c) == length(new_biomes),
        length(fpermafrost_c) == length(new_biomes),
        length(fnpp_flux0) == length(new_biomes),
        sum(fveg_c) == 1, all(fveg_c > 0),
        sum(fdetritus_c) == 1, all(fdetritus_c > 0),
        sum(fsoil_c) == 1, all(fsoil_c > 0),
        sum(fpermafrost_c) == 1, all(fpermafrost_c >= 0),
        sum(fnpp_flux0) == 1, all(fnpp_flux0 > 0)
    )

    # If user supplied values in ... for warmingfactor, etc., use those
    # Otherwise use the old biome's values
    cv <- get_biome_inits(core, old_biome) # current values
    dots <- list(...)
    # We can't use ifelse for this because return shape may differ
    pick <- function(x, y) { if (is.null(x)) y else x } # nolint
    warmingfactor <- pick(dots$warmingfactor, cv[["warmingfactor"]])
    beta <- pick(dots$beta, cv[["beta"]])
    q10_rh <- pick(dots$q10_rh, cv[["q10_rh"]])
    f_nppv <- pick(dots$f_nppv, cv[["f_nppv"]])
    f_nppd <- pick(dots$f_nppd, cv[["f_nppd"]])
    f_litterd <- pick(dots$f_litterd, cv[["f_litterd"]])

    # This allows users to split the `global` biome without having to
    # rename it first. Otherwise, we hit an error about using non-global
    # biomes when a "global" biome is present.
    if (old_biome == "global") {
        invisible(rename_biome(core, "global", "_zzz"))
        old_biome <- "_zzz"
    }

    mapply(
        create_biome,
        biome = new_biomes,
        veg_c0 = cv[["veg_c"]] * fveg_c,
        detritus_c0 = cv[["detritus_c"]] * fdetritus_c,
        soil_c0 = cv[["soil_c"]] * fsoil_c,
        permafrost_c0 = cv[["permafrost_c"]] * fpermafrost_c,
        npp_flux0 = cv[["npp_flux0"]] * fnpp_flux0,
        warmingfactor = warmingfactor,
        beta = beta,
        q10_rh = q10_rh,
        f_nppv = f_nppv,
        f_nppd = f_nppd,
        f_litterd = f_litterd,
        MoreArgs = list(core = core)
    )

    delete_biome_impl(core, old_biome)

    reset(core, 0)
    invisible(core)
}

#' Retrieve the initial conditions and parameters for a given biome
#'
#' Internal helper function for biome functions.
#'
#' @param core Hector core
#' @param biome Name of biome for which to retrieve data
#' @return Named numeric vector of biome initial conditions and parameters
#' @author Alexey Shiklomanov
get_biome_inits <- function(core, biome) {
    # `fetchvars` requires date to be between start and end date, so
    # we need to call the lower-level `sendmessage` method here.
    current_data_1 <- rbind.data.frame(
        sendmessage(core, GETDATA(), VEG_C(biome), 0, NA, ""),
        sendmessage(core, GETDATA(), DETRITUS_C(biome), 0, NA, ""),
        sendmessage(core, GETDATA(), SOIL_C(biome), 0, NA, ""),
        sendmessage(core, GETDATA(), PERMAFROST_C(biome), 0, NA, ""),
        sendmessage(core, GETDATA(), THAWEDP_C(biome), 0, NA, "")
    )
    current_data_2 <- fetchvars(core, NA, c(
        NPP_FLUX0(biome),
        F_LITTERD(biome),
        F_NPPD(biome),
        F_NPPV(biome),
        BETA(biome),
        Q10_RH(biome),
        WARMINGFACTOR(biome)
    ))[, -1]
    current_data <- rbind.data.frame(current_data_1, current_data_2)
    current_values <- current_data[["value"]]
    names(current_values) <- gsub(paste0(biome, BIOME_SPLIT_CHAR()), "",
                                  current_data[["variable"]],
                                  fixed = TRUE
    )
    current_values
}
