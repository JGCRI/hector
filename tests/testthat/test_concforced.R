context("Run concentration forced Hector")

rcp45 <- function() newcore(system.file("input", "hector_rcp45.ini", package = "hector"))

test_that("Concentration-forced runs work for halocarbons", {

  # Note this test assumes that if the constraint works for a single HCF it will work
  # for all of Hector's HFCs. But in the "concentration driven runs via INI file" test
  # we check all of the different halocarbons.
  # 1. Set up Hector to use HFC23 concentrations as a constraint to make sure
  #       that the core can read in HFC constraints and produce identical restults
  #       to test the functionality of the core.
  # 2. Perterb the concentrations to test that applying the constraint has downstream
  #       consequences, impacts global mean temperature.


  # Run emission driven Hector
  hc <- rcp45()
  invisible(run(hc))

  # Save the dates and output variables to test.
  dates <- seq(startdate(hc), getdate(hc))
  outvars <- c(GLOBAL_TEMP(), "HFC23_concentration", EMISSIONS_HFC23())

  # Extract the emission driven results, saving a copy of the HFC23
  # concentrations and emission seperately to manipulate.
  emissOut <- fetchvars(hc, dates, outvars)
  emissOut_HFCconc <- subset(emissOut, variable == "HFC23_concentration")
  emissOut_HFCemiss <- subset(emissOut, variable == EMISSIONS_HFC23())

  # Set up and run Hector with constrained HFC concentrations.
  setvar(hc,
         emissOut_HFCconc$year, HFC23_CONSTRAIN(),
         emissOut_HFCconc$value,
         getunits(HFC23_CONSTRAIN()))
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

  # The HCF concentrations should equal the constrainnt read in.
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
  hc <- rcp45()
  invisible(run(hc))

  # Save the dates and output variables to test.
  dates <- seq(startdate(hc), getdate(hc))
  outvars <- c(GLOBAL_TEMP(), ATMOSPHERIC_CH4(), EMISSIONS_CH4())

  # Extract the emission driven results, saving a copy of the CH4
  # concentrations and emission seperately to manipulate.
  emissOut <- fetchvars(hc, dates, outvars)
  emissOut_CH4conc <- subset(emissOut, variable == ATMOSPHERIC_CH4())
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

  # The CH4 concentrations should equal the constrainnt read in.
  expect_equal(fetchvars(hc, dates, ATMOSPHERIC_CH4())$value, new_CH4_con)

  # Compare the outputs from the emission driven and the perturbed
  # concentration driven runs. Since the CH4 concentration was increased and
  # CH4 has a warming effect the new values should be larger than the original
  # concentration driven runs.
  new_out <- fetchvars(hc, 2000:2100, outvars)
  to_compare <- subset(conOut, year %in% 2000:2100)
  expect_true(all(new_out$value >= to_compare$value))
})

test_that("Concentration-forced runs work for N2O", {

  # Run emission driven Hector
  hc <- rcp45()
  invisible(run(hc))

  # Save the dates and output variables to test.
  dates <- seq(startdate(hc), getdate(hc))
  outvars <- c(GLOBAL_TEMP(), ATMOSPHERIC_N2O(), EMISSIONS_N2O())

  # Extract the emission driven results, saving a copy of the N2O
  # concentrations and emission seperately to manipulate.
  emissOut <- fetchvars(hc, dates, outvars)
  emissOut_N2Oconc <- subset(emissOut, variable == ATMOSPHERIC_N2O())
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
  expect_equal(fetchvars(hc, dates, ATMOSPHERIC_N2O())$value, new_N2O_con)

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
  vars <- c(ATMOSPHERIC_CO2(), GLOBAL_TEMP(), RF_TOTAL(), RF_CO2())

  # Initate a Hector core and save results.
  hc <- rcp45()
  run(hc)
  rcp45_out <- fetchvars(hc, years, vars)
  rcp45_conc <- subset(rcp45_out, variable == ATMOSPHERIC_CO2())

  constrained_values <- rcp45_conc$value * 3
  setvar(hc, years, CO2_CONSTRAIN(), constrained_values, getunits(CO2_CONSTRAIN()))
  reset(hc)
  run(hc)

  # Make sure that the atmospheric CO2 maches the constraint.
  hico2_out <- fetchvars(hc, years, vars)
  hico2_conc <- subset(hico2_out, variable == ATMOSPHERIC_CO2())
  expect_equal(hico2_conc$value, constrained_values)

  # The run with the higher CO2 should be warmer, with a higher total and CO2 RF.
  expect_true(all(hico2_out$value >= rcp45_out$value))

  # Now test what happens when the CO2 concentrations are decreased
  constrained_values <- rcp45_conc$value * 0.5
  setvar(hc, years, CO2_CONSTRAIN(), constrained_values, getunits(CO2_CONSTRAIN()))
  reset(hc)
  run(hc)

  # Extract the results from the low CO2 run
  lowco2_out <- fetchvars(hc, years, vars)
  lowco2_conc <- subset(lowco2_out, variable == ATMOSPHERIC_CO2())
  expect_equal(lowco2_conc$value, constrained_values)

  # The run with the higher CO2 should be warmer, with a higher total and CO2 RF.
  expect_true(all(lowco2_out$value <= rcp45_out$value))
})

# Instead of using the R wrapper to set up the concetnration driven runs
# manipulate the underlying ini file. This tests to make sure the underly C++
# code works.
test_that("Concentration driven runs via INI file works", {

  # Read in the ini file and find all of the lines in the ini file that are commented out
  # they will all start with ";"
  ini_txt_all <- readLines(system.file("input", "hector_rcp45.ini", package = "hector"))
  ini_txt <- grep("^ *;", ini_txt_all, value = TRUE, invert = TRUE)

  # Parese out all of the types of the halocarbons.
  rxp <- "\\[([[:alnum:]]+)_halocarbon\\]"
  halocarbs <- gsub(rxp, "\\1", grep(rxp, ini_txt, value = TRUE))

  # A vector of all the concentrations to check.
  outvars <- c(
    ATMOSPHERIC_CO2(),
    ATMOSPHERIC_CH4(),
    ATMOSPHERIC_N2O(),
    paste0(halocarbs, "_concentration")
  )

  # Run Hector and save all of the concentrations.
  hc <- rcp45()
  invisible(run(hc))
  yrs <- seq(startdate(hc), enddate(hc))
  results <- fetchvars(hc, yrs, outvars)

  # Format the data the data frame concentrations into a format that can be used as Hector input.
  results_sub <- results[, c("year", "variable", "value")]
  wide <- reshape(results_sub, v.names = "value", idvar = "year",
                  timevar = "variable", direction = "wide")
  names(wide) <- gsub("value\\.", "", names(wide))
  names(wide)[names(wide) == "year"] <- "Date"
  names(wide)[names(wide) == "Ca"] <- CO2_CONSTRAIN()
  names(wide)[names(wide) == "CH4"] <- CH4_CONSTRAIN()
  names(wide)[names(wide) == "N2O"] <- N2O_CONSTRAIN()
  names(wide) <- gsub("_concentration$", "_constrain", names(wide))

  # Save the concentration table, note that this must be written out
  # so that the ini can read it in.
  tmp_dir <- tempfile()
  dir.create(tmp_dir, showWarnings = FALSE)
  tmpfile <- file.path(tmp_dir, "test_conc.csv")
  write.csv(wide, tmpfile, row.names = FALSE, quote = FALSE)

  # Add lines of text to the ini that create the different
  # concentration constraints. The constraints will use the
  # concentration time series stored in the data files.
  ini_txt <- append(
    ini_txt,
    paste0("N2O_constrain=csv:", tmpfile),
    grep("\\[N2O\\]", ini_txt)
  )

  ini_txt <- append(
    ini_txt,
    paste0("CH4_constrain=csv:", tmpfile),
    grep("\\[CH4\\]", ini_txt)
  )

  for (h in halocarbs) {
    ini_txt <- append(
      ini_txt,
      paste0(h, "_constrain=csv:", tmpfile),
      grep(paste0("\\[", h, "_halocarbon", "\\]"), ini_txt)
    )
  }


  # Copy over the emission files from the input directory to the same location as the
  # the concentration constraints csv file. This is required by the Hector core.
  tmprcp45_dir <- file.path(dirname(tmp_dir), "emissions")
  dir.create(tmprcp45_dir, showWarnings = FALSE, recursive = TRUE)
  file.copy(system.file("input", "emissions", "RCP45_emissions.csv",
                        package = "hector"), tmprcp45_dir)
  file.copy(system.file("input", "emissions", "volcanic_RF.csv",
                        package = "hector"), tmprcp45_dir)

  # Write out the new ini file.
  tmpini <- tempfile(fileext = ".ini")
  writeLines(ini_txt, tmpini)

  # Run Hector using the new ini file and extract the results.
  hc2 <- newcore(tmpini)
  invisible(run(hc2))
  results2 <- fetchvars(hc2, yrs, outvars)

  # The results should be equal.
  expect_equivalent(results, results2)
})

test_that("Discontinuous constraint works", {

  # Set up a Hector core.
  hc <- rcp45()
  all_years <- seq(startdate(hc), enddate(hc))

  # Create two CO2 constraints for two different time chunks, since
  # the purpose of this test is to make sure that the constraints work
  # if they are discontinuous so these two chuncks MUST be discontinuous.
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
  out_ca <- fetchvars(hc, all_years, ATMOSPHERIC_CO2())

  # RFs and concentrations during the constraint period are constant,
  # they should be since we read in constant CO2 concentrations.
  expect_equal(length(unique(out_rf[out_rf$year %in% ca_years_1, "value"])), 1)
  expect_equal(length(unique(out_rf[out_rf$year %in% ca_years_2, "value"])), 1)

  expect_equal(out_ca[out_ca$year %in% ca_years_1, "value"], ca_vals_1)
  expect_equal(out_ca[out_ca$year %in% ca_years_2, "value"], ca_vals_2)

  # Because we only read in CO2 concentrations over a short periods of time outside of the
  # first and second constrains the scenario should follow the RCP 4.5 trajectory increasing temp.
  expect_true(out_rf[out_ca$year == 2000, "value"] > out_rf[out_ca$year == 1900, "value"])
  expect_true(out_ca[out_ca$year == 2000, "value"] > out_ca[out_ca$year == 1900, "value"])

  # Make sure that the CO2 concentration matches the constraints read in during the two
  # different constraint periods.
  expect_equal(fetchvars(hc, min(ca_years_1), ATMOSPHERIC_CO2())$value, ca_vals_1[1])
  expect_equal(fetchvars(hc, max(ca_years_1), ATMOSPHERIC_CO2())$value, ca_vals_1[1])
  expect_equal(fetchvars(hc, min(ca_years_2), ATMOSPHERIC_CO2())$value, ca_vals_2[1])
  expect_equal(fetchvars(hc, max(ca_years_2), ATMOSPHERIC_CO2())$value, ca_vals_2[1])

  # You should NOT be able to retrieve constraints before, between, or after
  # user-specified values.
  baddates <- c(min(ca_years_1) - 1, max(ca_years_1) + 1, max(ca_years_2) + 1)
  ca_before <- fetchvars(hc, baddates, CO2_CONSTRAIN())
  expect_true(all(is.na(ca_before$value)))
  expect_true(all(!is.nan(ca_before$value)))
})
