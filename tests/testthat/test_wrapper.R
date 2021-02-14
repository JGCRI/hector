context("Test basic R wrapper functionality")

inputdir <- system.file("input", package = "hector")
testvars <- c(ATMOSPHERIC_CO2(), RF_TOTAL(), GLOBAL_TEMP())
dates <- 2000:2300


test_that("Rerunning spinup produces minimal change", {

  # First Hector core run
  hc <- newcore(file.path(inputdir, "hector_rcp45.ini"), name = "RCP45", suppresslogging = TRUE)
  run(hc, 2100)
  dd1 <- fetchvars(hc, dates, testvars)

  # Reset to a time before the start date and re run.
  reset(hc, 0.0)
  run(hc, 2100)
  dd2 <- fetchvars(hc, dates, testvars)

  # 1.0e-6 is a small number selected to make sure that re-running the spin-up
  # only produces a minimal change.
  diff <- abs((dd2$value - dd1$value) / (dd1$value + 1.0e-6))
  expect_lt(max(diff), 1.0e-6)

  shutdown(hc)
})


test_that("Basic hcore functionality works", {
  hc <- newcore(file.path(inputdir, "hector_rcp45.ini"), name = "RCP45", suppresslogging = TRUE)
  run(hc, 2100)
  expect_true(inherits(hc, "hcore"))
  expect_true(isactive(hc))
  expect_equal(startdate(hc), 1745)
  expect_equal(enddate(hc), 2300)
  expect_equal(getdate(hc), 2100)
  expect_equal(getname(hc), "RCP45")

  expect_error(run(hc, 2050), "is prior to the current date")
  expect_silent(run(hc, 2100))
  expect_silent(run(hc))

  hc <- shutdown(hc)
  expect_false(isactive(hc))
})

test_that("Write out logs", {

  log_dir <- file.path(getwd(), "logs")
  if (dir.exists(log_dir)) {
    # Remove any pre-existing logs so we get an accurate count below
    unlink(file.path(log_dir, "*.log"))
  }

  ## Turn logging ON for one test and confirm it runs (see GitHub issues #372 and #381)
  hc_log <- newcore(file.path(inputdir, "hector_rcp45.ini"),
                    name = "RCP45",
                    suppresslogging = FALSE)
  run(hc_log, 2100)
  shutdown(hc_log)

  # Look for the existence of the `logs` directory
  expect_true(dir.exists(log_dir))

  # Check to see that individual log files were written out
  expect_equal(length(list.files(log_dir, pattern = ".log")), 40)

  # Check that errors on shutdown cores get caught
  expect_error(getdate(hc_log), "Invalid or inactive")
  expect_error(run(hc_log), "Invalid or inactive")
  expect_error(fetchvars(hc_log), "Invalid or inactive")
})


## Make sure that that when the Hector core is shut downsys
## everything is tidied up.
test_that("Garbage collection shuts down hector cores", {
  ## This test makes use of some knowledge about the structure of the hector
  ## core objects that no user should ever assume.
  hc <- newcore(file.path(inputdir, "hector_rcp45.ini"), suppresslogging = TRUE, name = "core1")
  expect_true(isactive(hc))
  coreidx1 <- hc$coreidx

  hc <- newcore(file.path(inputdir, "hector_rcp45.ini"), suppresslogging = TRUE, name = "core2")
  expect_true(isactive(hc))
  coreidx2 <- hc$coreidx
  expect_equal(coreidx2, 1 + coreidx1)
  gc(verbose = FALSE)

  ## make a deep copy of the structure
  oldhc <- as.environment(as.list(hc))
  oldhc$coreidx <- coreidx1
  class(oldhc) <- c("hcore", "environment")
  expect_false(isactive(oldhc))
  expect_true(isactive(hc))
  shutdown(hc)
})

## Ensure the scenario name can change in the output
## when the core is set up different names.
test_that("Scenario column is created in output", {
  hc <- newcore(file.path(inputdir, "hector_rcp45.ini"), suppresslogging = TRUE, name = "scenario1")
  run(hc)

  outdata1 <- fetchvars(hc, dates, testvars)
  expect_equal(outdata1$scenario, rep("scenario1", nrow(outdata1)))

  outdata2 <- fetchvars(hc, dates, testvars, "scenario2")
  expect_equal(outdata2$scenario, rep("scenario2", nrow(outdata2)))

  shutdown(hc)
})

# As a deterministic model Hector should produce the same output each time it is run.
# Run Hector multiple times and check output.
test_that("Reset produces identical results", {
  hc <- newcore(file.path(inputdir, "hector_rcp45.ini"), suppresslogging = TRUE)
  run(hc)
  expect_equal(getdate(hc), enddate(hc))
  outdata1 <- fetchvars(hc, dates, testvars)

  reset(hc, 2000)
  expect_equal(getdate(hc), 2000)
  run(hc)
  outdata2 <- fetchvars(hc, dates, testvars)
  expect_equal(outdata1, outdata2)
  hc <- shutdown(hc)
})

test_that("Exceptions are caught", {
  expect_error(hc <- newcore("foo"), "does not exist")
  hc <- newcore(file.path(inputdir, "hector_rcp45.ini"), suppresslogging = TRUE)
  setvar(hc, NA, BETA(), -1.0, NA)
  expect_error(reset(hc), "beta")
  ## Verify that the core can continue to run after an error has been thrown.
  setvar(hc, NA, BETA(), 0.5, NA)
  expect_silent(reset(hc))
  expect_silent(run(hc, 2100))
})


test_that("Automatic reset is performed if and only if core is not marked 'clean'.", {
  hc <- newcore(file.path(inputdir, "hector_rcp45.ini"), suppresslogging = TRUE)

  expect_true(hc$clean)
  run(hc, 2100)
  ## We can test whether reset actually runs by noting that trying to run to
  ## an earlier date is an error, but an auto-reset will prevent the error
  expect_error(run(hc, 2050), "is prior") # No reset
  hc$clean <- FALSE
  hc$reset_date <- 0
  expect_silent(run(hc, 2050)) # reset performed
  expect_true(hc$clean)

  hc$clean <- FALSE
  reset(hc) # explicit reset
  expect_true(hc$clean)

  hc$clean <- FALSE
  reset(hc, startdate(hc)) # doesn't rerun spinup
  expect_false(hc$clean)
  hc$reset_date <- 2000 # spinup no longer required
  reset(hc, startdate(hc))
  expect_true(hc$clean)

  shutdown(hc)
})


test_that("Setting future values does not trigger a reset.", {
  hc <- newcore(file.path(inputdir, "hector_rcp45.ini"), suppresslogging = TRUE)

  run(hc, 2100)
  setvar(hc, 2101:2300, FFI_EMISSIONS(), 0.0, "Pg C/yr")
  expect_true(hc$clean)

  shutdown(hc)
})


test_that("Setting past or parameter values does trigger a reset.", {
  hc <- newcore(file.path(inputdir, "hector_rcp45.ini"), suppresslogging = TRUE)
  run(hc, 2100)

  setvar(hc, 2050:2150, FFI_EMISSIONS(), 0.0, "Pg C/yr")
  expect_false(hc$clean)
  expect_equal(hc$reset_date, 2049)
  expect_error(run(hc, 2048), "is prior")
  expect_true(hc$clean) # reset still gets run!
  expect_silent(run(hc, 2050))
  expect_true(hc$clean)

  setvar(hc, 2050:2150, FFI_EMISSIONS(), 0.0, "Pg C/yr") # edge case
  expect_false(hc$clean)
  expect_equal(hc$reset_date, 2049)
  expect_error(run(hc, 2048), "is prior")
  expect_silent(run(hc, 2050))
  expect_true(hc$clean)

  ## Setting two sets of values should reset to the lower one
  setvar(hc, 2000, FFI_EMISSIONS(), 0.0, "Pg C/yr")
  setvar(hc, 2010, FFI_EMISSIONS(), 0.0, "Pg C/yr")
  expect_equal(hc$reset_date, 1999)
  expect_false(hc$clean)
  setvar(hc, 1972, FFI_EMISSIONS(), 0.0, "Pg C/yr")
  expect_equal(hc$reset_date, 1971)
  expect_false(hc$clean)

  ## Setting parameter values should trigger a reset
  setvar(hc, NA, ECS(), 2.5, "degC")
  expect_false(hc$clean)
  expect_equal(hc$reset_date, 0)
  reset(hc)
  expect_true(hc$clean)
  setvar(hc, NA, ECS(), 3, "degC")
  expect_false(hc$clean)
  setvar(hc, 1800, FFI_EMISSIONS(), 0.0, "Pg C/yr") # shouldn't change the
  # reset date
  expect_equal(hc$reset_date, 0)

  shutdown(hc)
})
