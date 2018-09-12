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
// [[Rcpp::export]]
String MSG_GETDATA() {
return M_GETDATA;
}

// [[Rcpp::export]]
String MSG_SETDATA() {
return M_SETDATA;
}


/*****************************************************************
 * Capabilities
 *****************************************************************/

/* BC component */
// [[Rcpp::export]]
String EMISSIONS_BC() {
return D_EMISSIONS_BC;
}

/* Forcing component */
// [[Rcpp::export]]
String RF_TOTAL() {
return D_RF_TOTAL;
}


// [[Rcpp::export]]
String RF_T_ALBEDO() {
return D_RF_T_ALBEDO;
}

// [[Rcpp::export]]
String RF_CO2() {
return D_RF_CO2;
}

// [[Rcpp::export]]
String RF_N2O() {
return D_RF_N2O;
}

// [[Rcpp::export]]
String RF_H2O() {
return D_RF_N2O;
}

// [[Rcpp::export]]
String RF_O3() {
return D_RF_O3;
}

// [[Rcpp::export]]
String RF_BC() {
return D_RF_BC;
}

// [[Rcpp::export]]
String RF_OC() {
return D_RF_OC;
}

// [[Rcpp::export]]
String RF_SO2D() {
return D_RF_SO2d;
}

// [[Rcpp::export]]
String RF_SO2I() {
return D_RF_SO2i;
}

// [[Rcpp::export]]
String RF_SO2() {
return D_RF_SO2;
}

// [[Rcpp::export]]
String RF_VOL() {
return D_RF_VOL;
}

/* Halocarbon forcings */
// [[Rcpp::export]]
String RF_HALOCARBONS() {
return D_RF_halocarbons;
}

// [[Rcpp::export]]
String RF_CF4() {
return D_RF_CF4;
}

// [[Rcpp::export]]
String RF_C2F6() {
return D_RF_C2F6;
}

// [[Rcpp::export]]
String RF_HFC23() {
return D_RF_HFC23;
}

// [[Rcpp::export]]
String RF_HFC32() {
return D_RF_HFC32;
}

// [[Rcpp::export]]
String RF_HFC4310() {
return D_RF_HFC4310;
}

// [[Rcpp::export]]
String RF_HFC125() {
return D_RF_HFC125;
}

// [[Rcpp::export]]
String RF_HFC134A() {
return D_RF_HFC134a;
}

// [[Rcpp::export]]
String RF_HFC143A() {
return D_RF_HFC143a;
}

// [[Rcpp::export]]
String RF_HFC227EA() {
return D_RF_HFC227ea;
}

// [[Rcpp::export]]
String RF_HFC245FA() {
return D_RF_HFC245fa;
}

// [[Rcpp::export]]
String RF_SF6() {
return D_RF_SF6;
}

// [[Rcpp::export]]
String RF_CFC11() {
return D_RF_CFC11;
}

// [[Rcpp::export]]
String RF_CFC12() {
return D_RF_CFC12;
}

// [[Rcpp::export]]
String RF_CFC113() {
return D_RF_CFC113;
}

// [[Rcpp::export]]
String RF_CFC114() {
return D_RF_CFC114;
}

// [[Rcpp::export]]
String RF_CFC115() {
return D_RF_CFC115;
}

// [[Rcpp::export]]
String RF_CCL4() {
return D_RF_CCl4;
}

// [[Rcpp::export]]
String RF_CH3CCL3() {
return D_RF_CH3CCl3;
}

// [[Rcpp::export]]
String RF_HCF22() {
return D_RF_HCF22;
}

// [[Rcpp::export]]
String RF_HCF141B() {
return D_RF_HCF141b;
}

// [[Rcpp::export]]
String RF_HCF142B() {
return D_RF_HCF142b;
}

// [[Rcpp::export]]
String RF_HALON1211() {
return D_RF_halon1211;
}

// [[Rcpp::export]]
String RF_HALON1301() {
return D_RF_halon1301;
}

// [[Rcpp::export]]
String RF_HALON2402() {
return D_RF_halon2402;
}

// [[Rcpp::export]]
String RF_CH3CL() {
return D_RF_CH3Cl;
}

// [[Rcpp::export]]
String RF_CH3BR() {
return D_RF_CH3Br;
}

/* halocarbon emissions */
// [[Rcpp::export]]
String EMISSIONS_CF4() {
return D_EMISSIONS_CF4;
}

// [[Rcpp::export]]
String EMISSIONS_C2F6() {
return D_EMISSIONS_C2F6;
}

// [[Rcpp::export]]
String EMISSIONS_HFC23() {
return D_EMISSIONS_HFC23;
}

// [[Rcpp::export]]
String EMISSIONS_HFC32() {
return D_EMISSIONS_HFC32;
}

// [[Rcpp::export]]
String EMISSIONS_HFC4310() {
return D_EMISSIONS_HFC4310;
}

// [[Rcpp::export]]
String EMISSIONS_HFC125() {
return D_EMISSIONS_HFC125;
}

// [[Rcpp::export]]
String EMISSIONS_HFC134A() {
return D_EMISSIONS_HFC134a;
}

// [[Rcpp::export]]
String EMISSIONS_HFC143A() {
return D_EMISSIONS_HFC143a;
}

// [[Rcpp::export]]
String EMISSIONS_HFC227EA() {
return D_EMISSIONS_HFC227ea;
}

// [[Rcpp::export]]
String EMISSIONS_HFC245FA() {
return D_EMISSIONS_HFC245fa;
}

// [[Rcpp::export]]
String EMISSIONS_SF6() {
return D_EMISSIONS_SF6;
}

// [[Rcpp::export]]
String EMISSIONS_CFC11() {
return D_EMISSIONS_CFC11;
}

// [[Rcpp::export]]
String EMISSIONS_CFC12() {
return D_EMISSIONS_CFC12;
}

// [[Rcpp::export]]
String EMISSIONS_CFC113() {
return D_EMISSIONS_CFC113;
}

// [[Rcpp::export]]
String EMISSIONS_CFC114() {
return D_EMISSIONS_CFC114;
}

// [[Rcpp::export]]
String EMISSIONS_CFC115() {
return D_EMISSIONS_CFC115;
}

// [[Rcpp::export]]
String EMISSIONS_CCL4() {
return D_EMISSIONS_CCl4;
}

// [[Rcpp::export]]
String EMISSIONS_CH3CCL3() {
return D_EMISSIONS_CH3CCl3;
}

// [[Rcpp::export]]
String EMISSIONS_HCF22() {
return D_EMISSIONS_HCF22;
}

// [[Rcpp::export]]
String EMISSIONS_HCF141B() {
return D_EMISSIONS_HCF141b;
}

// [[Rcpp::export]]
String EMISSIONS_HCF142B() {
return D_EMISSIONS_HCF142b;
}

// [[Rcpp::export]]
String EMISSIONS_HALON1211() {
return D_EMISSIONS_halon1211;
}

// [[Rcpp::export]]
String EMISSIONS_HALON1301() {
return D_EMISSIONS_halon1301;
}

// [[Rcpp::export]]
String EMISSIONS_HALON2402() {
return D_EMISSIONS_halon2402;
}

// [[Rcpp::export]]
String EMISSIONS_CH3CL() {
return D_EMISSIONS_CH3Cl;
}

// [[Rcpp::export]]
String EMISSIONS_CH3BR() {
return D_EMISSIONS_CH3Br;
}

/* Halocarbon misc */
// [[Rcpp::export]]
String PREINDUSTRIAL_HC() {
return D_PREINDUSTRIAL_HC;
}

// [[Rcpp::export]]
String HC_CONCENTRATION() {
return D_HC_CONCENTRATION;
}

// [[Rcpp::export]]
String HC_EMISSION() {
return D_HC_EMISSION;
}


/* Methane component */
// [[Rcpp::export]]
String ATMOSPHERIC_CH4() {
return D_ATMOSPHERIC_CH4;
}

// [[Rcpp::export]]
String PREINDUSTRIAL_CH4() {
return D_PREINDUSTRIAL_CH4;
}

// [[Rcpp::export]]
String RF_CH4() {
return D_RF_CH4;
}

// [[Rcpp::export]]
String EMISSIONS_CH4() {
return D_EMISSIONS_CH4;
}

// [[Rcpp::export]]
String NATURAL_CH4() {
return D_NATURAL_CH4;
}

// [[Rcpp::export]]
String CONVERSION_CH4() {
return D_CONVERSION_CH4;
}

// [[Rcpp::export]]
String LIFETIME_SOIL() {
return D_LIFETIME_SOIL;
}

// [[Rcpp::export]]
String LIFETIME_STRAT() {
return D_LIFETIME_STRAT;
}

/* N2O component */
// [[Rcpp::export]]
String ATMOSPHERIC_N2O() {
return D_ATMOSPHERIC_N2O;
}

// [[Rcpp::export]]
String PREINDUSTRIAL_N2O() {
return D_PREINDUSTRIAL_N2O;
}

// [[Rcpp::export]]
String EMISSIONS_N2O() {
return D_EMISSIONS_N2O;
}

// [[Rcpp::export]]
String NAT_EMISSIONS_N2O() {
return D_NAT_EMISSIONS_N2O;
}

// [[Rcpp::export]]
String CONVERSION_N2O() {
return D_CONVERSION_N2O;
}

// [[Rcpp::export]]
String INITIAL_LIFETIME_N2O() {
return D_INITIAL_LIFETIME_N2O;
}

// [[Rcpp::export]]
String LIFETIME_N2O() {
return D_LIFETIME_N2O;
}

/* OH component */
// [[Rcpp::export]]
String LIFETIME_OH() {
return D_LIFETIME_OH;
}

// [[Rcpp::export]]
String INITIAL_LIFETIME_OH() {
return D_INITIAL_LIFETIME_OH;
}

// [[Rcpp::export]]
String COEFFICIENT_NOX() {
return D_COEFFICENT_NOX;
}

// [[Rcpp::export]]
String COEFFICIENT_CH4() {
return D_COEFFICENT_CH4;
}

// [[Rcpp::export]]
String COEFFICIENT_NMVOC() {
return D_COEFFICENT_NMVOC;
}

// [[Rcpp::export]]
String COEFFICIENT_CO() {
return D_COEFFICENT_CO;
}

/* O3 COMPONENT */
// [[Rcpp::export]]
String PREINDUSTRIAL_O3() {
return D_PREINDUSTRIAL_O3;
}

// [[Rcpp::export]]
String ATMOSPHERIC_O3() {
return D_ATMOSPHERIC_O3;
}

// [[Rcpp::export]]
String ATMOSPHERIC_EM_CH4() {
return D_ATMOSPHERIC_EM_CH4;
}

// [[Rcpp::export]]
String EMISSIONS_NOX() {
return D_EMISSIONS_NOX;
}

// [[Rcpp::export]]
String EMISSIONS_CO() {
return D_EMISSIONS_CO;
}

// [[Rcpp::export]]
String EMISSIONS_NMVOC() {
return D_EMISSIONS_NMVOC;
}

/* OC component */
// [[Rcpp::export]]
String EMISSIONS_OC() {
return D_EMISSIONS_OC;
}

/* Ocean component */
// [[Rcpp::export]]
String OCEAN_CFLUX() {
return D_OCEAN_CFLUX;
}

// [[Rcpp::export]]
String OCEAN_C() {
return D_OCEAN_C;
}

// [[Rcpp::export]]
String TT() {
return D_TT;
}

// [[Rcpp::export]]
String TU() {
return D_TU;
}

// [[Rcpp::export]]
String TWI() {
return D_TWI;
}

// [[Rcpp::export]]
String TID() {
return D_TID;
}

// [[Rcpp::export]]
String CIRC_TOPT() {
return D_CIRC_TOPT;
}

// [[Rcpp::export]]
String CIRC_T50_HIGH() {
return D_CIRC_T50_HIGH;
}

// [[Rcpp::export]]
String CIRC_T50_LOW() {
return D_CIRC_T50_LOW;
}

// [[Rcpp::export]]
String HL_DO() {
return D_HL_DO;
}

// [[Rcpp::export]]
String PH_HL() {
return D_PH_HL;
}

// [[Rcpp::export]]
String PH_LL() {
return D_PH_LL;
}

// [[Rcpp::export]]
String ATM_OCEAN_FLUX_HL() {
return D_ATM_OCEAN_FLUX_HL;
}

// [[Rcpp::export]]
String ATM_OCEAN_FLUX_LL() {
return D_ATM_OCEAN_FLUX_LL;
}

// [[Rcpp::export]]
String PCO2_HL() {
return D_PCO2_HL;
}

// [[Rcpp::export]]
String PCO2_LL() {
return D_PCO2_LL;
}

// [[Rcpp::export]]
String DIC_HL() {
return D_DIC_HL;
}

// [[Rcpp::export]]
String DIC_LL() {
return D_DIC_LL;
}

// [[Rcpp::export]]
String P0_LL() {
return D_P0_LL;
}

// [[Rcpp::export]]
String CARBON_HL() {
return D_CARBON_HL;
}

// [[Rcpp::export]]
String CARBON_LL() {
return D_CARBON_LL;
}

// [[Rcpp::export]]
String CARBON_IO() {
return D_CARBON_IO;
}

// [[Rcpp::export]]
String CARBON_DO() {
return D_CARBON_DO;
}

// [[Rcpp::export]]
String OMEGACA_HL() {
return D_OMEGACA_HL;
}

// [[Rcpp::export]]
String OMEGACA_LL() {
return D_OMEGACA_LL;
}

// [[Rcpp::export]]
String OMEGAAR_HL() {
return D_OMEGAAR_HL;
}

// [[Rcpp::export]]
String OMEGAAR_LL() {
return D_OMEGAAR_LL;
}

// [[Rcpp::export]]
String TEMP_HL() {
return D_TEMP_HL;
}

// [[Rcpp::export]]
String TEMP_LL() {
return D_TEMP_LL;
}

// [[Rcpp::export]]
String SPINUP_CHEM() {
return D_SPINUP_CHEM;
}

// [[Rcpp::export]]
String CO3_LL() {
return D_CO3_LL;
}

// [[Rcpp::export]]
String CO3_HL() {
return D_CO3_HL;
}

// [[Rcpp::export]]
String ATM_OCEAN_CONSTRAIN() {
return D_ATM_OCEAN_CONSTRAIN;
}

// [[Rcpp::export]]
String TIMESTEPS() {
return D_TIMESTEPS;
}

// [[Rcpp::export]]
String REVELLE_HL() {
return D_REVELLE_HL;
}

// [[Rcpp::export]]
String REVELLE_LL() {
return D_REVELLE_LL;
}


/* Simple Nbox Component */
// [[Rcpp::export]]
String LAND_CFLUX() {
return D_LAND_CFLUX;
}

// [[Rcpp::export]]
String ATMOSPHERIC_CO2() {
return D_ATMOSPHERIC_CO2;
}

// [[Rcpp::export]]
String PREINDUSTRIAL_CO2() {
return D_PREINDUSTRIAL_CO2;
}

// [[Rcpp::export]]
String ATMOSPHERIC_C() {
return D_ATMOSPHERIC_C;
}

// [[Rcpp::export]]
String ATMOSPHERIC_C_RESIDUAL() {
return D_ATMOSPHERIC_C_RESIDUAL;
}

// [[Rcpp::export]]
String EARTHC() {
return D_EARTHC;
}

// [[Rcpp::export]]
String VEGC() {
return D_VEGC;
}

// [[Rcpp::export]]
String DETRITUSC() {
return D_DETRITUSC;
}

// [[Rcpp::export]]
String SOILC() {
return D_SOILC;
}

// [[Rcpp::export]]
String FFI_EMISSIONS() {
return D_FFI_EMISSIONS;
}

// [[Rcpp::export]]
String LUC_EMISSIONS() {
return D_LUC_EMISSIONS;
}

// [[Rcpp::export]]
String CO2FERT() {
return D_CO2FERT;
}

// [[Rcpp::export]]
String TEMPFERTD() {
return D_TEMPFERTD;
}

// [[Rcpp::export]]
String TEMPFERTS() {
return D_TEMPFERTS;
}

// [[Rcpp::export]]
String Q10_RH() {
return D_Q10_RH;
}

// [[Rcpp::export]]
String NPP() {
return D_NPP;
}

// [[Rcpp::export]]
String RH() {
return D_RH;
}

// [[Rcpp::export]]
String F_NPPV() {
return D_F_NPPV;
}

// [[Rcpp::export]]
String F_NPPD() {
return D_F_NPPD;
}

// [[Rcpp::export]]
String F_LITTERD() {
return D_F_LITTERD;
}

// [[Rcpp::export]]
String F_LUCV() {
return D_F_LUCV;
}

// [[Rcpp::export]]
String F_LUCD() {
return D_F_LUCD;
}

// [[Rcpp::export]]
String NPP_FLUX0() {
return D_NPP_FLUX0;
}

// [[Rcpp::export]]
String CA_CONSTRAIN() {
return D_CA_CONSTRAIN;
}

// [[Rcpp::export]]
String BETA() {
return D_BETA;
}

// [[Rcpp::export]]
String WARMINGFACTOR() {
return D_WARMINGFACTOR;
}

/* SLR component */
// [[Rcpp::export]]
String SL_RC() {
return D_SL_RC;
}

// [[Rcpp::export]]
String SLR() {
return D_SLR;
}

// [[Rcpp::export]]
String SL_RC_NO_ICE() {
return D_SL_RC_NO_ICE;
}

// [[Rcpp::export]]
String SLR_NO_ICE() {
return D_SLR_NO_ICE;
}

/* SO2 component */
// [[Rcpp::export]]
String NATURAL_SO2() {
return D_NATURAL_SO2;
}

// [[Rcpp::export]]
String D2000_SO2() {
return D_2000_SO2;
}

// [[Rcpp::export]]
String EMISSIONS_SO2() {
return D_EMISSIONS_SO2;
}

// [[Rcpp::export]]
String VOLCANIC_SO2() {
return D_VOLCANIC_SO2;
}

/* Temperature component */
// [[Rcpp::export]]
String ECS() {
return D_ECS;
}

// [[Rcpp::export]]
String GLOBAL_TEMP() {
return D_GLOBAL_TEMP;
}

// [[Rcpp::export]]
String GLOBAL_TEMPEQ() {
return D_GLOBAL_TEMPEQ;
}

// [[Rcpp::export]]
String TGAV_CONSTRAIN() {
return D_TGAV_CONSTRAIN;
}

// [[Rcpp::export]]
String SO2D_B() {
return D_SO2D_B;
}

// [[Rcpp::export]]
String SO2I_B() {
return D_SO2I_B;
}

// [[Rcpp::export]]
String OC_B() {
return D_OC_B;
}

// [[Rcpp::export]]
String BC_B() {
return D_BC_B;
}

// [[Rcpp::export]]
String DIFFUSIVITY() {
return D_DIFFUSIVITY;
}

// [[Rcpp::export]]
String AERO_SCALE() {
return D_AERO_SCALE;
}

// [[Rcpp::export]]
String FLUX_MIXED() {
return D_FLUX_MIXED;
}

// [[Rcpp::export]]
String FLUX_INTERIOR() {
return D_FLUX_INTERIOR;
}

// [[Rcpp::export]]
String HEAT_FLUX() {
return D_HEAT_FLUX;
}

