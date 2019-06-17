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
                         warmingfactor = 0,
                         beta = 0.36) {
  c_create_biome(core, biome)
  setvar(core, NA, VEG_C(biome), veg_c0)
  setvar(core, NA, DETRITUS_c(biome), detritus_c0)
  setvar(core, NA, SOIL_C(biome), soil_c0)
  setvar(core, NA, NPP_FLUX0(biome), detritus_c0)
  setvar(core, NA, WARMINGFACTOR(biome), warmingfactor)
  setvar(core, NA, BETA(biome), beta)
  reset(core, 0)
  invisible(core)
}

#' Create a new biome by splitting an existing one
split_biome <- function(core, biome,
                        from_biome = get_biome_list(core)[[1]],
                        fveg_c = 0.5,
                        fdetritus_c = fveg_c,
                        fsoil_c = fveg_c,
                        fnpp_flux0 = f_vegc,
                        beta = NULL,
                        warmingfactor = NULL) {
  current_data <- rbind.data.frame(
    fetchvars(core, NA, VEG_C(from_biome)),
    fetchvars(core, NA, DETRITUS_C(from_biome)),
    fetchvars(core, NA, SOIL_C(from_biome)),
    fetchvars(core, NA, NPP_FLUX0(from_biome)),
    fetchvars(core, NA, BETA(from_biome)),
    fetchvars(core, NA, WARMINGFACTOR(from_biome))
  )
  create_biome(core, biome)
}
