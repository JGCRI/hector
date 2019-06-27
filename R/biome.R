#' Create new biome
#'
#' @param core Hector core
#' @param biome Name of new biome
#' @param veg_c0 Initial vegetation C pool (default = `0`)
#' @param detritus_c0 Initial detritus C pool (default = `0`)
#' @param soil_c0 Initial soil C pool (default = `0`)
#' @param npp_flux0 Initial net primary productivity (default = `0`)
#' @param warmingfactor Temperature multiplier (default =
#'   `1.0`)
#' @param beta CO2 fertilization effect (default = `0.36`) 
#' @return Hector core, invisibly
#' @author Alexey Shiklomanov
#' @export
create_biome <- function(core, biome,
                         veg_c0 = 0,
                         detritus_c0 = 0,
                         soil_c0 = 0,
                         npp_flux0 = 0,
                         warmingfactor = 1,
                         beta = 0.36) {
  c_create_biome(core, biome)
  setvar(core, 0, VEG_C(biome), veg_c0, "PgC")
  setvar(core, 0, DETRITUS_C(biome), detritus_c0, "PgC")
  setvar(core, 0, SOIL_C(biome), soil_c0, "PgC")
  setvar(core, NA, NPP_FLUX0(biome), npp_flux0, "PgC/yr")
  setvar(core, NA, WARMINGFACTOR(biome), warmingfactor, NA)
  setvar(core, NA, BETA(biome), beta, NA)
  invisible(core)
}

#' Create a new biome by splitting an existing one
#'
#' @param core Hector core
#' @param biome Name of new biome
#' @param from_biome
#' @param fveg_c
#' @param fdetritus_c
#' @param fsoil_c
#' @param fnpp_flux0
#' @param ...
#' @param veg_c0 Initial vegetation C pool (default = `0`)
#' @param detritus_c0 Initial detritus C pool (default = `0`)
#' @param soil_c0 Initial soil C pool (default = `0`)
#' @param npp_flux0 Initial net primary productivity (default = `0`)
#' @param warmingfactor Temperature multiplier (default = `1.0`)
#' @param beta CO2 fertilization effect (default = `0.36`)
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
    length(fveg_c) == length(new_biomes),
    length(fdetritus_c) == length(new_biomes),
    length(fsoil_c) == length(new_biomes),
    length(fnpp_flux0) == length(new_biomes),
    sum(fveg_c) == 1, all(fveg_c > 0),
    sum(fdetritus_c) == 1, all(fdetritus_c > 0),
    sum(fsoil_c) == 1, all(fsoil_c > 0),
    sum(fnpp_flux0) == 1, all(fnpp_flux0 > 0)
  )

  # `fetchvars` requires date to be between start and end date, so
  # we need to call the lower-level `sendmessage` method here.
  current_data_1 <- rbind.data.frame(
    sendmessage(core, GETDATA(), VEG_C(old_biome), 0, NA, ""),
    sendmessage(core, GETDATA(), DETRITUS_C(old_biome), 0, NA, ""),
    sendmessage(core, GETDATA(), SOIL_C(old_biome), 0, NA, "")
  )

  current_data_2 <- fetchvars(core, NA, c(NPP_FLUX0(old_biome),
                                          BETA(old_biome),
                                          WARMINGFACTOR(old_biome)))[, -1]
  current_data <- rbind.data.frame(current_data_1, current_data_2)
  current_values <- setNames(
    current_data[["value"]],
    gsub(paste0(old_biome, "."), "", current_data[["variable"]], fixed = TRUE)
  )

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

  c_delete_biome(core, old_biome)
  reset(core, 0)
  invisible(core)

}
