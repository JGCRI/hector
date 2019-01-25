context("Running Hector with multiple biomes")

test_that("Hector runs with multiple biomes.", {

  skip_if_not_installed("hectortools")

  vars <- c(
    ATMOSPHERIC_CO2(),
    RF_TOTAL(),
    GLOBAL_TEMP()
  )

  dates <- seq(2000, 2100)
  rcp45_file <- system.file("input", "hector_rcp45.ini", package = "hector")
  rcp45 <- hectortools::read_ini(rcp45_file)
  rcp45_core <- newcore(rcp45_file, name = "default")
  invisible(run(rcp45_core))
  rcp45_result <- fetchvars(rcp45_core, dates, vars)
  invisible(shutdown(rcp45_core))

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
  biome_file <- tempfile()
  hectortools::write_ini(biome, biome_file)
  teardown(file.remove(biome_file))
  core <- newcore(biome_file, name = "biome")
  invisible(run(core))
  result <- fetchvars(core, dates, vars = vars)
  invisible(shutdown(core))

  result_diff <- rcp45_result$value - result$value
  diff_summary <- tapply(result_diff, rcp45_result$variable, sum)
  expect_true(all(abs(diff_summary) > 0))
})
