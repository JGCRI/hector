context("Running Hector with multiple biomes")

test_that("Hector runs with multiple biomes.", {

  skip_if_not_installed("hectortools")

  quickrun <- function(ini_list, name) {
    ini_file <- tempfile()
    on.exit(file.remove(ini_file), add = TRUE)
    hectortools::write_ini(ini_list, ini_file)
    core <- newcore(ini_file, name = name, suppresslogging = TRUE)
    invisible(run(core))
    on.exit(shutdown(core), add = TRUE)
    dates <- seq(2000, 2100)
    vars <- c(
      ATMOSPHERIC_CO2(),
      RF_TOTAL(),
      GLOBAL_TEMP()
    )
    fetchvars(core, dates, vars)
  }

  rcp45_file <- system.file("input", "hector_rcp45.ini", package = "hector")
  rcp45 <- hectortools::read_ini(rcp45_file)
  rcp45_result <- quickrun(rcp45, "default")

  # Set biome-specific variables
  biome <- modifyList(rcp45, list(simpleNbox = list(
    veg_c = NULL,
    boreal.veg_c = 100,
    tropical.veg_c = 450,
    detritus_c = NULL,
    boreal.detritus_c = 15,
    tropical.detritus_c = 45,
    soil_c = NULL,
    boreal.soil_c = 1200,
    tropical.soil_c = 578,
    npp_flux0 = NULL,
    boreal.npp_flux0 = 5.0,
    tropical.npp_flux0 = 45.0,
    beta = NULL,
    boreal.beta = 0.36,
    tropical.beta = 0.36,
    q10_rh = NULL,
    boreal.q10_rh = 2.0,
    tropical.q10_rh = 2.0
  )))
  biome_result <- quickrun(biome, "biome")

  result_diff <- rcp45_result$value - biome_result$value
  diff_summary <- tapply(result_diff, rcp45_result$variable, sum)
  expect_true(all(abs(diff_summary) > 0))

  # Add the warming tag
  warm_biome <- modifyList(biome, list(simpleNbox = list(
    boreal.warmingfactor = 2.5,
    tropical.warmingfactor = 1.0
  )))
  warm_biome_result <- quickrun(warm_biome, "warm_biome")
  default_tgav <- rcp45_result[rcp45_result[["variable"]] == "Tgav",
                               "value"]
  warm_tgav <- warm_biome_result[warm_biome_result[["variable"]] == "Tgav",
                                 "value"]
  expect_true(mean(default_tgav) < mean(warm_tgav))
})
