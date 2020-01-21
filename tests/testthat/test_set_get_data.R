context("Setting and getting data")

emissions <- c(
  EMISSIONS_BC(),
  EMISSIONS_C2F6(),
  EMISSIONS_CCL4(),
  EMISSIONS_CF4(),
  EMISSIONS_CFC11(),
  EMISSIONS_CFC113(),
  EMISSIONS_CFC114(),
  EMISSIONS_CFC115(),
  EMISSIONS_CFC12(),
  EMISSIONS_CH3BR(),
  EMISSIONS_CH3CCL3(),
  EMISSIONS_CH3CL(),
  EMISSIONS_CH4(),
  EMISSIONS_CO(),
  EMISSIONS_HALON1211(),
  EMISSIONS_HALON1301(),
  EMISSIONS_HALON2402(),
  EMISSIONS_HCF141B(),
  EMISSIONS_HCF142B(),
  EMISSIONS_HCF22(),
  EMISSIONS_HFC125(),
  EMISSIONS_HFC134A(),
  EMISSIONS_HFC143A(),
  EMISSIONS_HFC227EA(),
  EMISSIONS_HFC23(),
  EMISSIONS_HFC245FA(),
  EMISSIONS_HFC32(),
  EMISSIONS_HFC4310(),
  EMISSIONS_N2O(),
  EMISSIONS_NMVOC(),
  EMISSIONS_NOX(),
  EMISSIONS_OC(),
  EMISSIONS_SF6(),
  EMISSIONS_SO2(),
  FFI_EMISSIONS(),
  LUC_EMISSIONS(),
  NAT_EMISSIONS_N2O()
)

# Setting emissions variables
for (v in emissions) {
  hc <- newcore(system.file("input", "hector_rcp45.ini", package = "hector"),
                suppresslogging = TRUE)
  # Use a random (positive) value here to make sure we are getting and setting
  # it consistently.
  val <- rexp(1, 0.2)
  year <- 1800
  unit <- getunits(v)
  test_that(paste0("Setting variable ", v, " works."), {
    expect_silent(setvar(hc, year, v, val, unit))
  })
  invisible(run(hc, year))
  test_that(paste0("Getting variable ", v, " works."), {
    expect_equal(fetchvars(hc, year, v)[["value"]], val)
  })
}
