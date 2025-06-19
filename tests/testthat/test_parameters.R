context("Hector parameter changes")

# Make sure that changing parameters has the desired impact on Hector output.
inputdir <- system.file("input", package = "hector")
sampledir <- system.file("output", package = "hector")
testvars <- c(CONCENTRATIONS_CO2(), RF_TOTAL(), GLOBAL_TAS())

dates <- 1750:2100

# Limit the test dates to the future, where the historical
# variability won't impact the temp.
tdates <- 2000:2100

ssp245 <- file.path(inputdir, "hector_ssp245.ini")


test_that('All "fraction" parameters can be set and retrieved', {
  # Set up Hector core
  hc <- newcore(ssp245)

  # All of the fraction parameters.
  params <- tolower(c("F_NPPV", "F_NPPD", "F_LITTERD"))
  default_params <- fetchvars(hc, NA, tolower(params))
  new_values <- default_params$value * 0.5

  # Set up the Hector core with the new parameter values.
  mapply(function(p, v) {
    setvar(hc, dates = NA, var = p, values = v, unit = "(unitless)")
  }, p = params, v = new_values)
  reset(hc)
  run(hc)

  # Extract the parameters and make sure they match the values read in.
  out <- fetchvars(hc, NA, params)
  expect_equivalent(new_values, out$value)

  shutdown(hc)
})

test_that("Initial CO2 concentration equals preindustrial", {
  # Run default Hector RCP 45
  hc <- newcore(ssp245, suppresslogging = TRUE)
  run(hc, 1800)

  # Extract the inital atmosphere CO2 from the output.
  initcval <- fetchvars(hc, 1745, CONCENTRATIONS_CO2())

  # Extract the preindustrial value, this Hector is a parameter.
  preind <- fetchvars(hc, NA, PREINDUSTRIAL_CO2())

  # The should be equal.
  expect_equal(initcval$value, preind$value)

  # Reset the preindustiral CO2 parameter.
  setvar(hc, NA, PREINDUSTRIAL_CO2(), 285, "ppmv CO2")
  reset(hc)
  run(hc, 1800)

  # The new value parameter and initial CO2 concentration should
  # be the same.
  initcval_new <- fetchvars(hc, 1745, CONCENTRATIONS_CO2())
  preind_new <- fetchvars(hc, NA, PREINDUSTRIAL_CO2())
  expect_equal(initcval_new$value, preind_new$value)

  # Compare the new and old values with one another.
  expect_true(initcval$value != initcval_new$value)
  expect_true(preind$value != preind_new$value)
})

test_that("Lowering initial CO2 lowers output CO2", {
  # Define Hector core.
  hc <- newcore(ssp245, suppresslogging = TRUE)

  # Run and save results.
  run(hc, 2100)
  dd1 <- fetchvars(hc, dates, CONCENTRATIONS_CO2())

  ## Change the preindustrial CO2
  setvar(hc, NA, PREINDUSTRIAL_CO2(), 250, "ppmv CO2")
  reset(hc, 0.0)
  run(hc, 2100)
  dd2 <- fetchvars(hc, dates, CONCENTRATIONS_CO2())

  ## The concentrations should start off lower than the reference run by
  ## approximately the change in initial concentration, and the deficit
  ## increases with time.  We'll test for this approximately
  diff <- dd2$value - dd1$value # should all be negative
  expect_lt(max(diff), -26.0)

  shutdown(hc)
})


test_that("Lowering ECS lowers output Temperature", {
  # Define Hector core.
  hc <- newcore(ssp245, suppresslogging = TRUE)

  # Run and save results
  run(hc, 2100)
  dd1 <- fetchvars(hc, tdates, GLOBAL_TAS())

  # Decrease the ECS by half.
  default_ECS <- fetchvars(hc, NA, ECS())
  new_ECS <- default_ECS$value * 0.5

  ## make sure this still works with automatic reset.
  setvar(hc, NA, ECS(), new_ECS, getunits(ECS()))
  reset(hc, hc$reset_date)
  run(hc, 2100)
  dd2 <- fetchvars(hc, tdates, GLOBAL_TAS())

  ## Check that temperature is lower
  diff <- dd2$value - dd1$value
  expect_lt(max(diff), 0.0)

  shutdown(hc)
})

test_that("Raising Q10 increases CO2 concentration", {
  tdates <- 2050:2100

  # Define Hector core.
  hc <- newcore(ssp245, suppresslogging = TRUE)

  # Run and save results
  run(hc, 2100)
  vars <- c(CONCENTRATIONS_CO2(), GLOBAL_TAS())
  dd1 <- fetchvars(hc, tdates, vars)

  # Save the default Q10 value.
  default_q10 <- fetchvars(hc, NA, Q10_RH())
  new_q10 <- default_q10$value * 2

  # Set up the new core with the new value
  setvar(hc, NA, Q10_RH(), new_q10, NA)
  reset(hc, 0.0)
  run(hc, 2100)
  dd2 <- fetchvars(hc, tdates, vars)

  ## Check that concentration is higher across the board
  diff <- dd2$value - dd1$value
  expect_gt(min(diff), 0.0)

  shutdown(hc)
})

test_that("Lowering diffusivity increases temperature", {
  # Define Hector core.
  hc <- newcore(ssp245, suppresslogging = TRUE)
  run(hc, 2100)

  # Extract results from the default run.
  vars <- GLOBAL_TAS()
  dd1 <- fetchvars(hc, tdates, vars)

  # Extract and change the default value.
  default_kappa <- fetchvars(hc, NA, DIFFUSIVITY())
  new_kappa <- default_kappa$value * 0.5

  # Set up and run Hector with the new kappa (ocean heat diffusivity)
  setvar(hc, NA, DIFFUSIVITY(), new_kappa, default_kappa$units)
  reset(hc, hc$reset_date)
  run(hc, 2100)
  dd2 <- fetchvars(hc, tdates, vars)

  # Make sure the temperature is warmer when diffusivity decreases.
  diff <- dd2$value - dd1$value
  expect_gt(min(diff), 0.0)

  shutdown(hc)
})

test_that("Lowering aerosol forcing scaling factor increases temperature", {
  # Relevant vars to save and test.
  vars <- c(GLOBAL_TAS(), RF_BC(), RF_OC(), RF_SO2(), RF_NH3(), RF_ACI(), RF_TOTAL())

  # Define Hector core.
  hc <- newcore(ssp245, suppresslogging = TRUE)

  # Run and fetch data.
  run(hc, 2100)
  dd1 <- fetchvars(hc, tdates, vars)

  # Save and modify the default aerosol scalar forcing.
  default_alpha <- fetchvars(hc, NA, AERO_SCALE())
  new_alpha <- default_alpha$value * 0.5

  # Run with new alpha
  setvar(hc, NA, AERO_SCALE(), new_alpha, getunits(AERO_SCALE()))
  reset(hc, hc$reset_date)
  run(hc, 2100)
  dd2 <- fetchvars(hc, tdates, vars)

  # Check to make sure that the temp and RF have changed.

  # Checking that temperatures increase
  temp1 <- dd1[dd1$variable == GLOBAL_TAS(), ]
  temp2 <- dd2[dd2$variable == GLOBAL_TAS(), ]
  temp_diff <- temp2$value - temp1$value
  expect_gt(min(temp_diff), 0.0)

  # Checking that black carbon RF decreases
  bc1 <- dd1[dd1$variable == RF_BC(), ]
  bc2 <- dd2[dd2$variable == RF_BC(), ]
  bc_diff <- bc2$value - bc1$value
  expect_lt(max(bc_diff), 0.0)

  # Checking organic carbon RF magnitude decreases
  oc1 <- dd1[dd1$variable == RF_OC(), ]
  oc2 <- dd2[dd2$variable == RF_OC(), ]
  oc_diff <- abs(oc2$value) - abs(oc1$value)
  expect_lt(max(oc_diff), 0.0)

  # Checking that NH3 RF increases (becomes less negative)
  nh3_1 <- dd1[dd1$variable == RF_NH3(), ]
  nh3_2 <- dd2[dd2$variable == RF_NH3(), ]
  nh3_diff <- nh3_2$value - nh3_1$value
  expect_gt(min(nh3_diff), 0.0)

  # Checking that SO2 RF increases (becomes less negative)
  so2_1 <- dd1[dd1$variable == RF_SO2(), ]
  so2_2 <- dd2[dd2$variable == RF_SO2(), ]
  so2_diff <- so2_2$value - so2_1$value
  expect_gt(min(so2_diff), 0.0)

  # Checking that ACI RF increases (becomes less negative)
  aci1 <- dd1[dd1$variable == RF_ACI(), ]
  aci2 <- dd2[dd2$variable == RF_ACI(), ]
  aci_diff <- aci2$value - aci1$value
  expect_gt(min(aci_diff), 0.0)



  shutdown(hc)
})

test_that("Increasing volcanic forcing scaling factor increases the effect of volcanism", {
  ## Because the volcanic forcing scaling factor only has an impact during the
  ## the volcanic events. Only check the temp during those years.
  tdates <- c(1960, 1965)
  vars <- c(GLOBAL_TAS(), RF_VOL())

  # Set up and run Hector
  hc <- newcore(ssp245, suppresslogging = TRUE)
  run(hc, 1971)
  out <- fetchvars(hc, tdates, vars)

  # Run Hector with the new volcanic parameter.
  default_vol <- fetchvars(hc, NA, VOLCANIC_SCALE(), getunits(VOLCANIC_SCALE()))
  new_vol <- default_vol$value * 2
  setvar(hc, NA, VOLCANIC_SCALE(), new_vol, getunits(VOLCANIC_SCALE()))
  reset(hc, hc$reset_date)
  run(hc)
  new_out <- fetchvars(hc, tdates, vars)

  # Getting temperature and RF data
  temps <- out[out$variable == GLOBAL_TAS(), ]
  rfs <- out[out$variable == RF_VOL(), ]

  new_temps <- new_out[new_out$variable == GLOBAL_TAS(), ]
  new_rfs <- new_out[new_out$variable == RF_VOL(), ]

  expect_true(all(temps$value != new_temps$value))

  # Volcanic RF should change and the absolute difference
  # the volcanic forcing should be greater than 0.
  rf_diff <- abs(new_rfs$value - rfs$value)
  expect_true(all(rf_diff > 0.0))
})

test_that("Decreasing vegetation NPP fraction has down stream impacts", {
  # Define Hector core.
  hc <- newcore(ssp245, suppresslogging = TRUE)
  run(hc, 2100)

  # Extract results from the default run.
  vars <- c(GLOBAL_TAS(), CONCENTRATIONS_CO2())
  dd1 <- fetchvars(hc, tdates, vars)

  # Set up the Hector core with a lower NPP fraction.
  # More NPP to vegetation means less C to soil, where it decomposes.
  default_vals <- fetchvars(hc, NA, F_NPPV())
  new_val <- default_vals$value / 2
  setvar(hc, NA, F_NPPV(), new_val, NA)
  reset(hc, 0.0)
  run(hc, 2100)
  dd2 <- fetchvars(hc, tdates, vars)

  ## Check that CO2 concentration and temp is higher across the board
  diff <- dd2$value - dd1$value
  expect_gt(abs(min(diff)), 0.0)
  expect_true(all(abs(dd2$value - dd1$value) > 0))

  shutdown(hc)
})

test_that("Decreasing detritus NPP fraction has down stream impacts", {
  # Define Hector core.
  hc <- newcore(ssp245, suppresslogging = TRUE)
  run(hc, 2100)

  # Extract results from the default run.
  vars <- c(GLOBAL_TAS(), CONCENTRATIONS_CO2())
  dd1 <- fetchvars(hc, tdates, vars)

  # Change the fraction of NPP that contributes to detritus
  default_vals <- fetchvars(hc, NA, F_NPPD(), NA)
  new_val <- default_vals$value / 2
  setvar(hc, NA, F_NPPD(), new_val, NA)
  reset(hc, 0.0)
  run(hc, 2100)
  dd2 <- fetchvars(hc, tdates, vars)

  # Check that concentration is lower across the board because
  # Less detritus C means more soil C, which decomposes slower
  diff <- dd2$value - dd1$value
  expect_lt(min(diff), 0.0)
  expect_true(all(dd2$value <= dd1$value))

  shutdown(hc)
})

test_that("Decreasing litter flux to detritus has down stream impacts", {
  # Define Hector core.
  hc <- newcore(ssp245, suppresslogging = TRUE)
  run(hc, 2100)

  # Extract results from the default run.
  vars <- c(GLOBAL_TAS(), CONCENTRATIONS_CO2())
  dd1 <- fetchvars(hc, tdates, vars)

  # Change the litter fraction to detritus.
  default_vals <- fetchvars(hc, NA, F_LITTERD(), NA)
  new_val <- default_vals$value / 2
  setvar(hc, NA, F_LITTERD(), new_val, NA)
  reset(hc, 0.0)
  run(hc, 2100)
  dd2 <- fetchvars(hc, tdates, vars)

  # Check that concentration is lower across the board because
  # Less detritus C means more soil C, which decomposes slower
  diff <- dd2$value - dd1$value
  expect_lt(min(diff), 0.0)
  expect_true(all(dd2$value <= dd1$value))

  shutdown(hc)
})

test_that("Increasing CO2 fertilization factor increases NPP", {
  # Define Hector core.
  hc <- newcore(ssp245, suppresslogging = TRUE)
  run(hc, 2100)

  # Extract results from the default run.
  vars <- NPP()
  dd1 <- fetchvars(hc, tdates, vars)

  # Change the litter fraction to detritus.
  default_vals <- fetchvars(hc, NA, BETA(), NA)
  new_val <- default_vals$value * 2
  setvar(hc, NA, BETA(), new_val, NA)
  reset(hc, 0.0)
  run(hc, 2100)
  dd2 <- fetchvars(hc, tdates, vars)

  # More sensitive to atmospheric CO2 more NPP
  diff <- dd2$value - dd1$value
  expect_gt(min(diff), 0.0)

  shutdown(hc)
})

test_that("land ocean warming ratio", {
  # Check the land ocean warming ratio function
  expect_true(is.character(LO_WARMING_RATIO()))

  # Set up the Hector core & determine the dates & variables to keep.
  core <- newcore(ssp245)
  keep <- floor(seq(from = 1850, to = 2100, length.out = 30))
  vars <- c(GLOBAL_TAS(), LAND_TAS(), OCEAN_TAS(), SST())

  # The expected value for the lo warming ratio is 0, meaning that no user defined
  # land ocean warming ratio is being used. LO warming is an emergent property
  # from deoclim.
  defualt_lo <- fetchvars(core, NA, LO_WARMING_RATIO())
  expect_equal(defualt_lo$value, 0)

  run(core, max(keep))
  out1 <- fetchvars(core, keep, vars)

  # Check to make sure that when running default Hector that the land ocean warming ratio is not
  # held constant.
  land_temp_vals <- out1[out1$variable == LAND_TAS(), ][["value"]]
  ocean_temp_vals <- out1[out1$variable == OCEAN_TAS(), ][["value"]]
  emergent_ratio <- land_temp_vals / ocean_temp_vals
  expect_equal(length(unique(emergent_ratio)), length(emergent_ratio))

  # Reset the land ocean warming ratio. Make sure that a value can be passed into the core,
  # that is can be reset & have down stream effects.
  new_ratio <- 3
  setvar(core, NA, LO_WARMING_RATIO(), new_ratio, "(unitless)")
  reset(core)

  # Check to make sure the new land ocean warming ratio is read in.
  new_lo <- fetchvars(core, NA, LO_WARMING_RATIO())
  expect_true(defualt_lo$value != new_lo$value)
  expect_true(new_ratio == new_lo$value)

  # Run Hector with the new land ocean ratio, check the output.
  run(core, max(keep))
  out2 <- fetchvars(core, keep, vars)

  # Ensure ratio backed out of from Hector output equals user defined lo-ratio.
  land_temp_vals <- out2[out2$variable == LAND_TAS(), ][["value"]]
  ocean_temp_vals <- out2[out2$variable == OCEAN_TAS(), ][["value"]]
  ratio_from_output <- land_temp_vals / ocean_temp_vals
  expect_true(all(abs(new_ratio - unique(ratio_from_output)) <= 1e-5))
  expect_equal(length(unique(round(ratio_from_output, digits = 3))), 1)


  # Make sure that the change in the global mean temp is relatively small.
  out1_global_vals <- out1[out1$variable == GLOBAL_TAS(), ][["value"]]
  out2_global_vals <- out2[out2$variable == GLOBAL_TAS(), ][["value"]]
  tas_diff <- mean(abs(out1_global_vals - out2_global_vals))
  expect_lt(tas_diff, 1e-1)

  out1_land_vals <- out1[out1$variable == LAND_TAS(), ][["value"]]
  out2_land_vals <- out2[out2$variable == LAND_TAS(), ][["value"]]
  land_diff <- mean(abs(out1_land_vals - out2_land_vals))
  expect_gt(land_diff, 1e-1)

  out1_ocean_vals <- out1[out1$variable == OCEAN_TAS(), ][["value"]]
  out2_ocean_vals <- out2[out2$variable == OCEAN_TAS(), ][["value"]]
  ocean_diff <- mean(abs(out1_ocean_vals - out2_ocean_vals))
  expect_gt(ocean_diff, 1e-1)
})
