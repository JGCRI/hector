context("Hector running with constraints")

ssp245 <- function() newcore(system.file("input", "hector_ssp245.ini", package = "hector"))

test_that("Concentration-forced runs work for halocarbons", {

  # Note this test assumes that if the constraint works for a single HCF it will work
  # for all of Hector's HFCs. But in the "concentration driven runs via INI file" test
  # we check all of the different halocarbons.
  # 1. Set up Hector to use HFC23 concentrations as a constraint to make sure
  #       that the core can read in HFC constraints and produce identical results
  #       to test the functionality of the core.
  # 2. Perterb the concentrations to test that applying the constraint has downstream
  #       consequences, impacts global mean temperature.


  # Run emission driven Hector
  hc <- ssp245()
  invisible(run(hc))

  # Save the dates and output variables to test.
  dates <- seq(startdate(hc), getdate(hc))
  outvars <- c(GLOBAL_TAS(), "HFC23_concentration", EMISSIONS_HFC23())

  # Extract the emission driven results, saving a copy of the HFC23
  # concentrations and emission separately to manipulate.
  emissOut <- fetchvars(hc, dates, outvars)
  emissOut_HFCconc <- subset(emissOut, variable == "HFC23_concentration")
  emissOut_HFCemiss <- subset(emissOut, variable == EMISSIONS_HFC23())

  # Set up and run Hector with constrained HFC concentrations.
  setvar(hc,
         emissOut_HFCconc$year, HFC23_CONSTRAIN(),
         emissOut_HFCconc$value,
         getunits(HFC23_CONSTRAIN())
  )
  invisible(reset(hc))
  invisible(run(hc))
  conOut <- fetchvars(hc, dates, outvars)

  # Since both runs should have the same HFC concentrations the results should be the same.
  expect_equivalent(emissOut$value, conOut$value, tol = 1e-10)

  # Now change the HFC concentrations and check for downstream changes.
  new_HFC_con <- emissOut_HFCconc$value * 3
  setvar(hc, dates, HFC23_CONSTRAIN(), new_HFC_con, getunits(HFC23_CONSTRAIN()))
  invisible(reset(hc))
  invisible(run(hc))

  # The HCF concentrations should equal the constraint read in.
  expect_equal(fetchvars(hc, dates, "HFC23_concentration")$value, new_HFC_con)

  # Compare the outputs from the emission driven and the perturbed
  # concentration driven runs. Since the HFC concentration was increased and
  # HFCs have a warming effect the new values should be larger than the original
  # concentration driven runs.
  new_out <- fetchvars(hc, dates, outvars)
  expect_true(all(new_out$value >= conOut$value))
})

test_that("Concentration-forced runs work for CH4", {

  # Run emission driven Hector
  hc <- ssp245()
  invisible(run(hc))

  # Save the dates and output variables to test.
  dates <- seq(startdate(hc), getdate(hc))
  outvars <- c(GLOBAL_TAS(), CONCENTRATIONS_CH4(), EMISSIONS_CH4(), RF_CH4())

  # Extract the emission driven results, saving a copy of the CH4
  # concentrations and emission separately to manipulate.
  emissOut <- fetchvars(hc, dates, outvars)
  emissOut_CH4conc <- subset(emissOut, variable == CONCENTRATIONS_CH4())
  emissOut_CH4emiss <- subset(emissOut, variable == EMISSIONS_CH4())

  # Set up and run Hector with constrained CH4 concentrations.
  setvar(hc,
         emissOut_CH4conc$year,
         CH4_CONSTRAIN(),
         emissOut_CH4conc$value,
         getunits(CH4_CONSTRAIN()))
  invisible(reset(hc))
  invisible(run(hc))
  conOut <- fetchvars(hc, dates, outvars)

  # Since both runs should have the same CH4 concentrations the results should be the same.
  expect_equivalent(emissOut$value, conOut$value, tol = 1e-10)

  # Now change the CH4 concentrations and check for downstream changes.
  new_CH4_con <- emissOut_CH4conc$value * 3
  setvar(hc, dates, CH4_CONSTRAIN(), new_CH4_con, getunits(CH4_CONSTRAIN()))
  invisible(reset(hc))
  invisible(run(hc))

  # The CH4 concentrations should equal the constraint read in.
  expect_equal(fetchvars(hc, dates, CONCENTRATIONS_CH4())$value, new_CH4_con)

  # Compare the outputs from the emission driven and the perturbed
  # concentration driven runs. Since the CH4 concentration was increased and
  # CH4 has a warming effect the new values should be larger than the original
  # concentration driven runs.
  new_out <- fetchvars(hc, 2000:2100, outvars)
  to_compare <- subset(conOut, year %in% 2000:2100)
  names(new_out) <- c("scenario", "year", "variable", "new_value", "units")

  expect_true(all(new_out$value >= to_compare$value))
})

test_that("Concentration-forced runs work for N2O", {

  # Run emission driven Hector
  hc <- ssp245()
  invisible(run(hc))

  # Save the dates and output variables to test.
  dates <- seq(startdate(hc), getdate(hc))
  outvars <- c(GLOBAL_TAS(), CONCENTRATIONS_N2O(), EMISSIONS_N2O(), RF_N2O())

  # Extract the emission driven results, saving a copy of the N2O
  # concentrations and emission separately to manipulate.
  emissOut <- fetchvars(hc, dates, outvars)
  emissOut_N2Oconc <- subset(emissOut, variable == CONCENTRATIONS_N2O())
  emissOut_N2Oemiss <- subset(emissOut, variable == EMISSIONS_N2O())

  # Set up and run Hector with constrained N2O concentrations.
  setvar(hc,
         emissOut_N2Oconc$year,
         N2O_CONSTRAIN(),
         emissOut_N2Oconc$value,
         getunits(N2O_CONSTRAIN()))
  invisible(reset(hc))
  invisible(run(hc))
  conOut <- fetchvars(hc, dates, outvars)

  # Since both runs should have the same N2O concentrations the results should be the same.
  expect_equivalent(emissOut$value, conOut$value, tol = 1e-10)

  # Now change the N2O concentrations and check for downstream changes.
  new_N2O_con <- emissOut_N2Oconc$value * 3
  setvar(hc, dates, N2O_CONSTRAIN(), new_N2O_con, getunits(N2O_CONSTRAIN()))
  invisible(reset(hc))
  invisible(run(hc))

  # The N2O concentrations should equal the constrainnt read in.
  expect_equal(fetchvars(hc, dates, CONCENTRATIONS_N2O())$value, new_N2O_con)

  # Compare the outputs from the emission driven and the perturbed
  # concentration driven runs. Since the N2O concentration was increased and
  # N2O has a warming effect the new values should be larger than the original
  # concentration driven runs.
  new_out <- fetchvars(hc, 2000:2100, outvars)
  to_compare <- subset(conOut, year %in% 2000:2100)
  expect_true(all(new_out$value >= to_compare$value))
})

test_that("Atmospheric CO2 concentrations can be constrained", {

  # Variables and years to save.
  years <- 1850:2100
  vars <- c(CONCENTRATIONS_CO2(), GLOBAL_TAS(), RF_TOTAL(), RF_CO2())

  # Instantiate a Hector core and save results.
  hc <- ssp245()
  run(hc)
  ssp245_out <- fetchvars(hc, years, vars)
  ssp245_conc <- subset(ssp245_out, variable == CONCENTRATIONS_CO2())

  constrained_values <- ssp245_conc$value * 3
  setvar(hc, years, CO2_CONSTRAIN(), constrained_values, getunits(CO2_CONSTRAIN()))
  reset(hc)
  run(hc)

  # Make sure that the atmospheric CO2 matches the constraint.
  hico2_out <- fetchvars(hc, years, vars)
  hico2_conc <- subset(hico2_out, variable == CONCENTRATIONS_CO2())
  expect_equal(hico2_conc$value, constrained_values)

  # The run with the higher CO2 should be warmer, with a higher total and CO2 RF.
  expect_true(all(hico2_out$value >= ssp245_out$value))

  # Now test what happens when the CO2 concentrations are decreased
  constrained_values <- ssp245_conc$value * 0.5
  setvar(hc, years, CO2_CONSTRAIN(), constrained_values, getunits(CO2_CONSTRAIN()))
  reset(hc)
  run(hc)

  # Extract the results from the low CO2 run
  lowco2_out <- fetchvars(hc, years, vars)
  lowco2_conc <- subset(lowco2_out, variable == CONCENTRATIONS_CO2())
  expect_equal(lowco2_conc$value, constrained_values)

  # The run with the higher CO2 should be warmer, with a higher total and CO2 RF.
  expect_true(all(lowco2_out$value <= ssp245_out$value))
})

test_that("Discontinuous constraint works", {

  # Set up a Hector core.
  hc <- ssp245()
  all_years <- seq(startdate(hc), enddate(hc))

  # Create two CO2 constraints for two different time chunks, since
  # the purpose of this test is to make sure that the constraints work
  # if they are discontinuous so these two chunks MUST be discontinuous.
  ca_years_1 <- 1850:1860
  ca_vals_1 <- rep(278, length(ca_years_1))
  ca_years_2 <- 1870:1880
  ca_vals_2 <- rep(298, length(ca_years_2))

  # Set up the Hector core with the constraints.
  setvar(hc, ca_years_1, CO2_CONSTRAIN(), ca_vals_1, getunits(CO2_CONSTRAIN()))
  reset(hc)
  setvar(hc, ca_years_2, CO2_CONSTRAIN(), ca_vals_2, getunits(CO2_CONSTRAIN()))
  reset(hc)

  # Reset and run the core.
  reset(hc)
  invisible(run(hc))

  # Make sure we can retrieve all these values.
  out_rf <- fetchvars(hc, all_years, RF_CO2())
  out_ca <- fetchvars(hc, all_years, CONCENTRATIONS_CO2())

  # RFs and concentrations during the constraint period are constant,
  # they should be since we read in constant CO2 concentrations.
  # Assume that if the year to year change is less than the threshold
  # then the constant CO2 concentrations were held true.
  threshold <- 1e-5
  expect_true(all(abs(diff(out_rf[out_rf$year %in% ca_years_1, "value"])) <= threshold))
  expect_true(all(abs(diff(out_rf[out_rf$year %in% ca_years_2, "value"])) <= threshold))
  expect_equal(out_ca[out_ca$year %in% ca_years_1, "value"], ca_vals_1)
  expect_equal(out_ca[out_ca$year %in% ca_years_2, "value"], ca_vals_2)

  # Because we only read in CO2 concentrations over a short periods of time outside of the
  # first and second constrains the scenario should follow the RCP 4.5 trajectory increasing temp.
  expect_true(out_rf[out_ca$year == 2000, "value"] > out_rf[out_ca$year == 1900, "value"])
  expect_true(out_ca[out_ca$year == 2000, "value"] > out_ca[out_ca$year == 1900, "value"])

  # Make sure that the CO2 concentration matches the constraints read in during the two
  # different constraint periods.
  expect_equal(fetchvars(hc, min(ca_years_1), CONCENTRATIONS_CO2())$value, ca_vals_1[1])
  expect_equal(fetchvars(hc, max(ca_years_1), CONCENTRATIONS_CO2())$value, ca_vals_1[1])
  expect_equal(fetchvars(hc, min(ca_years_2), CONCENTRATIONS_CO2())$value, ca_vals_2[1])
  expect_equal(fetchvars(hc, max(ca_years_2), CONCENTRATIONS_CO2())$value, ca_vals_2[1])

  # You should NOT be able to retrieve constraints before, between, or after
  # user-specified values.
  baddates <- c(min(ca_years_1) - 1, max(ca_years_1) + 1, max(ca_years_2) + 1)
  ca_before <- fetchvars(hc, baddates, CO2_CONSTRAIN())
  expect_true(all(is.na(ca_before$value)))
  expect_true(all(!is.nan(ca_before$value)))
})

test_that("tas constraint works", {
    t2000 <- 2.0
    hc <- ssp245()
    setvar(hc, 2000, TAS_CONSTRAIN(), t2000, getunits(TAS_CONSTRAIN()))
    invisible(run(hc))

    # constraint returns NA for non-set years
    constraint_otheryear <- fetchvars(hc, 1999, vars = TAS_CONSTRAIN())
    expect_true(is.na(constraint_otheryear$value))
    # constraint returns correct value for set year
    constraint_year <- fetchvars(hc, 2000, vars = TAS_CONSTRAIN())
    expect_equal(constraint_year$value, t2000)

    x <- fetchvars(hc, 1999:2001, vars = GLOBAL_TAS())

    expect_lt(x$value[1], t2000)    # global tas should be nowhere near 2C the year before,
    expect_equal(x$value[2], t2000) # identical to the constraint in 2000,
    expect_lt(x$value[3], t2000)    # and again lower the year after
})

test_that("NBP constraint works", {
    yr <- 2000
    nbp2000 <- 1.0
    hc <- ssp245()
    setvar(hc, 2000, NBP_CONSTRAIN(), nbp2000, getunits(NBP_CONSTRAIN()))
    invisible(run(hc))

    # constraint returns NA for non-set years
    constraint_otheryear <- fetchvars(hc, 1999, vars = NBP_CONSTRAIN())
    expect_true(is.na(constraint_otheryear$value))
    # constraint returns correct value for set year
    constraint_year <- fetchvars(hc, 2000, vars = NBP_CONSTRAIN())
    expect_equal(constraint_year$value, nbp2000)

    x <- fetchvars(hc, 1999:2001, vars = NBP())

    expect_gt(x$value[1], nbp2000)    # NBP should be higher than constraint the year before,
    expect_equal(x$value[2], nbp2000) # identical to the constraint in 2000,
    expect_gt(x$value[3], nbp2000)    # and again higher the year after
})
