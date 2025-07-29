context("setvar/fetchvars")

inputdir <- system.file("input", package = "hector")
inifile <- file.path(inputdir, "hector_ssp245.ini")

test_that("Invalid input variable returns error", {
  # Test that if the length of var_split is >2, error is returned
  core <- newcore(inifile)
  expect_error(setvar(core, NA, "global.permafrost.beta", 10, ""),
    regexp = "Invalid input variable: '"
  )
  shutdown(core)
})

test_that("Default fetchvar without dates returns error", {
  # Test that fetchvars errors if neither dates nor vars is supplied
  core <- newcore(inifile)
  expect_error(fetchvars(core, NA), regexp = "all require dates")
  shutdown(core)
})
