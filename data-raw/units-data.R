# Use `devtools::load_all()` here instead of `library()` because this script is
# more likely to be run while developing the package, in which case the R
# package library may have an out of date version of Hector (e.g. with new
# variables missing).
devtools::load_all()

hvars <- c(
  ECS(), PREINDUSTRIAL_CO2(), Q10_RH(), BETA(), AERO_SCALE(), VOLCANIC_SCALE(), DIFFUSIVITY(),
  FFI_EMISSIONS(), LUC_EMISSIONS(),
  EMISSIONS_BC(), EMISSIONS_N2O(), EMISSIONS_NOX(), EMISSIONS_CO(),
  EMISSIONS_NMVOC(), EMISSIONS_OC(), NAT_EMISSIONS_N2O(), EMISSIONS_NH3(),
  EMISSIONS_CH4(), PREINDUSTRIAL_CH4(), NATURAL_CH4(), LIFETIME_SOIL(), LIFETIME_STRAT(),
  EMISSIONS_SO2(), VOLCANIC_SO2(), CO2_CONSTRAIN(), TAS_CONSTRAIN(),
  CH4_CONSTRAIN(), N2O_CONSTRAIN(), NBP_CONSTRAIN(),
  TRACKING_DATE(), RHO_BC(), LO_WARMING_RATIO(), Q_CO2()
)


hunits <- c(
  "degC", "ppmv CO2", "(unitless)", "(unitless)", "(unitless)", "(unitless)", "cm2/s",
  "Pg C/yr", "Pg C/yr",
  "Tg", "Tg N", "Tg N", "Tg CO",
  "Tg NMVOC", "Tg", "Tg N", "Tg",
  "Tg CH4", "ppbv CH4", "Tg CH4", "Years", "Years",
  "Gg S", "W/m2", "ppmv CO2", "degC",
  "ppbv CH4", "ppbv N2O", "Pg C/yr",
  "(unitless)", "W/m2/Tg", "(unitless)", "W/m2"
)


haloemis <- c(
  EMISSIONS_CF4(), EMISSIONS_C2F6(), EMISSIONS_HFC23(),
  EMISSIONS_HFC32(), EMISSIONS_HFC4310(), EMISSIONS_HFC125(),
  EMISSIONS_HFC134A(), EMISSIONS_HFC143A(), EMISSIONS_HFC227EA(),
  EMISSIONS_HFC245FA(), EMISSIONS_SF6(), EMISSIONS_CFC11(),
  EMISSIONS_CFC12(), EMISSIONS_CFC113(), EMISSIONS_CFC114(),
  EMISSIONS_CFC115(), EMISSIONS_CCL4(), EMISSIONS_CH3CCL3(),
  EMISSIONS_HCFC22(), EMISSIONS_HCFC141B(), EMISSIONS_HCFC142B(),
  EMISSIONS_HALON1211(), EMISSIONS_HALON1301(),
  EMISSIONS_HALON2402(), EMISSIONS_CH3CL(), EMISSIONS_CH3BR()
)
halounits <- "Gg"

haloconstrain <- c(
  CF4_CONSTRAIN(), C2F6_CONSTRAIN(), HFC23_CONSTRAIN(),
  HFC32_CONSTRAIN(), HFC4310_CONSTRAIN(), HFC125_CONSTRAIN(),
  HFC134A_CONSTRAIN(), HFC143A_CONSTRAIN(), HFC227EA_CONSTRAIN(),
  HFC245FA_CONSTRAIN(), SF6_CONSTRAIN(), CFC11_CONSTRAIN(),
  CFC12_CONSTRAIN(), CFC113_CONSTRAIN(), CFC114_CONSTRAIN(),
  CFC115_CONSTRAIN(), CCL4_CONSTRAIN(), CH3CCL3_CONSTRAIN(),
  HCFC22_CONSTRAIN(), HCFC141B_CONSTRAIN(), HCFC142B_CONSTRAIN(),
  HALON1211_CONSTRAIN(), HALON1301_CONSTRAIN(), HALON2402_CONSTRAIN(),
  CH3CL_CONSTRAIN(), CH3BR_CONSTRAIN()
)
haloconstrain_units <- "pptv"

hdeltas <- c(DELTA_CF4(), DELTA_C2F6(), DELTA_HFC23(), DELTA_HFC32(), DELTA_HFC4310(), DELTA_HFC125(),
             DELTA_HFC134A(), DELTA_HFC143A(), DELTA_HFC227EA(), DELTA_HFC245FA(), DELTA_SF6(),
             DELTA_CFC11(), DELTA_CFC12(), DELTA_CFC113(), DELTA_CFC114(), DELTA_CFC115(), DELTA_CCL4(),
             DELTA_CH3CCL3(), DELTA_HCFC22(), DELTA_HCFC141B(), DELTA_HCFC142B(), DELTA_HALON1211(),
             DELTA_HALON1301(), DELTA_HALON2402(), DELTA_CH3CL(), DELTA_CH3BR(), DELTA_CH4(), DELTA_CO2()
)

hdeltas_units <- "(unitless)"

unitstable <- rbind(
  data.frame(variable = hvars, units = hunits),
  data.frame(variable = haloemis, units = halounits),
  data.frame(variable = haloconstrain, units = haloconstrain_units),
  data.frame(variable = hdeltas, units = hdeltas_units)
)

usethis::use_data(unitstable, internal = TRUE, overwrite = TRUE)
