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

//' @describeIn msgtype Character used to separate biome from variable name
// [[Rcpp::export]]
String BIOME_SPLIT_CHAR() {
   return SNBOX_PARSECHAR;
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

/* Core */
//' @describeIn parameters Start of carbon tracking (Year)
//' @export
// [[Rcpp::export]]
String TRACKING_DATE() {
return D_TRACKING_DATE;
}

/* BC component */
//' @describeIn emissions Black carbon emissions
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

//' @describeIn constraints Constrain total radiative forcing
//' @export
// [[Rcpp::export]]
String FTOT_CONSTRAIN() {
    return D_FTOT_CONSTRAIN;
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

//' @describeIn delta the foring tropospheric adjustments for N2O
//' @export
// [[Rcpp::export]]
String DELTA_CO2() {
    return D_DELTA_CO2;
}

//' @describeIn delta the foring tropospheric adjustments for N2O
//' @export
// [[Rcpp::export]]
String RF_N2O() {
return D_RF_N2O;
}

//' @describeIn delta Radiative forcing due to N2O
//' @export
// [[Rcpp::export]]
String DELTA_N2O() {
    return D_DELTA_N2O;
}

//' @describeIn forcings Radiative forcing due to stratospheric water vapor
//' @export
// [[Rcpp::export]]
String RF_H2O_STRAT() {
return D_RF_H2O_STRAT;
}

//' @describeIn forcings Radiative forcing due to tropospheric ozone
//' @export
// [[Rcpp::export]]
String RF_O3_TROP() {
return D_RF_O3_TROP;
}

//' @describeIn forcings Radiative forcing due to black carbon aerosol-radiation interactions
//' @export
// [[Rcpp::export]]
String RF_BC() {
return D_RF_BC;
}

//' @describeIn rho a radiative forcing efficiency for BC aerosol-radiation interactions
//' @export
// [[Rcpp::export]]
String RHO_BC() {
    return D_RHO_BC;
}

//' @describeIn forcings Radiative forcing due to organic carbon aerosol-radiation interactions
//' @export
// [[Rcpp::export]]
String RF_OC() {
return D_RF_OC;
}

//' @describeIn rho a radiative forcing efficiency for OC aerosol-radiation interactions
//' @export
// [[Rcpp::export]]
String RHO_OC() {
    return D_RHO_OC;
}

//' @describeIn forcings Radiative forcing due to NH3 aerosol-radiation interactions
//' @export
// [[Rcpp::export]]
String RF_NH3() {
    return D_RF_NH3;
}

//' @describeIn rho a radiative forcing efficiency for NH3 aerosol-radiation interactions
//' @export
// [[Rcpp::export]]
String RHO_NH3() {
    return D_RHO_NH3;
}

//' @describeIn forcings Radiative forcing due to SO2 aerosol-radiation interactions
//' @export
// [[Rcpp::export]]
String RF_SO2() {
return D_RF_SO2;
}

//' @describeIn forcings aerosol-cloud interactions, includes contributions from SO2, BC and OC
//' @export
// [[Rcpp::export]]
String RF_ACI() {
    return D_RF_ACI;
}

//' @describeIn rho a radiative forcing efficiency for SO2
//' @export
// [[Rcpp::export]]
String RHO_SO2() {
    return D_RHO_SO2;
}

//' @describeIn forcings Radiative forcing due to volcanic activity
//' @export
// [[Rcpp::export]]
String RF_VOL() {
return D_RF_VOL;
}

//' @describeIn forcings Radiative miscellaneous forcing read in from ini.
//' @export
// [[Rcpp::export]]
String RF_MISC() {
    return D_RF_MISC;
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
String RF_HCFC22() {
return D_RFADJ_HCFC22;
}

//' @describeIn haloforcings Radiative forcing due to HCFC-141b
//' @export
// [[Rcpp::export]]
String RF_HCFC141B() {
return D_RFADJ_HCFC141b;
}

//' @describeIn haloforcings Radiative forcing due to HCCFC-142b
//' @export
// [[Rcpp::export]]
String RF_HCFC142B() {
return D_RFADJ_HCFC142b;
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



/* halocarbon rho (radiative forcing efficiency) */
//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for CF4
//' @export
// [[Rcpp::export]]
String RHO_CF4() {
    return D_HCRHO_CF4;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for C2F6
//' @export
// [[Rcpp::export]]
String RHO_C2F6() {
    return D_HCRHO_C2F6;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for HFC-23
//' @export
// [[Rcpp::export]]
String RHO_HFC23() {
    return D_HCRHO_HFC23;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for HFC-32
//' @export
// [[Rcpp::export]]
String RHO_HFC32() {
    return D_HCRHO_HFC32;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for HFC-4310
//' @export
// [[Rcpp::export]]
String RHO_HFC4310() {
    return D_HCRHO_HFC4310;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for HFC-125
//' @export
// [[Rcpp::export]]
String RHO_HFC125() {
    return D_HCRHO_HFC125;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for HFC-134a
//' @export
// [[Rcpp::export]]
String RHO_HFC134A() {
    return D_HCRHO_HFC134a;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for HFC-143a
//' @export
// [[Rcpp::export]]
String RHO_HFC143A() {
    return D_HCRHO_HFC143a;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for HFC-227ea
//' @export
// [[Rcpp::export]]
String RHO_HFC227EA() {
    return D_HCRHO_HFC227ea;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for HFC-254fa
//' @export
// [[Rcpp::export]]
String RHO_HFC245FA() {
    return D_HCRHO_HFC245fa;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for sulfur hexafluoride
//' @export
// [[Rcpp::export]]
String RHO_SF6() {
    return D_HCRHO_SF6;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for CFC-11
//' @export
// [[Rcpp::export]]
String RHO_CFC11() {
    return D_HCRHO_CFC11;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for CFC-12
//' @export
// [[Rcpp::export]]
String RHO_CFC12() {
    return D_HCRHO_CFC12;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for CFC-113
//' @export
// [[Rcpp::export]]
String RHO_CFC113() {
    return D_HCRHO_CFC113;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for CFC-114
//' @export
// [[Rcpp::export]]
String RHO_CFC114() {
    return D_HCRHO_CFC114;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for CFC-115
//' @export
// [[Rcpp::export]]
String RHO_CFC115() {
    return D_HCRHO_CFC115;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for carbon tetrachloride
//' @export
// [[Rcpp::export]]
String RHO_CCL4() {
    return D_HCRHO_CCl4;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for trichloroethane
//' @export
// [[Rcpp::export]]
String RHO_CH3CCL3() {
    return D_HCRHO_CH3CCl3;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for HFC-22
//' @export
// [[Rcpp::export]]
String RHO_HCFC22() {
    return D_HCRHO_HCFC22;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for HFC-141b
//' @export
// [[Rcpp::export]]
String RHO_HCFC141B() {
    return D_HCRHO_HCFC141b;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for HCFC-142b
//' @export
// [[Rcpp::export]]
String RHO_HCFC142B() {
    return D_HCRHO_HCFC142b;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for halon-1211
//' @export
// [[Rcpp::export]]
String RHO_HALON1211() {
    return D_HCRHO_halon1211;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for halon-1301
//' @export
// [[Rcpp::export]]
String RHO_HALON1301() {
    return D_HCRHO_halon1301;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for halon-2402
//' @export
// [[Rcpp::export]]
String RHO_HALON2402() {
    return D_HCRHO_halon2402;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for chloromethane
//' @export
// [[Rcpp::export]]
String RHO_CH3CL() {
    return D_HCRHO_CH3Cl;
}

//' @describeIn rho a radiative forcing efficiency for user-specified preindustrial concentration (Wm-2 pptv-1) for bromomethane
//' @export
// [[Rcpp::export]]
String RHO_CH3BR() {
    return D_HCRHO_CH3Br;
}

/* halocarbon delta (tropospheric adjustments for stratospheric-temperature adjusted radiative forcing) */
//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_CF4() {
    return D_HCDELTA_CF4;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_C2F6() {
    return D_HCDELTA_C2F6;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_HFC23() {
    return D_HCDELTA_HFC23;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_HFC32() {
    return D_HCDELTA_HFC32;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_HFC4310() {
    return D_HCDELTA_HFC4310;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_HFC125() {
    return D_HCDELTA_HFC125;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_HFC134A() {
    return D_HCDELTA_HFC134a;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_HFC143A() {
    return D_HCDELTA_HFC143a;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_HFC227EA() {
    return D_HCDELTA_HFC227ea;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_HFC245FA() {
    return D_HCDELTA_HFC245fa;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_SF6() {
    return D_HCDELTA_SF6;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_CFC11() {
    return D_HCDELTA_CFC11;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_CFC12() {
    return D_HCDELTA_CFC12;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_CFC113() {
    return D_HCDELTA_CFC113;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_CFC114() {
    return D_HCDELTA_CFC114;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_CFC115() {
    return D_HCDELTA_CFC115;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_CCL4() {
    return D_HCDELTA_CCl4;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_CH3CCL3() {
    return D_HCDELTA_CH3CCl3;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_HCFC22() {
    return D_HCDELTA_HCFC22;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_HCFC141B() {
    return D_HCDELTA_HCFC141b;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_HCFC142B() {
    return D_HCDELTA_HCFC142b;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_HALON1211() {
    return D_HCDELTA_halon1211;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_HALON1301() {
    return D_HCDELTA_halon1301;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_HALON2402() {
    return D_HCDELTA_halon2402;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_CH3CL() {
    return D_HCDELTA_CH3Cl;
}

//' @describeIn delta the tropospheric adjustments used to convert from stratospheric-temperature adjusted radiative forcing to effective radiative forcing
//' @export
// [[Rcpp::export]]
String DELTA_CH3BR() {
    return D_HCDELTA_CH3Br;
}

//' @describeIn forcings Radiative forcing efficiency for doubling of CO2
//' @export
// [[Rcpp::export]]
String Q_CO2() {
    return D_QCO2;
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
String EMISSIONS_HCFC22() {
return D_EMISSIONS_HCFC22;
}

//' @describeIn haloemiss Emissions for HcFC-141b
//' @export
// [[Rcpp::export]]
String EMISSIONS_HCFC141B() {
return D_EMISSIONS_HCFC141b;
}

//' @describeIn haloemiss Emissions for HCFC-142b
//' @export
// [[Rcpp::export]]
String EMISSIONS_HCFC142B() {
return D_EMISSIONS_HCFC142b;
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

/* halocarbon constraints */
//' @describeIn haloconstrain Concentration constraint for CF4
//' @export
// [[Rcpp::export]]
String CF4_CONSTRAIN() {
return D_CONSTRAINT_CF4;
}

//' @describeIn haloconstrain Concentration constraint for C2F6
//' @export
// [[Rcpp::export]]
String C2F6_CONSTRAIN() {
return D_CONSTRAINT_C2F6;
}

//' @describeIn haloconstrain Concentration constraint for HFC-23
//' @export
// [[Rcpp::export]]
String HFC23_CONSTRAIN() {
return D_CONSTRAINT_HFC23;
}

//' @describeIn haloconstrain Concentration constraint for HFC-32
//' @export
// [[Rcpp::export]]
String HFC32_CONSTRAIN() {
return D_CONSTRAINT_HFC32;
}

//' @describeIn haloconstrain Concentration constraint for HFC-4310
//' @export
// [[Rcpp::export]]
String HFC4310_CONSTRAIN() {
return D_CONSTRAINT_HFC4310;
}

//' @describeIn haloconstrain Concentration constraint for HFC-125
//' @export
// [[Rcpp::export]]
String HFC125_CONSTRAIN() {
return D_CONSTRAINT_HFC125;
}

//' @describeIn haloconstrain Concentration constraint for HFC-134a
//' @export
// [[Rcpp::export]]
String HFC134A_CONSTRAIN() {
return D_CONSTRAINT_HFC134a;
}

//' @describeIn haloconstrain Concentration constraint for HFC-143a
//' @export
// [[Rcpp::export]]
String HFC143A_CONSTRAIN() {
return D_CONSTRAINT_HFC143a;
}

//' @describeIn haloconstrain Concentration constraint for HFC-227ea
//' @export
// [[Rcpp::export]]
String HFC227EA_CONSTRAIN() {
return D_CONSTRAINT_HFC227ea;
}

//' @describeIn haloconstrain Concentration constraint for HFC-254fa
//' @export
// [[Rcpp::export]]
String HFC245FA_CONSTRAIN() {
return D_CONSTRAINT_HFC245fa;
}

//' @describeIn haloconstrain Concentration constraint for sulfur hexafluoride
//' @export
// [[Rcpp::export]]
String SF6_CONSTRAIN() {
return D_CONSTRAINT_SF6;
}

//' @describeIn haloconstrain Concentration constraint for CFC-11
//' @export
// [[Rcpp::export]]
String CFC11_CONSTRAIN() {
return D_CONSTRAINT_CFC11;
}

//' @describeIn haloconstrain Concentration constraint for CFC-12
//' @export
// [[Rcpp::export]]
String CFC12_CONSTRAIN() {
return D_CONSTRAINT_CFC12;
}

//' @describeIn haloconstrain Concentration constraint for CFC-113
//' @export
// [[Rcpp::export]]
String CFC113_CONSTRAIN() {
return D_CONSTRAINT_CFC113;
}

//' @describeIn haloconstrain Concentration constraint for CFC-114
//' @export
// [[Rcpp::export]]
String CFC114_CONSTRAIN() {
return D_CONSTRAINT_CFC114;
}

//' @describeIn haloconstrain Concentration constraint for CFC-115
//' @export
// [[Rcpp::export]]
String CFC115_CONSTRAIN() {
return D_CONSTRAINT_CFC115;
}

//' @describeIn haloconstrain Concentration constraint for carbon tetrachloride
//' @export
// [[Rcpp::export]]
String CCL4_CONSTRAIN() {
return D_CONSTRAINT_CCl4;
}

//' @describeIn haloconstrain Concentration constraint for trichloroethane
//' @export
// [[Rcpp::export]]
String CH3CCL3_CONSTRAIN() {
return D_CONSTRAINT_CH3CCl3;
}

//' @describeIn haloconstrain Concentration constraint for HCFC-22
//' @export
// [[Rcpp::export]]
String HCFC22_CONSTRAIN() {
return D_CONSTRAINT_HCFC22;
}

//' @describeIn haloconstrain Concentration constraint for HCFC-141b
//' @export
// [[Rcpp::export]]
String HCFC141B_CONSTRAIN() {
return D_CONSTRAINT_HCFC141b;
}

//' @describeIn haloconstrain Concentration constraint for HCFC-142b
//' @export
// [[Rcpp::export]]
String HCFC142B_CONSTRAIN() {
return D_CONSTRAINT_HCFC142b;
}

//' @describeIn haloconstrain Concentration constraint for halon-1211
//' @export
// [[Rcpp::export]]
String HALON1211_CONSTRAIN() {
return D_CONSTRAINT_halon1211;
}

//' @describeIn haloconstrain Concentration constraint for halon-1301
//' @export
// [[Rcpp::export]]
String HALON1301_CONSTRAIN() {
return D_CONSTRAINT_halon1301;
}

//' @describeIn haloconstrain Concentration constraint for halon-2402
//' @export
// [[Rcpp::export]]
String HALON2402_CONSTRAIN() {
return D_CONSTRAINT_halon2402;
}

//' @describeIn haloconstrain Concentration constraint for chloromethane
//' @export
// [[Rcpp::export]]
String CH3CL_CONSTRAIN() {
return D_CONSTRAINT_CH3Cl;
}

//' @describeIn haloconstrain Concentration constraint for bromomethane
//' @export
// [[Rcpp::export]]
String CH3BR_CONSTRAIN() {
return D_CONSTRAINT_CH3Br;
}


/* Methane component */
//' @rdname methane
//' @export
// [[Rcpp::export]]
String ATMOSPHERIC_CH4() {
return D_ATMOSPHERIC_CH4;
}

//' @describeIn constraints CH4 concentration constraint \code{"ppbv CH4"}
//' @export
// [[Rcpp::export]]
String CH4_CONSTRAIN() {
return D_CONSTRAINT_CH4;
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

//' @describeIn delta Radiative forcing tropospheric adjustment for CH4
//' @export
// [[Rcpp::export]]
String DELTA_CH4() {
    return D_DELTA_CH4;
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

//' @describeIn emissions Natural N2O emissions (\code{"Tg N"})
//' @export
// [[Rcpp::export]]
String NAT_EMISSIONS_N2O() {
return D_NAT_EMISSIONS_N2O;
}

//' @describeIn constraints N2O concentration constraint (\code{"pbbv N2O"})
//' @export
// [[Rcpp::export]]
String N2O_CONSTRAIN() {
return D_CONSTRAINT_N2O;
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

/* NH3 component */
//' @describeIn emissions Emissions for NH3 (\code{"Tg"})
//' @export
// [[Rcpp::export]]
String EMISSIONS_NH3() {
    return D_EMISSIONS_NH3;
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

//' @describeIn ocean Ocean surface or mixed layer carbon pool
//' @export
// [[Rcpp::export]]
String OCEAN_C_ML() {
    return D_CARBON_ML;
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

//' @describeIn ocean High-latitude pH
//' @export
// [[Rcpp::export]]
String PH_HL() {
return D_PH_HL;
}

//' @describeIn ocean Low-latitude pH
//' @export
// [[Rcpp::export]]
String PH_LL() {
return D_PH_LL;
}

//' @describeIn ocean Ocean surface pH
//' @export
// [[Rcpp::export]]
String PH() {
    return D_PH;
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

//' @describeIn ocean Ocean surface partial pressure of CO2
//' @export
// [[Rcpp::export]]
String PCO2() {
    return D_PCO2;
}

//' @describeIn ocean Dissolved inorganic carbon, high-latitude
//' @export
// [[Rcpp::export]]
String DIC_HL() {
return D_DIC_HL;
}

//' @describeIn ocean Ocean surface dissolved inorganic carbon
//' @export
// [[Rcpp::export]]
String DIC() {
    return D_DIC;
}

//' @describeIn ocean Dissolved inorganic carbon, low-latitude
//' @export
// [[Rcpp::export]]
String DIC_LL() {
return D_DIC_LL;
}

//' @describeIn ocean Absolute ocean surface temperature, high-latitude (deg C)
//' @export
// [[Rcpp::export]]
String TEMP_HL() {
return D_TEMP_HL;
}

//' @describeIn ocean Absolute ocean surface temperature, low-latitude (deg C)
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

//' @describeIn ocean Ocean surface carbonate concentration
//' @export
// [[Rcpp::export]]
String CO3() {
    return D_CO3;
}


/* Simple Nbox Component */
//' @rdname carboncycle
//' @export
// [[Rcpp::export]]
String LAND_CFLUX() {
return D_LAND_CFLUX;
}

// An alias for LAND_CFLUX that may be more familiar to carbon cycle people
//' @rdname carboncycle
//' @export
// [[Rcpp::export]]
String NBP() {
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

//' @rdname carboncycle
//' @export
// [[Rcpp::export]]
String RH() {
    return D_RH;
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
String DACCS_UPTAKE() {
   return D_DACCS_UPTAKE;
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
  out += BIOME_SPLIT_CHAR();
  out += D_BETA;
  return out;
}

//' @describeIn parameters Heterotrophic respiration temperature sensitivity factor (\code{"(unitless)"})
//' @export
// [[Rcpp::export]]
String Q10_RH(String biome = "") {
  if (biome == "") return D_Q10_RH;
  String out = biome;
  out += BIOME_SPLIT_CHAR();
  out += D_Q10_RH;
  return out;
}


//' @describeIn parameters Biome-specific warming factor (`(unitless)`)
//' @export
// [[Rcpp::export]]
String WARMINGFACTOR(String biome = "") {
  if (biome == "") return D_WARMINGFACTOR;
  String out = biome;
  out += BIOME_SPLIT_CHAR();
  out += D_WARMINGFACTOR;
  return out;
}

//' @describeIn carboncycle Constrain atmospheric CO2 concentration  (\code{"(ppmv CO2)"})
//' @export
// [[Rcpp::export]]
String CO2_CONSTRAIN() {
    return D_CO2_CONSTRAIN;
}

//' @describeIn parameters NPP fraction to vegetation (\code{"(unitless)"})
//' @export
// [[Rcpp::export]]
String F_NPPV(String biome = "") {
  if (biome == "") return D_F_NPPV;
  String out = biome;
  out += BIOME_SPLIT_CHAR();
  out += D_F_NPPV;
  return out;
}

//' @describeIn parameters NPP fraction to detritus (\code{"(unitless)"})
//' @export
// [[Rcpp::export]]
String F_NPPD(String biome = "") {
  if (biome == "") return D_F_NPPD;
  String out = biome;
  out += BIOME_SPLIT_CHAR();
  out += D_F_NPPD;
  return out;
}

//' @describeIn parameters Litter fraction to detritus (\code{"(unitless)"})
//' @export
// [[Rcpp::export]]
String F_LITTERD(String biome = "") {
  if (biome == "") return D_F_LITTERD;
  String out = biome;
  out += BIOME_SPLIT_CHAR();
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
//' @param biome Name of biome (leave empty for global)
//' @export
// [[Rcpp::export]]
String VEG_C(String biome = "") {
  if (biome == "") return D_VEGC;
  String out = biome;
  out += BIOME_SPLIT_CHAR();
  out += D_VEGC;
  return out;
}

//' @describeIn carboncycle Vegetation detritus C pool (`"Pg C"`)
//' @param biome Name of biome (leave empty for global)
//' @export
// [[Rcpp::export]]
String DETRITUS_C(String biome = "") {
  if (biome == "") return D_DETRITUSC;
  String out = biome;
  out += BIOME_SPLIT_CHAR();
  out += D_DETRITUSC;
  return out;
}

//' @describeIn carboncycle Soil C pool (`"Pg C"`)
//' @param biome Name of biome (leave empty for global)
//' @export
// [[Rcpp::export]]
String SOIL_C(String biome = "") {
  if (biome == "") return D_SOILC;
  String out = biome;
  out += BIOME_SPLIT_CHAR();
  out += D_SOILC;
  return out;
}

//' @rdname carboncycle
//' @export
// [[Rcpp::export]]
String EARTH_C() {
   return D_EARTHC;
}

//' @describeIn carboncycle Initial net primary productivity (NPP)
//'   flux (`"Pg C year^-1"`)
//' @param biome Name of biome (leave empty for global)
//' @export
// [[Rcpp::export]]
String NPP_FLUX0(String biome = "") {
  if (biome == "") return D_NPP_FLUX0;
  String out = biome;
  out += BIOME_SPLIT_CHAR();
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

//' @describeIn temperature Global mean air temperature anomaly
//' @export
// [[Rcpp::export]]
String GLOBAL_TEMP() {
return D_GLOBAL_TEMP;
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

//' @describeIn temperature Average land temperature anomaly, land surface temperature and air temperature
//' over land are assumed to be equivalent.
//' @export
// [[Rcpp::export]]
String LAND_AIR_TEMP() {
return D_LAND_AIR_TEMP;
}

//' @describeIn parameters Land-Ocean Warming Ratio (\code{"(unitless)"}), by default set to 0
//' meaning that the land ocean warming ratio is an emergent property of Hector's temperature
//' component otherwise the user defined land ocean warming ratio will be used.
//' @export
// [[Rcpp::export]]
String LO_WARMING_RATIO(){
    return D_LO_WARMING_RATIO;
}

//' @describeIn constraints Constrain global mean temperature  (\code{"(degC)"})
//' @export
// [[Rcpp::export]]
String TGAV_CONSTRAIN() {
    return D_TGAV_CONSTRAIN;
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
