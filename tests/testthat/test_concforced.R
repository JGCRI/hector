context("Concentration-forced runs")

rcp45 <- function() newcore(system.file("input", "hector_rcp45.ini",
                                        package = "hector"))

test_that("Concentration-forced runs work for halocarbons", {
  hc <- rcp45()
  invisible(run(hc))
  dates <- seq(startdate(hc), getdate(hc))
  outvars <- c(GLOBAL_TEMP(), outvars[2])
  out1 <- fetchvars(hc, dates, outvars)
  conc1 <- subset(out1, variable == outvars[2])
  emiss1 <- fetchvars(hc, dates, "HFC23_emissions")
  setvar(hc, conc1$year, outvars[2], conc1$value, conc1$units[1])
  invisible(reset(hc))
  invisible(run(hc))
  out2 <- fetchvars(hc, dates, outvars)
  expect_equivalent(out1$value, out2$value, tol = 1e-10)
  emiss2 <- fetchvars(hc, dates, "HFC23_emissions")
  expect_true(all(emiss2$value == 0))

  test_that("Increasing HFC23 concentrations increases global temp", {
    newhfc <- conc1$value * 1.05
    setvar(hc, conc1$year, outvars[2], newhfc, conc1$units[1])
    expect_equal(fetchvars(hc, conc1$year, outvars[2])$value, newhfc)
    invisible(reset(hc))
    invisible(run(hc))
    newout <- fetchvars(hc, dates, outvars)
    expect_true(all(newout$value >= out2$value))
  })
})

test_that("Concentration-forced runs work for CH4", {
  hc <- rcp45()
  invisible(run(hc))
  dates <- seq(startdate(hc), getdate(hc))
  outvars <- c(GLOBAL_TEMP(), ATMOSPHERIC_CH4())
  out1 <- fetchvars(hc, dates, outvars)
  conc1 <- subset(out1, variable == outvars[2])
  emiss1 <- fetchvars(hc, dates, EMISSIONS_CH4())
  setvar(hc, conc1$year, outvars[2], conc1$value, conc1$units[1])
  invisible(reset(hc))
  invisible(run(hc))
  out2 <- fetchvars(hc, dates, outvars)
  expect_equivalent(out1$value, out2$value, tol = 1e-10)
  emiss2 <- fetchvars(hc, dates, EMISSIONS_CH4())
  expect_true(all(emiss2$value == 0))

  test_that("Increasing CH4 concentrations increases global temp", {
    newhfc <- conc1$value * 1.05
    setvar(hc, conc1$year, outvars[2], newhfc, conc1$units[1])
    expect_equal(fetchvars(hc, conc1$year, outvars[2])$value, newhfc)
    invisible(reset(hc))
    invisible(run(hc))
    newout <- fetchvars(hc, 2000:2100, outvars)
    expect_true(all(newout$value >= subset(out2, year %in% 2000:2100)$value))
  })
})
