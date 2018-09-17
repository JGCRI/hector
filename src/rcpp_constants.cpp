#include <Rcpp.h>
#include "hector.hpp"
#include "component_data.hpp"

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
//' @export
// [[Rcpp::export]]
String GETDATA() {
return M_GETDATA;
}

//' @export
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
//' @export
// [[Rcpp::export]]
String EMISSIONS_BC() {
return D_EMISSIONS_BC;
}

/* Forcing component */
//' @export
// [[Rcpp::export]]
String RF_TOTAL() {
return D_RF_TOTAL;
}


//' @export
// [[Rcpp::export]]
String RF_T_ALBEDO() {
return D_RF_T_ALBEDO;
}

//' @export
// [[Rcpp::export]]
String RF_CO2() {
return D_RF_CO2;
}

//' @export
// [[Rcpp::export]]
String RF_N2O() {
return D_RF_N2O;
}

//' @export
// [[Rcpp::export]]
String RF_H2O() {
return D_RF_N2O;
}

//' @export
// [[Rcpp::export]]
String RF_O3() {
return D_RF_O3;
}

//' @export
// [[Rcpp::export]]
String RF_BC() {
return D_RF_BC;
}

//' @export
// [[Rcpp::export]]
String RF_OC() {
return D_RF_OC;
}

//' @export
// [[Rcpp::export]]
String RF_SO2D() {
return D_RF_SO2d;
}

//' @export
// [[Rcpp::export]]
String RF_SO2I() {
return D_RF_SO2i;
}

//' @export
// [[Rcpp::export]]
String RF_SO2() {
return D_RF_SO2;
}

//' @export
// [[Rcpp::export]]
String RF_VOL() {
return D_RF_VOL;
}

/* Halocarbon forcings */
//' @export
// [[Rcpp::export]]
String RF_HALOCARBONS() {
return D_RF_halocarbons;
}

//' @export
// [[Rcpp::export]]
String RF_CF4() {
return D_RF_CF4;
}

//' @export
// [[Rcpp::export]]
String RF_C2F6() {
return D_RF_C2F6;
}

//' @export
// [[Rcpp::export]]
String RF_HFC23() {
return D_RF_HFC23;
}

//' @export
// [[Rcpp::export]]
String RF_HFC32() {
return D_RF_HFC32;
}

//' @export
// [[Rcpp::export]]
String RF_HFC4310() {
return D_RF_HFC4310;
}

//' @export
// [[Rcpp::export]]
String RF_HFC125() {
return D_RF_HFC125;
}

//' @export
// [[Rcpp::export]]
String RF_HFC134A() {
return D_RF_HFC134a;
}

//' @export
// [[Rcpp::export]]
String RF_HFC143A() {
return D_RF_HFC143a;
}

//' @export
// [[Rcpp::export]]
String RF_HFC227EA() {
return D_RF_HFC227ea;
}

//' @export
// [[Rcpp::export]]
String RF_HFC245FA() {
return D_RF_HFC245fa;
}

//' @export
// [[Rcpp::export]]
String RF_SF6() {
return D_RF_SF6;
}

//' @export
// [[Rcpp::export]]
String RF_CFC11() {
return D_RF_CFC11;
}

//' @export
// [[Rcpp::export]]
String RF_CFC12() {
return D_RF_CFC12;
}

//' @export
// [[Rcpp::export]]
String RF_CFC113() {
return D_RF_CFC113;
}

//' @export
// [[Rcpp::export]]
String RF_CFC114() {
return D_RF_CFC114;
}

//' @export
// [[Rcpp::export]]
String RF_CFC115() {
return D_RF_CFC115;
}

//' @export
// [[Rcpp::export]]
String RF_CCL4() {
return D_RF_CCl4;
}

//' @export
// [[Rcpp::export]]
String RF_CH3CCL3() {
return D_RF_CH3CCl3;
}

//' @export
// [[Rcpp::export]]
String RF_HCF22() {
return D_RF_HCF22;
}

//' @export
// [[Rcpp::export]]
String RF_HCF141B() {
return D_RF_HCF141b;
}

//' @export
// [[Rcpp::export]]
String RF_HCF142B() {
return D_RF_HCF142b;
}

//' @export
// [[Rcpp::export]]
String RF_HALON1211() {
return D_RF_halon1211;
}

//' @export
// [[Rcpp::export]]
String RF_HALON1301() {
return D_RF_halon1301;
}

//' @export
// [[Rcpp::export]]
String RF_HALON2402() {
return D_RF_halon2402;
}

//' @export
// [[Rcpp::export]]
String RF_CH3CL() {
return D_RF_CH3Cl;
}

//' @export
// [[Rcpp::export]]
String RF_CH3BR() {
return D_RF_CH3Br;
}

/* halocarbon emissions */
//' @export
// [[Rcpp::export]]
String EMISSIONS_CF4() {
return D_EMISSIONS_CF4;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_C2F6() {
return D_EMISSIONS_C2F6;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_HFC23() {
return D_EMISSIONS_HFC23;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_HFC32() {
return D_EMISSIONS_HFC32;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_HFC4310() {
return D_EMISSIONS_HFC4310;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_HFC125() {
return D_EMISSIONS_HFC125;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_HFC134A() {
return D_EMISSIONS_HFC134a;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_HFC143A() {
return D_EMISSIONS_HFC143a;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_HFC227EA() {
return D_EMISSIONS_HFC227ea;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_HFC245FA() {
return D_EMISSIONS_HFC245fa;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_SF6() {
return D_EMISSIONS_SF6;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_CFC11() {
return D_EMISSIONS_CFC11;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_CFC12() {
return D_EMISSIONS_CFC12;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_CFC113() {
return D_EMISSIONS_CFC113;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_CFC114() {
return D_EMISSIONS_CFC114;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_CFC115() {
return D_EMISSIONS_CFC115;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_CCL4() {
return D_EMISSIONS_CCl4;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_CH3CCL3() {
return D_EMISSIONS_CH3CCl3;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_HCF22() {
return D_EMISSIONS_HCF22;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_HCF141B() {
return D_EMISSIONS_HCF141b;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_HCF142B() {
return D_EMISSIONS_HCF142b;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_HALON1211() {
return D_EMISSIONS_halon1211;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_HALON1301() {
return D_EMISSIONS_halon1301;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_HALON2402() {
return D_EMISSIONS_halon2402;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_CH3CL() {
return D_EMISSIONS_CH3Cl;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_CH3BR() {
return D_EMISSIONS_CH3Br;
}

/* Halocarbon misc */
//' @export
// [[Rcpp::export]]
String PREINDUSTRIAL_HC() {
return D_PREINDUSTRIAL_HC;
}

//' @export
// [[Rcpp::export]]
String HC_CONCENTRATION() {
return D_HC_CONCENTRATION;
}

//' @export
// [[Rcpp::export]]
String HC_EMISSION() {
return D_HC_EMISSION;
}


/* Methane component */
//' @export
// [[Rcpp::export]]
String ATMOSPHERIC_CH4() {
return D_ATMOSPHERIC_CH4;
}

//' @export
// [[Rcpp::export]]
String PREINDUSTRIAL_CH4() {
return D_PREINDUSTRIAL_CH4;
}

//' @export
// [[Rcpp::export]]
String RF_CH4() {
return D_RF_CH4;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_CH4() {
return D_EMISSIONS_CH4;
}

//' @export
// [[Rcpp::export]]
String NATURAL_CH4() {
return D_NATURAL_CH4;
}

//' @export
// [[Rcpp::export]]
String CONVERSION_CH4() {
return D_CONVERSION_CH4;
}

//' @export
// [[Rcpp::export]]
String LIFETIME_SOIL() {
return D_LIFETIME_SOIL;
}

//' @export
// [[Rcpp::export]]
String LIFETIME_STRAT() {
return D_LIFETIME_STRAT;
}

/* N2O component */
//' @export
// [[Rcpp::export]]
String ATMOSPHERIC_N2O() {
return D_ATMOSPHERIC_N2O;
}

//' @export
// [[Rcpp::export]]
String PREINDUSTRIAL_N2O() {
return D_PREINDUSTRIAL_N2O;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_N2O() {
return D_EMISSIONS_N2O;
}

//' @export
// [[Rcpp::export]]
String NAT_EMISSIONS_N2O() {
return D_NAT_EMISSIONS_N2O;
}

//' @export
// [[Rcpp::export]]
String CONVERSION_N2O() {
return D_CONVERSION_N2O;
}

//' @export
// [[Rcpp::export]]
String INITIAL_LIFETIME_N2O() {
return D_INITIAL_LIFETIME_N2O;
}

//' @export
// [[Rcpp::export]]
String LIFETIME_N2O() {
return D_LIFETIME_N2O;
}

/* OH component */
//' @export
// [[Rcpp::export]]
String LIFETIME_OH() {
return D_LIFETIME_OH;
}

//' @export
// [[Rcpp::export]]
String INITIAL_LIFETIME_OH() {
return D_INITIAL_LIFETIME_OH;
}

//' @export
// [[Rcpp::export]]
String COEFFICIENT_NOX() {
return D_COEFFICENT_NOX;
}

//' @export
// [[Rcpp::export]]
String COEFFICIENT_CH4() {
return D_COEFFICENT_CH4;
}

//' @export
// [[Rcpp::export]]
String COEFFICIENT_NMVOC() {
return D_COEFFICENT_NMVOC;
}

//' @export
// [[Rcpp::export]]
String COEFFICIENT_CO() {
return D_COEFFICENT_CO;
}

/* O3 COMPONENT */
//' @export
// [[Rcpp::export]]
String PREINDUSTRIAL_O3() {
return D_PREINDUSTRIAL_O3;
}

//' @export
// [[Rcpp::export]]
String ATMOSPHERIC_O3() {
return D_ATMOSPHERIC_O3;
}

//' @export
// [[Rcpp::export]]
String ATMOSPHERIC_EM_CH4() {
return D_ATMOSPHERIC_EM_CH4;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_NOX() {
return D_EMISSIONS_NOX;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_CO() {
return D_EMISSIONS_CO;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_NMVOC() {
return D_EMISSIONS_NMVOC;
}

/* OC component */
//' @export
// [[Rcpp::export]]
String EMISSIONS_OC() {
return D_EMISSIONS_OC;
}

/* Ocean component */
//' @export
// [[Rcpp::export]]
String OCEAN_CFLUX() {
return D_OCEAN_CFLUX;
}

//' @export
// [[Rcpp::export]]
String OCEAN_C() {
return D_OCEAN_C;
}

//' @export
// [[Rcpp::export]]
String TT() {
return D_TT;
}

//' @export
// [[Rcpp::export]]
String TU() {
return D_TU;
}

//' @export
// [[Rcpp::export]]
String TWI() {
return D_TWI;
}

//' @export
// [[Rcpp::export]]
String TID() {
return D_TID;
}

//' @export
// [[Rcpp::export]]
String CIRC_TOPT() {
return D_CIRC_TOPT;
}

//' @export
// [[Rcpp::export]]
String CIRC_T50_HIGH() {
return D_CIRC_T50_HIGH;
}

//' @export
// [[Rcpp::export]]
String CIRC_T50_LOW() {
return D_CIRC_T50_LOW;
}

//' @export
// [[Rcpp::export]]
String HL_DO() {
return D_HL_DO;
}

//' @export
// [[Rcpp::export]]
String PH_HL() {
return D_PH_HL;
}

//' @export
// [[Rcpp::export]]
String PH_LL() {
return D_PH_LL;
}

//' @export
// [[Rcpp::export]]
String ATM_OCEAN_FLUX_HL() {
return D_ATM_OCEAN_FLUX_HL;
}

//' @export
// [[Rcpp::export]]
String ATM_OCEAN_FLUX_LL() {
return D_ATM_OCEAN_FLUX_LL;
}

//' @export
// [[Rcpp::export]]
String PCO2_HL() {
return D_PCO2_HL;
}

//' @export
// [[Rcpp::export]]
String PCO2_LL() {
return D_PCO2_LL;
}

//' @export
// [[Rcpp::export]]
String DIC_HL() {
return D_DIC_HL;
}

//' @export
// [[Rcpp::export]]
String DIC_LL() {
return D_DIC_LL;
}

//' @export
// [[Rcpp::export]]
String P0_LL() {
return D_P0_LL;
}

//' @export
// [[Rcpp::export]]
String CARBON_HL() {
return D_CARBON_HL;
}

//' @export
// [[Rcpp::export]]
String CARBON_LL() {
return D_CARBON_LL;
}

//' @export
// [[Rcpp::export]]
String CARBON_IO() {
return D_CARBON_IO;
}

//' @export
// [[Rcpp::export]]
String CARBON_DO() {
return D_CARBON_DO;
}

//' @export
// [[Rcpp::export]]
String OMEGACA_HL() {
return D_OMEGACA_HL;
}

//' @export
// [[Rcpp::export]]
String OMEGACA_LL() {
return D_OMEGACA_LL;
}

//' @export
// [[Rcpp::export]]
String OMEGAAR_HL() {
return D_OMEGAAR_HL;
}

//' @export
// [[Rcpp::export]]
String OMEGAAR_LL() {
return D_OMEGAAR_LL;
}

//' @export
// [[Rcpp::export]]
String TEMP_HL() {
return D_TEMP_HL;
}

//' @export
// [[Rcpp::export]]
String TEMP_LL() {
return D_TEMP_LL;
}

//' @export
// [[Rcpp::export]]
String SPINUP_CHEM() {
return D_SPINUP_CHEM;
}

//' @export
// [[Rcpp::export]]
String CO3_LL() {
return D_CO3_LL;
}

//' @export
// [[Rcpp::export]]
String CO3_HL() {
return D_CO3_HL;
}

//' @export
// [[Rcpp::export]]
String ATM_OCEAN_CONSTRAIN() {
return D_ATM_OCEAN_CONSTRAIN;
}

//' @export
// [[Rcpp::export]]
String TIMESTEPS() {
return D_TIMESTEPS;
}

//' @export
// [[Rcpp::export]]
String REVELLE_HL() {
return D_REVELLE_HL;
}

//' @export
// [[Rcpp::export]]
String REVELLE_LL() {
return D_REVELLE_LL;
}


/* Simple Nbox Component */
//' @export
// [[Rcpp::export]]
String LAND_CFLUX() {
return D_LAND_CFLUX;
}

//' @export
// [[Rcpp::export]]
String ATMOSPHERIC_CO2() {
return D_ATMOSPHERIC_CO2;
}

//' @export
// [[Rcpp::export]]
String PREINDUSTRIAL_CO2() {
return D_PREINDUSTRIAL_CO2;
}

//' @export
// [[Rcpp::export]]
String ATMOSPHERIC_C() {
return D_ATMOSPHERIC_C;
}

//' @export
// [[Rcpp::export]]
String ATMOSPHERIC_C_RESIDUAL() {
return D_ATMOSPHERIC_C_RESIDUAL;
}

//' @export
// [[Rcpp::export]]
String EARTHC() {
return D_EARTHC;
}

//' @export
// [[Rcpp::export]]
String VEGC() {
return D_VEGC;
}

//' @export
// [[Rcpp::export]]
String DETRITUSC() {
return D_DETRITUSC;
}

//' @export
// [[Rcpp::export]]
String SOILC() {
return D_SOILC;
}

//' @export
// [[Rcpp::export]]
String FFI_EMISSIONS() {
return D_FFI_EMISSIONS;
}

//' @export
// [[Rcpp::export]]
String LUC_EMISSIONS() {
return D_LUC_EMISSIONS;
}

//' @export
// [[Rcpp::export]]
String CO2FERT() {
return D_CO2FERT;
}

//' @export
// [[Rcpp::export]]
String TEMPFERTD() {
return D_TEMPFERTD;
}

//' @export
// [[Rcpp::export]]
String TEMPFERTS() {
return D_TEMPFERTS;
}

//' @export
// [[Rcpp::export]]
String Q10_RH() {
return D_Q10_RH;
}

//' @export
// [[Rcpp::export]]
String NPP() {
return D_NPP;
}

//' @export
// [[Rcpp::export]]
String RH() {
return D_RH;
}

//' @export
// [[Rcpp::export]]
String F_NPPV() {
return D_F_NPPV;
}

//' @export
// [[Rcpp::export]]
String F_NPPD() {
return D_F_NPPD;
}

//' @export
// [[Rcpp::export]]
String F_LITTERD() {
return D_F_LITTERD;
}

//' @export
// [[Rcpp::export]]
String F_LUCV() {
return D_F_LUCV;
}

//' @export
// [[Rcpp::export]]
String F_LUCD() {
return D_F_LUCD;
}

//' @export
// [[Rcpp::export]]
String NPP_FLUX0() {
return D_NPP_FLUX0;
}

//' @export
// [[Rcpp::export]]
String CA_CONSTRAIN() {
return D_CA_CONSTRAIN;
}

//' @export
// [[Rcpp::export]]
String BETA() {
return D_BETA;
}

//' @export
// [[Rcpp::export]]
String WARMINGFACTOR() {
return D_WARMINGFACTOR;
}

/* SLR component */
//' @export
// [[Rcpp::export]]
String SL_RC() {
return D_SL_RC;
}

//' @export
// [[Rcpp::export]]
String SLR() {
return D_SLR;
}

//' @export
// [[Rcpp::export]]
String SL_RC_NO_ICE() {
return D_SL_RC_NO_ICE;
}

//' @export
// [[Rcpp::export]]
String SLR_NO_ICE() {
return D_SLR_NO_ICE;
}

/* SO2 component */
//' @export
// [[Rcpp::export]]
String NATURAL_SO2() {
return D_NATURAL_SO2;
}

//' @export
// [[Rcpp::export]]
String D2000_SO2() {
return D_2000_SO2;
}

//' @export
// [[Rcpp::export]]
String EMISSIONS_SO2() {
return D_EMISSIONS_SO2;
}

//' @export
// [[Rcpp::export]]
String VOLCANIC_SO2() {
return D_VOLCANIC_SO2;
}

/* Temperature component */
//' @export
// [[Rcpp::export]]
String ECS() {
return D_ECS;
}

//' @export
// [[Rcpp::export]]
String GLOBAL_TEMP() {
return D_GLOBAL_TEMP;
}

//' @export
// [[Rcpp::export]]
String GLOBAL_TEMPEQ() {
return D_GLOBAL_TEMPEQ;
}

//' @export
// [[Rcpp::export]]
String TGAV_CONSTRAIN() {
return D_TGAV_CONSTRAIN;
}

//' @export
// [[Rcpp::export]]
String SO2D_B() {
return D_SO2D_B;
}

//' @export
// [[Rcpp::export]]
String SO2I_B() {
return D_SO2I_B;
}

//' @export
// [[Rcpp::export]]
String OC_B() {
return D_OC_B;
}

//' @export
// [[Rcpp::export]]
String BC_B() {
return D_BC_B;
}

//' @export
// [[Rcpp::export]]
String DIFFUSIVITY() {
return D_DIFFUSIVITY;
}

//' @export
// [[Rcpp::export]]
String AERO_SCALE() {
return D_AERO_SCALE;
}

//' @export
// [[Rcpp::export]]
String FLUX_MIXED() {
return D_FLUX_MIXED;
}

//' @export
// [[Rcpp::export]]
String FLUX_INTERIOR() {
return D_FLUX_INTERIOR;
}

//' @export
// [[Rcpp::export]]
String HEAT_FLUX() {
return D_HEAT_FLUX;
}

