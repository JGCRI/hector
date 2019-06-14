#' Enable biomes
#'
#' @param first_biome
#' @return 
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
