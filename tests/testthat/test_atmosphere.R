context("Atmosphere and radiative forcing")
# Check some basic science to make sure Hector's forcing & temperature componet behave as expectd.

inputdir <- system.file("input", package = "hector")
sampledir <- system.file("output", package = "hector")
ssp245 <- file.path(inputdir, "hector_ssp245.ini")

# Our defined allowable error, when comparing two values we expect to be
# equal differences less than this are assumed to be related to round off error.
error_threshold <- 1e-8

test_that("Checking RF values", {

  # Define the comparison dates
  t_dates <- 1850:2100

  # Set and run Hector
  hc <- newcore(ssp245)
  run(hc, max(t_dates))

  # Now check to make sure that  the total radiative forcing is equal to the
  # sum of the different radiative forcing agents.
  # Save a copy of the total radiative forcing
  total_rf <- fetchvars(hc, dates = t_dates, vars = RF_TOTAL())

  # Extract all of the other RF values and calculate the total value.
  rf_list <- c(
    RF_ALBEDO(), RF_CO2(), RF_N2O(), RF_H2O_STRAT(), RF_O3_TROP(), RF_BC(),
    RF_OC(), RF_SO2(), RF_VOL(), RF_CH4(), RF_CF4(), RF_C2F6(),
    RF_HFC23(), RF_HFC32(), RF_HFC4310(), RF_HFC125(), RF_HFC134A(),
    RF_HFC143A(), RF_HFC227EA(), RF_HFC245FA(), RF_SF6(), RF_CFC11(),
    RF_CFC12(), RF_CFC113(), RF_CFC114(), RF_CFC115(), RF_CCL4(), RF_CH3CCL3(),
    RF_HCFC22(), RF_HCFC141B(), RF_HCFC142B(), RF_HALON1211(), RF_HALON1301(),
    RF_HALON2402(), RF_CH3CL(), RF_CH3BR(), RF_NH3(), RF_ACI(), RF_MISC()
  )

  # Calculate the total RF value from the sum of the individual RF values.
  individual_rf <- fetchvars(hc, dates = t_dates, vars = rf_list)
  sum_individuals <- aggregate(value ~ scenario + year + units,
    data = individual_rf,
    FUN = "sum"
  )

  expect_equal(total_rf$value, sum_individuals$value, tolerance = error_threshold)
  })

test_that("Check Temp", {

    # Define the comparison dates
    t_dates <- 2020:2100

    # Set and run Hector
    hc <- newcore(ssp245)
    run(hc, max(t_dates))

    # Make sure that the weighted sum of the land and ocean air temps is equal to the
    # global mean air temp. This is useful for preserving behavior of the temp component.
    land  <- fetchvars(hc, t_dates, vars = LAND_TAS())[["value"]]
    ocean <- fetchvars(hc, t_dates, vars = OCEAN_TAS())[["value"]]

    flnd <- 0.29
    weighted_sum <- flnd * land + ocean * (1 - flnd)
    global_temp_values <- fetchvars(hc, t_dates, vars = GLOBAL_TAS())[["value"]]
    expect_equal(global_temp_values, weighted_sum, tolerance = 1e-5)

    #  future global air temperature should be larger than surface temperature
    gmst <- fetchvars(hc, t_dates, vars = GMST())[["value"]]
    expect_true(all(global_temp_values >  gmst))

})
