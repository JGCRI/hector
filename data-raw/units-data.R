# Use `devtools::load_all()` here instead of `library()` because this script is
# more likely to be run while developing the package, in which case the R
# package library may have an out of date version of Hector (e.g. with new
# variables missing).
devtools::load_all()

simpleNbox_vars <- c(
  PREINDUSTRIAL_CO2(), NPP_FLUX0(),
  FFI_EMISSIONS(), DACCS_UPTAKE(), LUC_EMISSIONS(), LUC_UPTAKE(),
  CO2_CONSTRAIN(), NBP_CONSTRAIN(), BETA(), Q10_RH()
)
simpleNbox_uni <- c(
  "ppmv CO2", "Pg C/yr", "Pg C/yr", "Pg C/yr", "Pg C/yr",
  "Pg C/yr", "ppmv CO2", "Pg C/yr", "(unitless)", "(unitless)"
)

ocean_vars <- c(TT(), TU(), TWI(), TID(), OCEAN_PREIND_C_SURF(), OCEAN_PREIND_C_ID())
ocean_uni <- c("m3/s", "m3/s", "m3/s", "m3/s", "Pg C", "Pg C")

so2_vars <- c(EMISSIONS_SO2(), VOLCANIC_SO2())
so2_uni <- c("Gg S", "W/m2")

ch4_vars <- c(
  PREINDUSTRIAL_CH4(), NATURAL_CH4(), EMISSIONS_CH4(), CH4_CONSTRAIN(),
  LIFETIME_SOIL(), LIFETIME_STRAT()
)
ch4_uni <- c("ppbv CH4", "Tg CH4", "Tg CH4", "ppbv CH4", "Years", "Years")

oh_vars <- c(EMISSIONS_NOX(), EMISSIONS_CO(), EMISSIONS_NMVOC())
oh_uni <- c("Tg N", "Tg CO", "Tg NMVOC")

n2o_vars <- c(EMISSIONS_N2O(), N2O_CONSTRAIN(), NAT_EMISSIONS_N2O(), PREINDUSTRIAL_N2O())
n2o_uni <- c("Tg N", "ppbv N2O", "Tg N", "ppbv N2O")

forcing_vars <- c(
  DELTA_CO2(), DELTA_CH4(), DELTA_N2O(), RHO_BC(),
  RHO_OC(), RHO_SO2(), RHO_NH3(), RF_ALBEDO(), RF_MISC()
)
forcing_uni <- c(
  "(unitless)", "(unitless)", "(unitless)", "W/m2/Tg",
  "W/m2/Tg", "W/m2/Gg", "W/m2/Tg", "W/m2", "W/m2"
)

temp_vars <- c(
  ECS(), DIFFUSIVITY(), AERO_SCALE(), VOLCANIC_SCALE(),
  Q_CO2(), TAS_CONSTRAIN(), LO_WARMING_RATIO()
)
temp_uni <- c(
  "degC", "cm2/s", "(unitless)", "(unitless)", "W/m2",
  "degC", "(unitless)"
)

misc_emiss_vars <- c(EMISSIONS_BC(), EMISSIONS_OC(), EMISSIONS_NH3())
misc_emiss_uni <- c("Tg", "Tg", "Tg")

haloemis_vars <- c(
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
haloemis_uni <- "Gg"

haloconstrain_vars <- c(
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
haloconstrain_uni <- "pptv"

halodeltas_vars <- c(
  DELTA_CF4(), DELTA_C2F6(), DELTA_HFC23(), DELTA_HFC32(),
  DELTA_HFC4310(), DELTA_HFC125(), DELTA_HFC134A(), DELTA_HFC143A(),
  DELTA_HFC227EA(), DELTA_HFC245FA(), DELTA_SF6(), DELTA_CFC11(),
  DELTA_CFC12(), DELTA_CFC113(), DELTA_CFC114(), DELTA_CFC115(),
  DELTA_CCL4(), DELTA_CH3CCL3(), DELTA_HCFC22(), DELTA_HCFC141B(),
  DELTA_HCFC142B(), DELTA_HALON1211(), DELTA_HALON1301(),
  DELTA_HALON2402(), DELTA_CH3CL(), DELTA_CH3BR(), DELTA_CH4(),
  DELTA_CO2()
)
halodeltas_uni <- "(unitless)"

misc_var <- c(TRACKING_DATE())
misc_uni <- c("(unitless)")

unitstable <- rbind(
  data.frame(variable = simpleNbox_vars, units = simpleNbox_uni),
  data.frame(variable = ocean_vars, units = ocean_uni),
  data.frame(variable = so2_vars, units = so2_uni),
  data.frame(variable = ch4_vars, units = ch4_uni),
  data.frame(variable = oh_vars, units = oh_uni),
  data.frame(variable = n2o_vars, units = n2o_uni),
  data.frame(variable = forcing_vars, units = forcing_uni),
  data.frame(variable = temp_vars, units = temp_uni),
  data.frame(variable = misc_emiss_vars, units = misc_emiss_uni),
  data.frame(variable = haloemis_vars, units = haloemis_uni),
  data.frame(variable = haloconstrain_vars, units = haloconstrain_uni),
  data.frame(variable = halodeltas_vars, units = halodeltas_uni),
  data.frame(variable = misc_var, units = misc_uni)
)

usethis::use_data(unitstable, internal = FALSE, overwrite = TRUE)
