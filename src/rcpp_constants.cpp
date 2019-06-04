#include <Rcpp.h>
#include "hector.hpp"
#include "component_data.hpp"
#include "simpleNbox.hpp"

using namespace Rcpp;

/*********************************************************************************
 * Functions to expose constants defined in C++ headers to R
 *
 * The only way I've been able to find to get these constants into R is to provide
 * Rcpp exported functions that return them.
 *********************************************************************************/

// [[Rcpp::plugins("cpp11")]]

/*
 * Message types
 */
//' @include aadoc.R
//' @describeIn msgtype Message type for retrieving data from a component
//' @export
//' @keywords internal
// [[Rcpp::export]]
String GETDATA() {
return M_GETDATA;
}

//' @describeIn msgtype Message type for setting data in a component
//' @export
//' @keywords internal
// [[Rcpp::export]]
String SETDATA() {
return M_SETDATA;
}

/* Logging levels */

//' @describeIn loglevels Set logging at 'debug' level.
//' @export
// [[Rcpp::export]]
int LL_DEBUG() {
return (int) Hector::Logger::DEBUG;
}

//' @describeIn loglevels Set logging at 'notice' level.
//' @export
// [[Rcpp::export]]
int LL_NOTICE() {
return (int) Hector::Logger::NOTICE;
}

//' @describeIn loglevels Set logging at 'warning' level.
//' @export
// [[Rcpp::export]]
int LL_WARNING() {
return (int) Hector::Logger::WARNING;
}

//' @describeIn loglevels Set logging at 'severe' level.
//' @export
// [[Rcpp::export]]
int LL_SEVERE() {
return (int) Hector::Logger::SEVERE;
}


/*****************************************************************
 * Capabilities
 *****************************************************************/

/* BC component */
//' @describeIn emissions Black carbon emissions (\code{"Tg"})
//' @export
// [[Rcpp::export]]
String EMISSIONS_BC() {
return D_EMISSIONS_BC;
}

/* Forcing component */
//' @describeIn forcings Total radiative forcing
//' @export
// [[Rcpp::export]]
String RF_TOTAL() {
return D_RF_TOTAL;
}

//' @describeIn forcings Radiative forcing due to albedo
//' @export
// [[Rcpp::export]]
String RF_T_ALBEDO() {
return D_RF_T_ALBEDO;
}

//' @describeIn forcings Radiative forcing due to CO2
//' @export
// [[Rcpp::export]]
String RF_CO2() {
return D_RF_CO2;
}

//' @describeIn forcings Radiative forcing due to N2O
//' @export
// [[Rcpp::export]]
String RF_N2O() {
return D_RF_N2O;
}

//' @describeIn forcings Radiative forcing due to water vapor
//' @export
// [[Rcpp::export]]
String RF_H2O() {
return D_RF_H2O;
}

//' @describeIn forcings Radiative forcing due to ozone
//' @export
// [[Rcpp::export]]
String RF_O3() {
return D_RF_O3;
}

//' @describeIn forcings Radiative forcing due to black carbon
//' @export
// [[Rcpp::export]]
String RF_BC() {
return D_RF_BC;
}

//' @describeIn forcings Radiative forcing due to organic carbon
//' @export
// [[Rcpp::export]]
String RF_OC() {
return D_RF_OC;
}

//' @describeIn forcings Direct contribution of SO2 to radiative forcing
//' @export
// [[Rcpp::export]]
String RF_SO2D() {
return D_RF_SO2d;
}

//' @describeIn forcings Indirect contribution of SO2 to radiative forcing
//' @export
// [[Rcpp::export]]
String RF_SO2I() {
return D_RF_SO2i;
}

//' @describeIn forcings Total contribution (direct and indirect) of SO2 to radiative forcing
//' @export
// [[Rcpp::export]]
String RF_SO2() {
return D_RF_SO2;
}

//' @describeIn forcings Radiative forcing due to volcanic activity
//' @export
// [[Rcpp::export]]
String RF_VOL() {
return D_RF_VOL;
}

/* Halocarbon forcings */
/* There's a constant for total halocarbon forcing defined in component_data.hpp, but
   returning the total halocarbon forcing appears not to be implemented. */
// //' @export
// // [[Rcpp::export]]
// String RF_HALOCARBONS() {
// return D_RF_halocarbons;
// }

// This is the prefix for adjusted radiative forcing.  We use it
// internally to correct the reported names of the halocarbon
// forcings.
// [[Rcpp::export]]
String RFADJ_PREFIX() {
return D_RFADJ_PREFIX;
}

// Prefix for unadjusted forcing.  This is what we substitute for
// the adjusted forcing prefix
// [[Rcpp::export]]
String RF_PREFIX() {
return D_RF_PREFIX;
}

//' @describeIn haloforcings Radiative forcing due to CF4
//' @export
// [[Rcpp::export]]
String RF_CF4() {
return D_RFADJ_CF4;
}

//' @describeIn haloforcings Radiative forcing due to C2F6
//' @export
// [[Rcpp::export]]
String RF_C2F6() {
return D_RFADJ_C2F6;
}

//' @describeIn haloforcings Radiative forcing due to HFC-23
//' @export
// [[Rcpp::export]]
String RF_HFC23() {
return D_RFADJ_HFC23;
}

//' @describeIn haloforcings Radiative forcing due to HFC-32
//' @export
// [[Rcpp::export]]
String RF_HFC32() {
return D_RFADJ_HFC32;
}

//' @describeIn haloforcings Radiative forcing due to HFC-4310
//' @export
// [[Rcpp::export]]
String RF_HFC4310() {
return D_RFADJ_HFC4310;
}

//' @describeIn haloforcings Radiative forcing due to HFC-125
//' @export
// [[Rcpp::export]]
String RF_HFC125() {
return D_RFADJ_HFC125;
}

//' @describeIn haloforcings Radiative forcing due to HFC-134a
//' @export
// [[Rcpp::export]]
String RF_HFC134A() {
return D_RFADJ_HFC134a;
}

//' @describeIn haloforcings Radiative forcing due to HFC-143a
//' @export
// [[Rcpp::export]]
String RF_HFC143A() {
return D_RFADJ_HFC143a;
}

//' @describeIn haloforcings Radiative forcing due to HFC227ea
//' @export
// [[Rcpp::export]]
String RF_HFC227EA() {
return D_RFADJ_HFC227ea;
}

//' @describeIn haloforcings Radiative forcing due to HFC-245fa
//' @export
// [[Rcpp::export]]
String RF_HFC245FA() {
return D_RFADJ_HFC245fa;
}

//' @describeIn haloforcings Radiative forcing due to sulfur hexafluoride
//' @export
// [[Rcpp::export]]
String RF_SF6() {
return D_RFADJ_SF6;
}

//' @describeIn haloforcings Radiative forcing due to CFC-11
//' @export
// [[Rcpp::export]]
String RF_CFC11() {
return D_RFADJ_CFC11;
}

//' @describeIn haloforcings Radiative forcing due to CFC-12
//' @export
// [[Rcpp::export]]
String RF_CFC12() {
return D_RFADJ_CFC12;
}

//' @describeIn haloforcings Radiative forcing due to CFC-113
//' @export
// [[Rcpp::export]]
String RF_CFC113() {
return D_RFADJ_CFC113;
}

//' @describeIn haloforcings Radiative forcing due to CFC-114
//' @export
// [[Rcpp::export]]
String RF_CFC114() {
return D_RFADJ_CFC114;
}

//' @describeIn haloforcings Radiative forcing due to CFC-115
//' @export
// [[Rcpp::export]]
String RF_CFC115() {
return D_RFADJ_CFC115;
}

//' @describeIn haloforcings Radiative forcing due to carbon tetrachloride
//' @export
// [[Rcpp::export]]
String RF_CCL4() {
return D_RFADJ_CCl4;
}

//' @describeIn haloforcings Radiative forcing due to trichloroethane
//' @export
// [[Rcpp::export]]
String RF_CH3CCL3() {
return D_RFADJ_CH3CCl3;
}

//' @describeIn haloforcings Radiative forcing due to HCFC-22
//' @export
// [[Rcpp::export]]
String RF_HCF22() {
return D_RFADJ_HCF22;
}

//' @describeIn haloforcings Radiative forcing due to HCFC-141b
//' @export
// [[Rcpp::export]]
String RF_HCF141B() {
return D_RFADJ_HCF141b;
}

//' @describeIn haloforcings Radiative forcing due to HCFC-142b
//' @export
// [[Rcpp::export]]
String RF_HCF142B() {
return D_RFADJ_HCF142b;
}

//' @describeIn haloforcings Radiative forcing due to halon-1211
//' @export
// [[Rcpp::export]]
String RF_HALON1211() {
return D_RFADJ_halon1211;
}

//' @describeIn haloforcings Radiative forcing due to halon-1301
//' @export
// [[Rcpp::export]]
String RF_HALON1301() {
return D_RFADJ_halon1301;
}

//' @describeIn haloforcings Radiative forcing due to halon-2402
//' @export
// [[Rcpp::export]]
String RF_HALON2402() {
return D_RFADJ_halon2402;
}

//' @describeIn haloforcings Radiative forcing due to chloromethane
//' @export
// [[Rcpp::export]]
String RF_CH3CL() {
return D_RFADJ_CH3Cl;
}

//' @describeIn haloforcings Radiative forcing due to bromomethane
//' @export
// [[Rcpp::export]]
String RF_CH3BR() {
return D_RFADJ_CH3Br;
}

/* halocarbon emissions */
//' @describeIn haloemiss Emissions for CF4
//' @export
// [[Rcpp::export]]
String EMISSIONS_CF4() {
return D_EMISSIONS_CF4;
}

//' @describeIn haloemiss Emissions for C2F6
//' @export
// [[Rcpp::export]]
String EMISSIONS_C2F6() {
return D_EMISSIONS_C2F6;
}

//' @describeIn haloemiss Emissions for HFC-23
//' @export
// [[Rcpp::export]]
String EMISSIONS_HFC23() {
return D_EMISSIONS_HFC23;
}

//' @describeIn haloemiss Emissions for HFC-32
//' @export
// [[Rcpp::export]]
String EMISSIONS_HFC32() {
return D_EMISSIONS_HFC32;
}

//' @describeIn haloemiss Emissions for HFC-4310
//' @export
// [[Rcpp::export]]
String EMISSIONS_HFC4310() {
return D_EMISSIONS_HFC4310;
}

//' @describeIn haloemiss Emissions for HFC-125
//' @export
// [[Rcpp::export]]
String EMISSIONS_HFC125() {
return D_EMISSIONS_HFC125;
}

//' @describeIn haloemiss Emissions for HFC-134a
//' @export
// [[Rcpp::export]]
String EMISSIONS_HFC134A() {
return D_EMISSIONS_HFC134a;
}

//' @describeIn haloemiss Emissions for HFC-143a
//' @export
// [[Rcpp::export]]
String EMISSIONS_HFC143A() {
return D_EMISSIONS_HFC143a;
}

//' @describeIn haloemiss Emissions for HFC-227ea
//' @export
// [[Rcpp::export]]
String EMISSIONS_HFC227EA() {
return D_EMISSIONS_HFC227ea;
}

//' @describeIn haloemiss Emissions for HFC-254fa
//' @export
// [[Rcpp::export]]
String EMISSIONS_HFC245FA() {
return D_EMISSIONS_HFC245fa;
}

//' @describeIn haloemiss Emissions for sulfur hexafluoride
//' @export
// [[Rcpp::export]]
String EMISSIONS_SF6() {
return D_EMISSIONS_SF6;
}

//' @describeIn haloemiss Emissions for CFC-11
//' @export
// [[Rcpp::export]]
String EMISSIONS_CFC11() {
return D_EMISSIONS_CFC11;
}

//' @describeIn haloemiss Emissions for CFC-12
//' @export
// [[Rcpp::export]]
String EMISSIONS_CFC12() {
return D_EMISSIONS_CFC12;
}

//' @describeIn haloemiss Emissions for CFC-113
//' @export
// [[Rcpp::export]]
String EMISSIONS_CFC113() {
return D_EMISSIONS_CFC113;
}

//' @describeIn haloemiss Emissions for CFC-114
//' @export
// [[Rcpp::export]]
String EMISSIONS_CFC114() {
return D_EMISSIONS_CFC114;
}

//' @describeIn haloemiss Emissions for CFC-115
//' @export
// [[Rcpp::export]]
String EMISSIONS_CFC115() {
return D_EMISSIONS_CFC115;
}

//' @describeIn haloemiss Emissions for carbon tetrachloride
//' @export
// [[Rcpp::export]]
String EMISSIONS_CCL4() {
return D_EMISSIONS_CCl4;
}

//' @describeIn haloemiss Emissions for trichloroethane
//' @export
// [[Rcpp::export]]
String EMISSIONS_CH3CCL3() {
return D_EMISSIONS_CH3CCl3;
}

//' @describeIn haloemiss Emissions for HCFC-22
//' @export
// [[Rcpp::export]]
String EMISSIONS_HCF22() {
return D_EMISSIONS_HCF22;
}

//' @describeIn haloemiss Emissions for HCFC-141b
//' @export
// [[Rcpp::export]]
String EMISSIONS_HCF141B() {
return D_EMISSIONS_HCF141b;
}

//' @describeIn haloemiss Emissions for HCFC-142b
//' @export
// [[Rcpp::export]]
String EMISSIONS_HCF142B() {
return D_EMISSIONS_HCF142b;
}

//' @describeIn haloemiss Emissions for halon-1211
//' @export
// [[Rcpp::export]]
String EMISSIONS_HALON1211() {
return D_EMISSIONS_halon1211;
}

//' @describeIn haloemiss Emissions for halon-1301
//' @export
// [[Rcpp::export]]
String EMISSIONS_HALON1301() {
return D_EMISSIONS_halon1301;
}

//' @describeIn haloemiss Emissions for halon-2402
//' @export
// [[Rcpp::export]]
String EMISSIONS_HALON2402() {
return D_EMISSIONS_halon2402;
}

//' @describeIn haloemiss Emissions for chloromethane
//' @export
// [[Rcpp::export]]
String EMISSIONS_CH3CL() {
return D_EMISSIONS_CH3Cl;
}

//' @describeIn haloemiss Emissions for bromomethane
//' @export
// [[Rcpp::export]]
String EMISSIONS_CH3BR() {
return D_EMISSIONS_CH3Br;
}


/* Methane component */
//' @rdname methane
//' @export
// [[Rcpp::export]]
String ATMOSPHERIC_CH4() {
return D_ATMOSPHERIC_CH4;
}


//' @rdname methane
//' @export
// [[Rcpp::export]]
String PREINDUSTRIAL_CH4() {
return D_PREINDUSTRIAL_CH4;
}

//' @describeIn forcings Radiative forcing due to methane
//' @export
// [[Rcpp::export]]
String RF_CH4() {
return D_RF_CH4;
}

//' @rdname methane
//' @export
// [[Rcpp::export]]
String EMISSIONS_CH4() {
return D_EMISSIONS_CH4;
}

//' @rdname methane
//' @export
// [[Rcpp::export]]
String NATURAL_CH4() {
return D_NATURAL_CH4;
}

//' @rdname methane
//' @export
// [[Rcpp::export]]
String LIFETIME_SOIL() {
return D_LIFETIME_SOIL;
}

//' @rdname methane
//' @export
// [[Rcpp::export]]
String LIFETIME_STRAT() {
return D_LIFETIME_STRAT;
}

/* N2O component */
//' @describeIn concentrations Atmospheric N2O concentration
//' @export
// [[Rcpp::export]]
String ATMOSPHERIC_N2O() {
return D_ATMOSPHERIC_N2O;
}

//' @describeIn concentrations Preindustrial atmospheric N2O concentration
//' @export
// [[Rcpp::export]]
String PREINDUSTRIAL_N2O() {
return D_PREINDUSTRIAL_N2O;
}

//' @describeIn emissions N2O emissions (\code{"Tg N"})
//' @export
// [[Rcpp::export]]
String EMISSIONS_N2O() {
return D_EMISSIONS_N2O;
}



/* O3 component */
//' @describeIn concentrations Preindustrial ozone concentration
//' @export
// [[Rcpp::export]]
String PREINDUSTRIAL_O3() {
return D_PREINDUSTRIAL_O3;
}

//' @describeIn concentrations Atmospheric ozone concentration
//' @export
// [[Rcpp::export]]
String ATMOSPHERIC_O3() {
return D_ATMOSPHERIC_O3;
}

//' @describeIn emissions Emissions for NOx compounds (\code{"Tg N"})
//' @export
// [[Rcpp::export]]
String EMISSIONS_NOX() {
return D_EMISSIONS_NOX;
}

//' @describeIn emissions Emissions for carbon monoxide (\code{"Tg CO"})
//' @export
// [[Rcpp::export]]
String EMISSIONS_CO() {
return D_EMISSIONS_CO;
}

//' @describeIn emissions Emissions for non-methane volatile organic compounds (NMVOC) (\code{"Tg NMVOC"})
//' @export
// [[Rcpp::export]]
String EMISSIONS_NMVOC() {
return D_EMISSIONS_NMVOC;
}

/* OC component */
//' @describeIn emissions Emissions for organic carbon (\code{"Tg"})
//' @export
// [[Rcpp::export]]
String EMISSIONS_OC() {
return D_EMISSIONS_OC;
}

/* Ocean component */
//' @describeIn ocean Atmosphere-ocean carbon flux
//' @export
// [[Rcpp::export]]
String OCEAN_CFLUX() {
return D_OCEAN_CFLUX;
}

//' @describeIn ocean Ocean total carbon pool
//' @export
// [[Rcpp::export]]
String OCEAN_C() {
return D_OCEAN_C;
}

//' @describeIn ocean Ocean surface high-latitude carbon pool
//' @export
// [[Rcpp::export]]
String OCEAN_C_HL() {
return D_CARBON_HL;
}

//' @describeIn ocean Ocean surface low-latitude carbon pool
//' @export
// [[Rcpp::export]]
String OCEAN_C_LL() {
return D_CARBON_LL;
}

//' @describeIn ocean Intermediate ocean carbon pool
//' @export
// [[Rcpp::export]]
String OCEAN_C_IO() {
return D_CARBON_IO;
}

//' @describeIn ocean Deep ocean carbon pool
//' @export
// [[Rcpp::export]]
String OCEAN_C_DO() {
return D_CARBON_DO;
}


//' @describeIn ocean Thermohaline overturning
//' @export
// [[Rcpp::export]]
String TT() {
return D_TT;
}

//' @describeIn ocean High-latitude overturning
//' @export
// [[Rcpp::export]]
String TU() {
return D_TU;
}

//' @describeIn ocean Warm-intermediate exchange
//' @export
// [[Rcpp::export]]
String TWI() {
return D_TWI;
}

//' @describeIn ocean Intermediate-deep exchange
//' @export
// [[Rcpp::export]]
String TID() {
return D_TID;
}

// The rest of these aren't yet implemented, but we might want to have them some day

//' @describeIn ocean High-latitude Ph
//' @export
// [[Rcpp::export]]
String PH_HL() {
return D_PH_HL;
}

//' @describeIn ocean Low-latitude Ph
//' @export
// [[Rcpp::export]]
String PH_LL() {
return D_PH_LL;
}

//' @describeIn ocean Atmosphere-ocean carbon flux, high-latitude
//' @export
// [[Rcpp::export]]
String ATM_OCEAN_FLUX_HL() {
return D_ATM_OCEAN_FLUX_HL;
}

//' @describeIn ocean Atmosphere-ocean carbon flux, low-latitude
//' @export
// [[Rcpp::export]]
String ATM_OCEAN_FLUX_LL() {
return D_ATM_OCEAN_FLUX_LL;
}

//' @describeIn ocean Partial pressure of CO2, high-latitude
//' @export
// [[Rcpp::export]]
String PCO2_HL() {
return D_PCO2_HL;
}

//' @describeIn ocean Partial pressure of CO2, low-latitude
//' @export
// [[Rcpp::export]]
String PCO2_LL() {
return D_PCO2_LL;
}

//' @describeIn ocean Dissolved inorganic carbon, high-latitude
//' @export
// [[Rcpp::export]]
String DIC_HL() {
return D_DIC_HL;
}

//' @describeIn ocean Dissolved inorganic carbon, low-latitude
//' @export
// [[Rcpp::export]]
String DIC_LL() {
return D_DIC_LL;
}

//' @describeIn ocean Ocean temperature, high-latitude
//' @export
// [[Rcpp::export]]
String TEMP_HL() {
return D_TEMP_HL;
}

//' @describeIn ocean Ocean temperature, low-latitude
//' @export
// [[Rcpp::export]]
String TEMP_LL() {
return D_TEMP_LL;
}

//' @describeIn ocean Carbonate concentration, low-latitude
//' @export
// [[Rcpp::export]]
String CO3_LL() {
return D_CO3_LL;
}

//' @describeIn ocean Carbonate concentration, high-latitude
//' @export
// [[Rcpp::export]]
String CO3_HL() {
return D_CO3_HL;
}



/* Simple Nbox Component */
//' @rdname carboncycle
//' @export
// [[Rcpp::export]]
String LAND_CFLUX() {
return D_LAND_CFLUX;
}

//' @rdname carboncycle
//' @export
// [[Rcpp::export]]
String ATMOSPHERIC_CO2() {
return D_ATMOSPHERIC_CO2;
}

//' @rdname carboncycle
//' @export
// [[Rcpp::export]]
String NPP() {
    return D_NPP;
}

//' @describeIn parameters Preindustrial CO2 concentration (\code{"ppmv CO2"})
//' @export
// [[Rcpp::export]]
String PREINDUSTRIAL_CO2() {
return D_PREINDUSTRIAL_CO2;
}

//' @rdname carboncycle
//' @export
// [[Rcpp::export]]
String ATMOSPHERIC_C() {
return D_ATMOSPHERIC_C;
}

//' @rdname carboncycle
//' @export
// [[Rcpp::export]]
String FFI_EMISSIONS() {
return D_FFI_EMISSIONS;
}

//' @rdname carboncycle
//' @export
// [[Rcpp::export]]
String LUC_EMISSIONS() {
return D_LUC_EMISSIONS;
}

//' @describeIn parameters CO2 fertilization factor (\code{"(unitless)"})
//' @param biome Biome for which to retrieve parameter. If missing or
//'   `""`, default to `"global"`.
//' @export
// [[Rcpp::export]]
String BETA(String biome = "") {
  if (biome == "") return D_BETA;
  // `Rcpp::String` has a `+=` method, but no `+` method, so have to use
  // this clunky workaround.
  String out = biome;
  out += ".";
  out += D_BETA;
  return out;
}

//' @describeIn parameters Heterotrophic respiration temperature sensitivity factor (\code{"(unitless)"})
//' @inheritParams BETA
//' @export
// [[Rcpp::export]]
String Q10_RH(String biome = "") {
  if (biome == "") return D_Q10_RH;
  String out = biome;
  out += ".";
  out += D_Q10_RH;
  return out;
}


//' @describeIn parameters Biome-specific warming factor (`(unitless)`)
//' @inheritParams BETA
//' @export
// [[Rcpp::export]]
String WARMINGFACTOR(String biome = "") {
  if (biome == "") return D_WARMINGFACTOR;
  String out = biome;
  out += ".";
  out += D_WARMINGFACTOR;
  return out;
}

//' @describeIn parameters NPP fraction to vegetation (\code{"(unitless)"})
//' @inheritParams BETA
//' @export
// [[Rcpp::export]]
String F_NPPV(String biome = "") {
  if (biome == "") return D_F_NPPV;
  String out = biome;
  out += ".";
  out += D_F_NPPV;
  return out;
}

//' @describeIn parameters NPP fraction to detritus (\code{"(unitless)"})
//' @inheritParams BETA
//' @export
// [[Rcpp::export]]
String F_NPPD(String biome = "") {
  if (biome == "") return D_F_NPPD;
  String out = biome;
  out += ".";
  out += D_F_NPPD;
  return out;
}

//' @describeIn parameters Litter fraction to detritus (\code{"(unitless)"})
//' @inheritParams BETA
//' @export
// [[Rcpp::export]]
String F_LITTERD(String biome = "") {
  if (biome == "") return D_F_LITTERD;
  String out = biome;
  out += ".";
  out += D_F_LITTERD;
  return out;
}

//' @describeIn parameters LUC fraction to vegetation (\code{"(unitless)"})
//' @export
// [[Rcpp::export]]
String F_LUCV() {
return D_F_LUCV;
}

//' @describeIn parameters LUC fraction to detritus (\code{"(unitless)"})
//' @export
// [[Rcpp::export]]
String F_LUCD() {
return D_F_LUCD;
}

//' @describeIn carboncycle Vegetation C pool (`"Pg C"`)
//' @inheritParams BETA
//' @export
// [[Rcpp::export]]
String VEG_C(String biome = "") {
  if (biome == "") return D_VEGC;
  String out = biome;
  out += ".";
  out += D_VEGC;
  return out;
}

//' @describeIn carboncycle Vegetation detritus C pool (`"Pg C"`)
//' @inheritParams BETA
//' @export
// [[Rcpp::export]]
String DETRITUS_C(String biome = "") {
  if (biome == "") return D_DETRITUSC;
  String out = biome;
  out += ".";
  out += D_DETRITUSC;
  return out;
}

//' @describeIn carboncycle Soil C pool (`"Pg C"`)
//' @inheritParams BETA
//' @export
// [[Rcpp::export]]
String SOIL_C(String biome = "") {
  if (biome == "") return D_SOILC;
  String out = biome;
  out += ".";
  out += D_SOILC;
  return out;
}

//' @describeIn carboncycle Initial net primary productivity (NPP)
//'   flux (`"Pg C year^-1"`)
//' @inheritParams BETA
//' @export
// [[Rcpp::export]]
String NPP_FLUX0(String biome = "") {
  if (biome == "") return D_NPP_FLUX0;
  String out = biome;
  out += ".";
  out += D_NPP_FLUX0;
  return out;
}

/* SLR component */
// // Apparently not yet implemented
// //' @export
// // [[Rcpp::export]]
// String SL_RC() {
// return D_SL_RC;
// }

// //' @export
// // [[Rcpp::export]]
// String SLR() {
// return D_SLR;
// }

// //' @export
// // [[Rcpp::export]]
// String SL_RC_NO_ICE() {
// return D_SL_RC_NO_ICE;
// }

// //' @export
// // [[Rcpp::export]]
// String SLR_NO_ICE() {
// return D_SLR_NO_ICE;
// }

/* SO2 component */
//' @rdname so2
//' @export
// [[Rcpp::export]]
String NATURAL_SO2() {
return D_NATURAL_SO2;
}

//' @rdname so2
//' @export
// [[Rcpp::export]]
String Y2000_SO2() {
return D_2000_SO2;
}

//' @rdname so2
//' @export
// [[Rcpp::export]]
String EMISSIONS_SO2() {
return D_EMISSIONS_SO2;
}

//' @rdname so2
//' @export
// [[Rcpp::export]]
String VOLCANIC_SO2() {
return D_VOLCANIC_SO2;
}

/* Temperature component */
//' @describeIn parameters Equilibrium Climate Sensitivity (\code{"degC"})
//' @export
// [[Rcpp::export]]
String ECS() {
return D_ECS;
}

//' @describeIn parameters Aerosol forcing scaling factor (\code{"(unitless)"})
//' @export
// [[Rcpp::export]]
String AERO_SCALE() {
  return D_AERO_SCALE;
}

//' @describeIn parameters Volcanic forcing scaling factor (\code{"(unitless)"})
//' @export
// [[Rcpp::export]]
String VOLCANIC_SCALE() {
return D_VOLCANIC_SCALE;
}

//' @describeIn temperature Global mean temperature
//' @export
// [[Rcpp::export]]
String GLOBAL_TEMP() {
return D_GLOBAL_TEMP;
}

//' @describeIn temperature Equilibrium global temperature
//' @export
// [[Rcpp::export]]
String GLOBAL_TEMPEQ() {
return D_GLOBAL_TEMPEQ;
}

//' @describeIn temperature Average ocean surface temperature anomaly
//' @export
// [[Rcpp::export]]
String OCEAN_SURFACE_TEMP() {
return D_OCEAN_SURFACE_TEMP;
}

//' @describeIn temperature Average ocean air temperature anomaly
//' @export
// [[Rcpp::export]]
String OCEAN_AIR_TEMP() {
return D_OCEAN_AIR_TEMP;
}

//' @describeIn temperature Average land temperature anomaly
//' @export
// [[Rcpp::export]]
String LAND_AIR_TEMP() {
return D_LAND_AIR_TEMP;
}


//' @describeIn parameters Ocean heat diffusivity (\code{"cm2/s"})
//' @export
// [[Rcpp::export]]
String DIFFUSIVITY() {
return D_DIFFUSIVITY;
}

//' @describeIn temperature Heat flux into the mixed layer of the ocean
//' @export
// [[Rcpp::export]]
String FLUX_MIXED() {
return D_FLUX_MIXED;
}

//' @describeIn temperature Heat flux into the interior layer of the ocean
//' @export
// [[Rcpp::export]]
String FLUX_INTERIOR() {
return D_FLUX_INTERIOR;
}

//' @describeIn temperature Total heat flux into the ocean
//' @export
// [[Rcpp::export]]
String HEAT_FLUX() {
return D_HEAT_FLUX;
}

//' @describeIn msgtype Character used to separate biome from variable name 
// [[Rcpp::export]]
String BIOME_SPLIT_CHAR() {
return SNBOX_PARSECHAR;
}
