#' Create new biome
#'
#' @param core Hector core
#' @param biome Name of new biome
#' @param veg_c0 Initial vegetation C pool
#' @param detritus_c0 Initial detritus C pool
#' @param soil_c0 Initial soil C pool
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
                         veg_c0, detritus_c0, soil_c0,
                         npp_flux0,
                         warmingfactor = 1,
                         beta = 0.36,
                         q10_rh = 2.0,
                         f_nppv = 0.35,
                         f_nppd = 0.60,
                         f_litterd = 0.98) {
  create_biome_impl(core, biome)
  setvar(core, 0, VEG_C(biome), veg_c0, "PgC")
  setvar(core, 0, DETRITUS_C(biome), detritus_c0, "PgC")
  setvar(core, 0, SOIL_C(biome), soil_c0, "PgC")
  setvar(core, NA, NPP_FLUX0(biome), npp_flux0, "PgC/yr")
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
#'   in `new_biomes`. Must be the same length as `new_biomes`. Default
#'   is to split vegetation C evenly between all biomes.
#' @param fdetritus_c Fractions of detritus C distributed to each
#'   biome. Defaults to the same value as `fveg_c`.
#' @param fsoil_c Fractions of soil C distributed to each biome.
#'   Defaults to the same value as `fveg_c`.
#' @param fnpp_flux0 Fraction of initial NPP flux distributed to each
#'   biome. Defaults to the same value as `fveg_c`.
#' @param ... Additional biome-specific parameters, as set by
#'   [create_biome()]. Note that these are passed to [create_biome()]
#'   via [base::mapply()], so they can be vectorized across biomes.
#' @export
split_biome <- function(core,
                        old_biome,
                        new_biomes,
                        fveg_c = rep(1 / length(new_biomes), length(new_biomes)),
                        fdetritus_c = fveg_c,
                        fsoil_c = fveg_c,
                        fnpp_flux0 = fveg_c,
                        ...) {

  stopifnot(
    length(old_biome) == 1,
    old_biome %in% get_biome_list(core),
    !any(new_biomes %in% get_biome_list(core)),
    length(fveg_c) == length(new_biomes),
    length(fdetritus_c) == length(new_biomes),
    length(fsoil_c) == length(new_biomes),
    length(fnpp_flux0) == length(new_biomes),
    sum(fveg_c) == 1, all(fveg_c > 0),
    sum(fdetritus_c) == 1, all(fdetritus_c > 0),
    sum(fsoil_c) == 1, all(fsoil_c > 0),
    sum(fnpp_flux0) == 1, all(fnpp_flux0 > 0)
  )

  current_values <- get_biome_inits(core, old_biome)

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
    veg_c0 = current_values[["veg_c"]] * fveg_c,
    detritus_c0 = current_values[["detritus_c"]] * fdetritus_c,
    soil_c0 = current_values[["soil_c"]] * fsoil_c,
    npp_flux0 = current_values[["npp_flux0"]] * fnpp_flux0,
    ...,
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
    sendmessage(core, GETDATA(), SOIL_C(biome), 0, NA, "")
  )
  current_data_2 <- fetchvars(core, NA, c(NPP_FLUX0(biome),
                                          BETA(biome),
                                          WARMINGFACTOR(biome)))[, -1]
  current_data <- rbind.data.frame(current_data_1, current_data_2)
  current_values <- current_data[["value"]]
  names(current_values) <- gsub(paste0(biome, "."), "",
                                current_data[["variable"]], fixed = TRUE)
  current_values
}
