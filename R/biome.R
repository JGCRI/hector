#' Enable biomes
#'
#' @param first_biome (Character) Name of first biome. Default is `"biome1"`.
#' @return Hector core, invisibly
#' @author Alexey Shiklomanov
#' @export
use_biomes <- function(core, first_biome = "biome1") {
  biome_list <- get_biome_list(core)
  # No-op if this has multiple biomes, or any of the biomes are not "global"
  if (!(length(biome_list) == 1 && biome_list == "global")) {
    message("Core is already using biomes.")
    invisible(return(core))
  }

  out <- c_rename_biome(core, "global", first_biome)
  invisible(out)
}

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
split_biome <- function(core, biome,
                        from_biome = get_biome_list(core)[[1]],
                        fveg_c = 0.5,
                        fdetritus_c = fveg_c,
                        fsoil_c = fveg_c,
                        fnpp_flux0 = fveg_c,
                        ...) {

  # `fetchvars` requires date to be between start and end date, so
  # we need to call the lower-level `sendmessage` method here.
  current_data_1 <- rbind.data.frame(
    sendmessage(core, GETDATA(), VEG_C(from_biome), 0, NA, ""),
    sendmessage(core, GETDATA(), DETRITUS_C(from_biome), 0, NA, ""),
    sendmessage(core, GETDATA(), SOIL_C(from_biome), 0, NA, "")
  )
  
  current_data_2 <- rbind.data.frame(
    fetchvars(core, NA, NPP_FLUX0(from_biome)),
    fetchvars(core, NA, BETA(from_biome)),
    fetchvars(core, NA, WARMINGFACTOR(from_biome))
  )[, -1] # Drop the "scenario" column because it's not returned by `sendmessage`
  current_data <- rbind.data.frame(current_data_1, current_data_2)
  fracs <- c("veg_c" = fveg_c,
             "detritus_c" = fdetritus_c,
             "soil_c" = fsoil_c,
             "npp_flux0" = fnpp_flux0)
  vals <- setNames(
    current_data[["value"]],
    gsub(paste0(from_biome, "."), "", current_data[["variable"]], fixed = TRUE)
  )

  create_biome(
    core = core,
    biome = biome,
    veg_c0 = vals[["veg_c"]] * fveg_c,
    detritus_c0 = vals[["detritus_c"]] * fdetritus_c,
    soil_c0 = vals[["soil_c"]] * fsoil_c,
    npp_flux0 = vals[["npp_flux0"]] * fnpp_flux0,
    ...
  )

  setvar(core, 0, VEG_C(from_biome),
         vals[["veg_c"]] * (1 - fveg_c), "PgC")
  setvar(core, 0, DETRITUS_C(from_biome),
         vals[["detritus_c"]] * (1 - fdetritus_c), "PgC")
  setvar(core, 0, SOIL_C(from_biome),
         vals[["soil_c"]] * (1 - fsoil_c), "PgC")
  setvar(core, NA, NPP_FLUX0(from_biome),
         vals[["npp_flux0"]] * (1 - fnpp_flux0), "PgC/yr")

  reset(core, 0)

  invisible(core)

}
