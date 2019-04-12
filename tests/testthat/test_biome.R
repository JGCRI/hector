context("Running Hector with multiple biomes")

test_that("Hector runs with multiple biomes.", {

  quickrun <- function(name, varlist = list()) {
    inifile <- system.file("input", "hector_rcp45.ini", package = "hector")
    hc <- newcore(inifile, suppresslogging = TRUE, name = name)
    for (v in varlist) {
      tryCatch({
        with(v, setvar(hc, NA, variable, value, unit))
      },
      error = function(e) stop("Caught error with variable: ",
                               v[["variable"]], ":\n",
                               conditionMessage(e))
      )
    }
    invisible(run(hc, 2100))
    dates <- seq(2000, 2100)
    vars <- c(
      ATMOSPHERIC_CO2(),
      RF_TOTAL(),
      GLOBAL_TEMP()
    )
    fetchvars(hc, dates, vars)
  }
  rcp45_result <- quickrun("default")

  biome <- list(
    list(variable = VEGC(), value = 450, unit = "PgC"),
    list(variable = VEGC("boreal"), value = 100, unit = "PgC"),
    list(variable = DETRITUSC(), value = 45, unit = "PgC"),
    list(variable = DETRITUSC("boreal"), value = 10, unit = "PgC"),
    list(variable = SOILC(), value = 582, unit = "PgC"),
    list(variable = SOILC("boreal"), value = 1200, unit = "PgC"),
    list(variable = NPP_FLUX0(), value = 45, unit = "PgC Yr"),
    list(variable = NPP_FLUX0("boreal"), value = 5, unit = "PgC Yr"),
    list(variable = BETA(), value = 0.36, unit = "(unitless)"),
    list(variable = BETA("boreal"), value = 0.36, unit = "(unitless)"),
    list(variable = Q10_RH(), value = 2, unit = "(unitless)"),
    list(variable = Q10_RH("boreal"), value = 2, unit = "(unitless)")
  )
  biome_result <- quickrun("biome", biome)

  result_diff <- rcp45_result$value - biome_result$value
  diff_summary <- tapply(result_diff, rcp45_result$variable, sum)
  expect_true(all(abs(diff_summary) > 0))

  # Add the warming tag
  warm_biome <- c(biome, list(
    list(variable = WARMINGFACTOR(), value = 1.0, unit = "(unitless)"),
    list(variable = WARMINGFACTOR("boreal"), value = 2.5, unit = "(unitless)")
  ))

  warm_biome_result <- quickrun("warm_biome", warm_biome)
  default_tgav <- rcp45_result[rcp45_result[["variable"]] == "Tgav",
                               "value"]
  warm_tgav <- warm_biome_result[warm_biome_result[["variable"]] == "Tgav",
                                 "value"]
  expect_true(mean(default_tgav) < mean(warm_tgav))
})
