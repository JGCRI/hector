/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef COMPONENT_DATA_H
#define COMPONENT_DATA_H
/*
 *  component_data.hpp
 *  hector
 *
 *  Created by Ben on 2/18/11.
 *
 */

#include "components/component_names.hpp"
// ------------------- names of data being passed between components -------------------------

// Note names in INI input need to match these.

// core
#define D_RUN_NAME              "run_name"
#define D_START_DATE            "startDate"
#define D_END_DATE              "endDate"
#define D_DO_SPINUP             "do_spinup"
#define D_MAX_SPINUP            "max_spinup"
#define D_ENABLED               "enabled"
#define D_OUTPUT_ENABLED        "output"

// bc component
#define D_EMISSIONS_BC          "BC_emissions"

// carbon cycle solver
#define D_CCS_EPS_ABS           "eps_abs"
#define D_CCS_EPS_REL           "eps_rel"
#define D_CCS_DT                "dt"
#define D_EPS_SPINUP            "eps_spinup"

// forcing component
#define D_RF_PREFIX             "F"
#define D_RF_TOTAL              D_RF_PREFIX "tot"
#define D_RF_T_ALBEDO           D_RF_PREFIX "talbedo"
#define D_FTOT_CONSTRAIN        D_RF_PREFIX "tot_constrain"
#define D_RF_BASEYEAR           "baseyear"
#define D_RF_CO2                D_RF_PREFIX "CO2"
#define D_RF_N2O                D_RF_PREFIX "N2O"
#define D_RF_H2O                D_RF_PREFIX "H2O"
#define D_RF_O3                 D_RF_PREFIX "O3"
#define D_RF_BC                 D_RF_PREFIX "BC"
#define D_RF_OC                 D_RF_PREFIX "OC"
#define D_RF_SO2d               D_RF_PREFIX "SO2d"
#define D_RF_SO2i               D_RF_PREFIX "SO2i"
#define D_RF_SO2                D_RF_PREFIX "SO2"
#define D_RF_VOL                D_RF_PREFIX "vol" // forcing from volcanic so2
#define D_RF_halocarbons        D_RF_PREFIX "halocarbons"

// halocarbon components
#define D_RF_CF4                D_RF_PREFIX CF4_COMPONENT_BASE
#define D_RF_C2F6               D_RF_PREFIX C2F6_COMPONENT_BASE
#define D_RF_HFC23              D_RF_PREFIX HFC23_COMPONENT_BASE
#define D_RF_HFC32              D_RF_PREFIX HFC32_COMPONENT_BASE
#define D_RF_HFC4310            D_RF_PREFIX HFC4310_COMPONENT_BASE
#define D_RF_HFC125             D_RF_PREFIX HFC125_COMPONENT_BASE
#define D_RF_HFC134a            D_RF_PREFIX HFC134a_COMPONENT_BASE
#define D_RF_HFC143a            D_RF_PREFIX HFC143a_COMPONENT_BASE
#define D_RF_HFC227ea           D_RF_PREFIX HFC227ea_COMPONENT_BASE
#define D_RF_HFC245fa           D_RF_PREFIX HFC245fa_COMPONENT_BASE
#define D_RF_SF6                D_RF_PREFIX SF6_COMPONENT_BASE
#define D_RF_CFC11              D_RF_PREFIX CFC11_COMPONENT_BASE
#define D_RF_CFC12              D_RF_PREFIX CFC12_COMPONENT_BASE
#define D_RF_CFC113             D_RF_PREFIX CFC113_COMPONENT_BASE
#define D_RF_CFC114             D_RF_PREFIX CFC114_COMPONENT_BASE
#define D_RF_CFC115             D_RF_PREFIX CFC115_COMPONENT_BASE
#define D_RF_CCl4               D_RF_PREFIX CCl4_COMPONENT_BASE
#define D_RF_CH3CCl3            D_RF_PREFIX CH3CCl3_COMPONENT_BASE
#define D_RF_HCF22              D_RF_PREFIX HCF22_COMPONENT_BASE
#define D_RF_HCF141b            D_RF_PREFIX HCF141b_COMPONENT_BASE
#define D_RF_HCF142b            D_RF_PREFIX HCF142b_COMPONENT_BASE
#define D_RF_halon1211          D_RF_PREFIX halon1211_COMPONENT_BASE
#define D_RF_halon1301          D_RF_PREFIX halon1301_COMPONENT_BASE
#define D_RF_halon2402          D_RF_PREFIX halon2402_COMPONENT_BASE
#define D_RF_CH3Cl              D_RF_PREFIX CH3Cl_COMPONENT_BASE
#define D_RF_CH3Br              D_RF_PREFIX CH3Br_COMPONENT_BASE
// halocarbon emissions
#define D_EMISSIONS_CF4                CF4_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_C2F6               C2F6_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_HFC23              HFC23_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_HFC32              HFC32_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_HFC4310            HFC4310_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_HFC125             HFC125_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_HFC134a            HFC134a_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_HFC143a            HFC143a_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_HFC227ea           HFC227ea_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_HFC245fa           HFC245fa_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_SF6                SF6_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_CFC11              CFC11_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_CFC12              CFC12_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_CFC113             CFC113_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_CFC114             CFC114_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_CFC115             CFC115_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_CCl4               CCl4_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_CH3CCl3            CH3CCl3_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_HCF22              HCF22_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_HCF141b            HCF141b_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_HCF142b            HCF142b_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_halon1211          halon1211_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_halon1301          halon1301_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_halon2402          halon2402_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_CH3Cl              CH3Cl_COMPONENT_BASE EMISSIONS_EXTENSION
#define D_EMISSIONS_CH3Br              CH3Br_COMPONENT_BASE EMISSIONS_EXTENSION

#define D_PREINDUSTRIAL_HC      "H0"
#define D_HC_CONCENTRATION      "hc_concentration"
#define D_HC_EMISSION           "hc_emission"
#define D_HC_TAU                "tau"
#define D_HC_RHO                "rho"
#define D_HC_MOLARMASS          "molarMass"

// methane component
#define D_ATMOSPHERIC_CH4       "CH4"
#define D_PREINDUSTRIAL_CH4     "M0"
#define D_RF_CH4                "FCH4"
#define D_EMISSIONS_CH4         "CH4_emissions"
#define D_NATURAL_CH4           "CH4N"
#define D_CONVERSION_CH4        "UC_CH4"
#define D_LIFETIME_SOIL         "Tsoil"
#define D_LIFETIME_STRAT        "Tstrat"

// n2o component
#define D_ATMOSPHERIC_N2O       "N2O"
#define D_PREINDUSTRIAL_N2O     "N0"
#define D_EMISSIONS_N2O         "N2O_emissions"
#define D_NAT_EMISSIONS_N2O     "N2ON_emissions"
#define D_CONVERSION_N2O        "UC_N2O"
#define D_INITIAL_LIFETIME_N2O   "TN2O0"
#define D_LIFETIME_N2O          "TAU_N2O"

// oh component
#define D_LIFETIME_OH           "TAU_OH"
#define D_INITIAL_LIFETIME_OH   "TOH0"
#define D_COEFFICENT_NOX        "CNOX"
#define D_COEFFICENT_CH4        "CCH4"
#define D_COEFFICENT_NMVOC      "CNMVOC"
#define D_COEFFICENT_CO         "CCO"

//o3 component
#define D_PREINDUSTRIAL_O3	      "PO3"
#define D_ATMOSPHERIC_O3	      "O3"
#define D_ATMOSPHERIC_EM_CH4      "CH4"
#define D_EMISSIONS_NOX           "NOX_emissions"
#define D_EMISSIONS_CO            "CO_emissions"
#define D_EMISSIONS_NMVOC         "NMVOC_emissions"
//Do not redefine Ma though it is used for o3 component
//#define D_EMISSIONS_O3 "O3_concentration"

// oc component
#define D_EMISSIONS_OC          "OC_emissions"

// ocean component
#define D_OCEAN_CFLUX           "atm_ocean_flux"
#define D_OCEAN_C               "ocean_c"
#define D_TT                    "tt"
#define D_TU                    "tu"
#define D_TWI                   "twi"
#define D_TID                   "tid"
#define D_CIRC_TOPT             "circ_Topt"
#define D_CIRC_T50_HIGH         "circ_T50_high"
#define D_CIRC_T50_LOW          "circ_T50_low"
#define D_HL_DO                 "HL_DO_Cflux"
#define D_PH_HL                 "pH_HL"
#define D_PH_LL                 "pH_LL"
#define D_ATM_OCEAN_FLUX_HL     "atm_ocean_flux_HL"
#define D_ATM_OCEAN_FLUX_LL     "atm_ocean_flux_LL"
#define D_PCO2_HL               "PCO2_HL"
#define D_PCO2_LL               "PCO2_LL"
#define D_DIC_HL                "DIC_HL"
#define D_DIC_LL                "DIC_LL"
#define D_P0_LL                 "p0_LL"
#define D_CARBON_HL             "carbon_HL"
#define D_CARBON_LL             "carbon_LL"
#define D_CARBON_IO             "carbon_IO"
#define D_CARBON_DO             "carbon_DO"
#define D_OMEGACA_HL            "OmegaCa_HL"
#define D_OMEGACA_LL            "OmegaCa_LL"
#define D_OMEGAAR_HL            "OmegaAr_HL"
#define D_OMEGAAR_LL            "OmegaAr_LL"
#define D_TEMP_HL               "Temp_HL"
#define D_TEMP_LL               "Temp_LL"
#define D_SPINUP_CHEM           "spinup_chem"

//#define D_SPECIFIC_HEAT			"cp"

#define D_CO3_LL				"CO3_LL"
#define D_CO3_HL				"CO3_HL"
#define D_ATM_OCEAN_CONSTRAIN   "atm_ocean_constrain"
#define D_TIMESTEPS             "ocean_timesteps"
#define D_REVELLE_HL            "Revelle_HL"
#define D_REVELLE_LL            "Revelle_LL"

// SimpleNbox component
#define D_LAND_CFLUX            "atm_land_flux"
#define D_ATMOSPHERIC_CO2       "Ca"
#define D_PREINDUSTRIAL_CO2     "C0"
#define D_ATMOSPHERIC_C         "atmos_c"
#define D_ATMOSPHERIC_C_RESIDUAL "atmos_c_residual"
#define D_EARTHC                "earth_c"
#define D_VEGC                  "veg_c"
#define D_DETRITUSC             "detritus_c"
#define D_SOILC                 "soil_c"
#define D_FFI_EMISSIONS         "ffi_emissions"
#define D_LUC_EMISSIONS         "luc_emissions"
#define D_CO2FERT               "co2fert"
#define D_TEMPFERTD              "detritus_tempfert"
#define D_TEMPFERTS              "soil_tempfert"
#define D_Q10_RH                "q10_rh"
#define D_NPP                   "npp"
#define D_RH                    "rh"
#define D_F_NPPV                "f_nppv"
#define D_F_NPPD                "f_nppd"
#define D_F_LITTERD             "f_litterd"
#define D_F_LUCV                "f_lucv"
#define D_F_LUCD                "f_lucd"
#define D_NPP_FLUX0             "npp_flux0"
#define D_CA_CONSTRAIN          "Ca_constrain"
#define D_BETA                  "beta"
//#define D_SIGMA                 "sigma"
#define D_WARMINGFACTOR         "warmingfactor"

// slr component
#define D_SL_RC                 "sl_rc"
#define D_SLR                   "slr"
#define D_SL_RC_NO_ICE          "sl_rc_no_ice"
#define D_SLR_NO_ICE            "slr_no_ice"

// so2 component
#define D_NATURAL_SO2       "SN"
#define D_2000_SO2     "S0"
#define D_EMISSIONS_SO2         "SO2_emissions"
#define D_VOLCANIC_SO2          "SV"

// temperature component
#define D_ECS                   "S"
#define D_GLOBAL_TEMP           "Tgav"
#define D_GLOBAL_TEMPEQ         "Tgaveq"
#define D_TGAV_CONSTRAIN        "tgav_constrain"
#define D_SO2D_B                "so2d_b"
#define D_SO2I_B                "so2i_b"
#define D_OC_B                  "oc_b"
#define D_BC_B                  "bc_b"

// temperature component
#define D_DIFFUSIVITY           "diff"
#define D_AERO_SCALE            "alpha"
#define D_FLUX_MIXED            "flux_mixed"
#define D_FLUX_INTERIOR         "flux_interior"
#define D_HEAT_FLUX             "heatflux"

// ------------------- messages being passed between components -------------------------

#define M_GETDATA               "getData"
#define M_SETDATA               "setData"

#define M_DUMP_TO_DEEP_OCEAN    "deepOceanCarbonDump"


#endif
