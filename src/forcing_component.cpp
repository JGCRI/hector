/* Hector -- A Simple Climate Model
   Copyright (C) 2022  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  forcing_component.cpp
 *  hector
 *
 *  Created by Ben on 02 March 2011.
 *
 */

/* References:

 Meinshausen et al. (2011): Meinshausen, M., Raper, S. C. B., and Wigley, T. M.
 L.: Emulating coupled atmosphere-ocean and carbon cycle models with a simpler
 model, MAGICC6 – Part 1: Model description and calibration, Atmos. Chem. Phys.,
 11, 1417–1456, https://doi.org/10.5194/acp-11-1417-2011, 2011.

 */

// some boost headers generate warnings under clang; not our problem, ignore
// 2023 and Boost 1.81.0_1: array.hpp still generates one warning
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <boost/array.hpp>
#pragma clang diagnostic pop
#include <math.h>

#include "avisitor.hpp"
#include "forcing_component.hpp"

namespace Hector {

/* These next two arrays and the map that connects them are a
 * workaround for the problems created by storing the halocarbon
 * forcings in the halocarbon components.  Because the halocarbon
 * components don't know about the base year adjustments, they can't
 * provide the forcings relative to the base year, which is what
 * outside callers will generally want.  Internally, however, we still
 * need to be able to get the raw forcings from the halocarbon
 * components, so we can't just change everything to point at the
 * forcing component (which would return the base year adjusted value).
 *
 * The solution we adopted was to create a second set of capabilities
 * to refer to the adjusted values, and we let the forcing component
 * intercept those.  However, the forcing values themselves are stored
 * under the names used for the unadjusted values, so we have to have
 * a name translation table so that we can find the data that the
 * message is asking for.  In the end, the whole process winds up
 * being a little ugly, but it gets the job done.
 */

const char *ForcingComponent::adjusted_halo_forcings[N_HALO_FORCINGS] = {
    D_RFADJ_CF4,      D_RFADJ_C2F6,      D_RFADJ_HFC23,     D_RFADJ_HFC32,
    D_RFADJ_HFC4310,  D_RFADJ_HFC125,    D_RFADJ_HFC134a,   D_RFADJ_HFC143a,
    D_RFADJ_HFC227ea, D_RFADJ_HFC245fa,  D_RFADJ_SF6,       D_RFADJ_CFC11,
    D_RFADJ_CFC12,    D_RFADJ_CFC113,    D_RFADJ_CFC114,    D_RFADJ_CFC115,
    D_RFADJ_CCl4,     D_RFADJ_CH3CCl3,   D_RFADJ_HCFC22,    D_RFADJ_HCFC141b,
    D_RFADJ_HCFC142b, D_RFADJ_halon1211, D_RFADJ_halon1301, D_RFADJ_halon2402,
    D_RFADJ_CH3Cl,    D_RFADJ_CH3Br};

const char *ForcingComponent::halo_forcing_names[N_HALO_FORCINGS] = {
    D_RF_CF4,      D_RF_C2F6,      D_RF_HFC23,     D_RF_HFC32,
    D_RF_HFC4310,  D_RF_HFC125,    D_RF_HFC134a,   D_RF_HFC143a,
    D_RF_HFC227ea, D_RF_HFC245fa,  D_RF_SF6,       D_RF_CFC11,
    D_RF_CFC12,    D_RF_CFC113,    D_RF_CFC114,    D_RF_CFC115,
    D_RF_CCl4,     D_RF_CH3CCl3,   D_RF_HCFC22,    D_RF_HCFC141b,
    D_RF_HCFC142b, D_RF_halon1211, D_RF_halon1301, D_RF_halon2402,
    D_RF_CH3Cl,    D_RF_CH3Br};

std::map<std::string, std::string> ForcingComponent::forcing_name_map;

using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 */
ForcingComponent::ForcingComponent() {
  Fmisc_ts.allowInterp(true);
  Fmisc_ts.name = D_RF_MISC;
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
ForcingComponent::~ForcingComponent() {}

//------------------------------------------------------------------------------
// documentation is inherited
string ForcingComponent::getComponentName() const {
  const string name = FORCING_COMPONENT_NAME;
  return name;
}

//------------------------------------------------------------------------------
// documentation is inherited
void ForcingComponent::init(Core *coreptr) {

  logger.open(getComponentName(), false,
              coreptr->getGlobalLogger().getEchoToFile(),
              coreptr->getGlobalLogger().getMinLogLevel());
  H_LOG(logger, Logger::DEBUG) << "hello " << getComponentName() << std::endl;

  core = coreptr;

  baseyear = 0.0;
  currentYear = 0.0;

  Ftot_constrain.allowInterp(true);
  Ftot_constrain.name = D_RF_TOTAL;

  // Register the data we can provide
  core->registerCapability(D_RF_TOTAL, getComponentName());
  core->registerCapability(D_RF_BASEYEAR, getComponentName());
  core->registerCapability(D_RF_CO2, getComponentName());
  core->registerCapability(D_RF_CH4, getComponentName());
  core->registerCapability(D_RF_N2O, getComponentName());
  core->registerCapability(D_RF_H2O_STRAT, getComponentName());
  core->registerCapability(D_RF_O3_TROP, getComponentName());
  core->registerCapability(D_RF_BC, getComponentName());
  core->registerCapability(D_RF_OC, getComponentName());
  core->registerCapability(D_RF_NH3, getComponentName());
  core->registerCapability(D_RF_VOL, getComponentName());
  core->registerCapability(D_DELTA_CH4, getComponentName());
  core->registerCapability(D_DELTA_N2O, getComponentName());
  core->registerCapability(D_DELTA_CO2, getComponentName());
  core->registerCapability(D_RHO_BC, getComponentName());
  core->registerCapability(D_RHO_OC, getComponentName());
  core->registerCapability(D_RHO_SO2, getComponentName());
  core->registerCapability(D_RF_SO2, getComponentName());
  core->registerCapability(D_RF_ACI, getComponentName());
  for (int i = 0; i < N_HALO_FORCINGS; ++i) {
    core->registerCapability(adjusted_halo_forcings[i], getComponentName());
    forcing_name_map[adjusted_halo_forcings[i]] = halo_forcing_names[i];
  }

  // Register our dependencies
  core->registerDependency(D_CH4_CONC, getComponentName());
  core->registerDependency(D_CO2_CONC, getComponentName());
  core->registerDependency(D_ATMOSPHERIC_O3, getComponentName());
  core->registerDependency(D_EMISSIONS_BC, getComponentName());
  core->registerDependency(D_EMISSIONS_OC, getComponentName());
  core->registerDependency(D_EMISSIONS_NH3, getComponentName());
  core->registerDependency(D_EMISSIONS_H2, getComponentName());
  core->registerDependency(D_N2O_CONC, getComponentName());
  core->registerDependency(D_RF_CF4, getComponentName());
  core->registerDependency(D_RF_C2F6, getComponentName());
  core->registerDependency(D_RF_HFC23, getComponentName());
  core->registerDependency(D_RF_HFC32, getComponentName());
  core->registerDependency(D_RF_HFC4310, getComponentName());
  core->registerDependency(D_RF_HFC125, getComponentName());
  core->registerDependency(D_RF_HFC134a, getComponentName());
  core->registerDependency(D_RF_HFC143a, getComponentName());
  core->registerDependency(D_RF_HFC227ea, getComponentName());
  core->registerDependency(D_RF_HFC245fa, getComponentName());
  core->registerDependency(D_RF_SF6, getComponentName());
  core->registerDependency(D_RF_CFC11, getComponentName());
  core->registerDependency(D_RF_CFC12, getComponentName());
  core->registerDependency(D_RF_CFC113, getComponentName());
  core->registerDependency(D_RF_CFC114, getComponentName());
  core->registerDependency(D_RF_CFC115, getComponentName());
  core->registerDependency(D_RF_CCl4, getComponentName());
  core->registerDependency(D_RF_CH3CCl3, getComponentName());
  core->registerDependency(D_RF_HCFC22, getComponentName());
  core->registerDependency(D_RF_HCFC141b, getComponentName());
  core->registerDependency(D_RF_HCFC142b, getComponentName());
  core->registerDependency(D_RF_halon1211, getComponentName());
  core->registerDependency(D_RF_halon1301, getComponentName());
  core->registerDependency(D_RF_halon2402, getComponentName());
  core->registerDependency(D_RF_CH3Br, getComponentName());
  core->registerDependency(D_RF_CH3Cl, getComponentName());
  core->registerDependency(D_RF_T_ALBEDO, getComponentName());

  // Register the inputs we can receive from outside
  core->registerInput(D_DELTA_CH4, getComponentName());
  core->registerInput(D_DELTA_N2O, getComponentName());
  core->registerInput(D_DELTA_CO2, getComponentName());
  core->registerInput(D_RHO_BC, getComponentName());
  core->registerInput(D_RHO_OC, getComponentName());
  core->registerInput(D_RHO_SO2, getComponentName());
  core->registerInput(D_RHO_NH3, getComponentName());
  core->registerInput(D_RF_MISC, getComponentName());
  core->registerInput(D_FTOT_CONSTRAIN, getComponentName());
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval ForcingComponent::sendMessage(const std::string &message,
                                      const std::string &datum,
                                      const message_data info) {
  unitval returnval;

  if (message == M_GETDATA) { //! Caller is requesting data
    return getData(datum, info.date);

  } else if (message == M_SETDATA) { //! Caller is requesting to set data
    setData(datum, info);

  } else { //! We don't handle any other messages
    H_THROW("Caller sent unknown message: " + message);
  }

  return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void ForcingComponent::setData(const string &varName,
                               const message_data &data) {
  H_LOG(logger, Logger::DEBUG) << "Setting " << varName << "[" << data.date
                               << "]=" << data.value_str << std::endl;

  try {
    if (varName == D_RF_BASEYEAR) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      baseyear = data.getUnitval(U_UNDEFINED);
    } else if (varName == D_DELTA_CH4) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      delta_ch4 = data.getUnitval(U_UNITLESS);
    } else if (varName == D_DELTA_N2O) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      delta_n2o = data.getUnitval(U_UNITLESS);
    } else if (varName == D_DELTA_CO2) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      delta_co2 = data.getUnitval(U_UNITLESS);
    } else if (varName == D_RHO_BC) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      rho_bc = data.getUnitval(U_W_M2_TG);
    } else if (varName == D_RHO_OC) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      rho_oc = data.getUnitval(U_W_M2_TG);
    } else if (varName == D_RHO_NH3) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      rho_nh3 = data.getUnitval(U_W_M2_TG);
    } else if (varName == D_RHO_SO2) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      rho_so2 = data.getUnitval(U_W_M2_GG);
    } else if (varName == D_FTOT_CONSTRAIN) {
      H_ASSERT(data.date != Core::undefinedIndex(), "date required");
      Ftot_constrain.set(data.date, data.getUnitval(U_W_M2));
    } else if (varName == D_RF_MISC) {
      H_ASSERT(data.date != Core::undefinedIndex(), "date required");
      Fmisc_ts.set(data.date, data.getUnitval(U_W_M2));
    } else {
      H_LOG(logger, Logger::DEBUG)
          << "Unknown variable " << varName << std::endl;
      H_THROW("Unknown variable name while parsing " + getComponentName() +
              ": " + varName);
    }
  } catch (h_exception &parseException) {
    H_RETHROW(parseException, "Could not parse var: " + varName);
  }
}

//------------------------------------------------------------------------------
// documentation is inherited
void ForcingComponent::prepareToRun() {

  H_LOG(logger, Logger::DEBUG) << "prepareToRun " << std::endl;

  if (baseyear == 0.0)
    baseyear = core->getStartDate() + 1; // default, if not supplied by user
  H_LOG(logger, Logger::DEBUG)
      << "Base year for reporting is " << baseyear << std::endl;

  H_ASSERT(baseyear > core->getStartDate(),
           "Base year must be >= model start date");

  if (Ftot_constrain.size()) {
    Logger &glog = core->getGlobalLogger();
    H_LOG(glog, Logger::WARNING)
        << "Total forcing will be overwritten by user-supplied values!"
        << std::endl;
  }

  // delta parameters must be between -1 and 1
  H_ASSERT(delta_ch4 >= -1 && delta_ch4 <= 1, "bad delta CH4 value");
  H_ASSERT(delta_n2o >= -1 && delta_n2o <= 1, "bad delta N2O value");
  H_ASSERT(delta_co2 >= -1 && delta_co2 <= 1, "bad delta CO2 value");

  baseyear_forcings.clear();
}

//------------------------------------------------------------------------------
// documentation is inherited
void ForcingComponent::run(const double runToDate) {

  // Calculate instantaneous radiative forcing for any & all agents
  // As each is computed, push it into 'forcings' map for Ftot calculation.
  // Note that forcings have to be mutually exclusive, there are no subtotals
  // for different species.
  H_LOG(logger, Logger::DEBUG) << "-----------------------------" << std::endl;
  currentYear = runToDate;

  if (runToDate < baseyear) {
    H_LOG(logger, Logger::DEBUG) << "not yet at baseyear" << std::endl;
  } else {
    forcings_t forcings;

    //  ---------- Major GHGs ----------
    if (core->checkCapability(D_CH4_CONC) &&
        core->checkCapability(D_N2O_CONC) &&
        core->checkCapability(D_CO2_CONC)) {

      // Parse our the pre industrial and concentrations to use in RF
      // calculations
      double C0 =
          core->sendMessage(M_GETDATA, D_PREINDUSTRIAL_CO2).value(U_PPMV_CO2);
      double M0 =
          core->sendMessage(M_GETDATA, D_PREINDUSTRIAL_CH4).value(U_PPBV_CH4);
      double N0 =
          core->sendMessage(M_GETDATA, D_PREINDUSTRIAL_N2O).value(U_PPBV_N2O);
      double CO2_conc =
          core->sendMessage(M_GETDATA, D_CO2_CONC, message_data(runToDate))
              .value(U_PPMV_CO2);
      double Ma =
          core->sendMessage(M_GETDATA, D_CH4_CONC, message_data(runToDate))
              .value(U_PPBV_CH4);
      double Na =
          core->sendMessage(M_GETDATA, D_N2O_CONC, message_data(runToDate))
              .value(U_PPBV_N2O);

      // ---------- CO2 ----------
      // CO2 SARF is calculated using simplified expressions from IPCC
      // AR6 listed in Table 7.SM.1. Then the SARF is adjusted by a scalar
      // value to account for tropospheric interactions see
      // Note that this simplified expression for radiative forcing was
      // calibrated with a preindustrial  N20 value of 277.15 ppm.
      double C_alpha_max = C0 - (b1 / (2 * a1));
      double n2o_alpha = c1 * sqrt(Na);
      double alpha_prime;
      if (CO2_conc > C_alpha_max) {
        alpha_prime = d1 - (pow(b1, 2) / (4 * a1));
      } else if (C0 < CO2_conc && CO2_conc < C_alpha_max) {
        alpha_prime = d1 + a1 * pow((CO2_conc - C0), 2) + b1 * (CO2_conc - C0);
      } else if (CO2_conc <= C0) {
        alpha_prime = d1;
      } else {
        H_THROW("Caller is requesting unknown condition for CO2 SARF ");
      }
      double sarf_co2 = (alpha_prime + n2o_alpha) * log(CO2_conc / C0);
      double fco2 = (sarf_co2 * delta_co2) + sarf_co2;
      forcings[D_RF_CO2].set(fco2, U_W_M2);

      // ---------- N2O ----------
      // N2O SARF is calculated using simplified expressions from IPCC
      // AR6 listed in Table 7.SM.1. Then the SARF is adjusted by a scalar
      // value to account for tropospheric interactions see 7.3.2.3.
      // Note that this simplified expression for radiative forcing was
      // calibrated with a preindustrial N20 value of 273.87 ppb.
      double sarf_n2o =
          (a2 * sqrt(CO2_conc) + b2 * sqrt(Na) + c2 * sqrt(Ma) + d2) *
          (sqrt(Na) - sqrt(N0));
      double fn2o = (delta_n2o * sarf_n2o) + sarf_n2o;
      forcings[D_RF_N2O].set(fn2o, U_W_M2);

      // ---------- CH4 ----------
      // CH4 SARF is calculated using simplified expressions from IPCC
      // AR6 listed in Table 7.SM.1. Then the SARF is adjusted by a scalar
      // value to account for tropospheric interactions.
      double sarf_ch4 =
          (a3 * sqrt(Ma) + b3 * sqrt(Na) + d3) * (sqrt(Ma) - sqrt(M0));
      double fch4 = (delta_ch4 * sarf_ch4) + sarf_ch4;
      forcings[D_RF_CH4].set(fch4, U_W_M2);

      // ---------- Stratospheric H2O based on CH4 oxidation ----------
      // The stratospheric water vapour RF based on changes in CH4
      // concentrations.
      const double Ma_base =
          1831; // 2014 CH4 concentration ppb from the cmip6 historical scenario
      const double stratH2O_base =
          0.0485; // W m-2 Strat H2O RF (1850 to 2014) from 7.3.2.6 IPCC AR6
     // unitval current_h2 = H2_emissions.get(runToDate);

        //H2_emissions.get(H2_emissions.firstdate()).value(U_TG_H2);


        double current_h2 = core->sendMessage(M_GETDATA, D_EMISSIONS_H2,
                                               message_data(runToDate))
                                 .value(U_TG_H2);
        // TODO this needs to actually be the first date should not be hard codded in
        double inital_h2 = core->sendMessage(M_GETDATA, D_EMISSIONS_H2,
                                             message_data(1750))
                                 .value(U_TG_H2);
        // TODO this is shoudl not be hard coded in ither!
      const double fh2o_strat = stratH2O_base * ((Ma - M0) / (Ma_base - M0)) + 0.001 * 0.19 * (current_h2 - inital_h2); //





      forcings[D_RF_H2O_STRAT].set(fh2o_strat, U_W_M2);
    }

    // ---------- Troposheric Ozone ----------
    if (core->checkCapability(D_ATMOSPHERIC_O3)) {
      // from Tanaka et al, 2007
      const double ozone = core->sendMessage(M_GETDATA, D_ATMOSPHERIC_O3,
                                             message_data(runToDate))
                               .value(U_DU_O3);
      const double fo3_trop = 0.042 * ozone;
      forcings[D_RF_O3_TROP].set(fo3_trop, U_W_M2);
    }

    // ---------- Halocarbons ----------
    boost::array<string, 26> halos = {
        {D_RF_CF4,      D_RF_C2F6,      D_RF_HFC23,     D_RF_HFC32,
         D_RF_HFC4310,  D_RF_HFC125,    D_RF_HFC134a,   D_RF_HFC143a,
         D_RF_HFC227ea, D_RF_HFC245fa,  D_RF_SF6,       D_RF_CFC11,
         D_RF_CFC12,    D_RF_CFC113,    D_RF_CFC114,    D_RF_CFC115,
         D_RF_CCl4,     D_RF_CH3CCl3,   D_RF_HCFC22,    D_RF_HCFC141b,
         D_RF_HCFC142b, D_RF_halon1211, D_RF_halon1301, D_RF_halon2402,
         D_RF_CH3Cl,    D_RF_CH3Br}};

    // Halocarbons can be disabled individually via the input file, so we run
    // through all possible ones
    for (auto hc : halos) {
      if (core->checkCapability(hc)) {
        // Forcing values are actually computed by the halocarbon itself
        forcings[hc] =
            core->sendMessage(M_GETDATA, hc, message_data(runToDate));
      }
    }

    // Aerosols
    if (core->checkCapability(D_EMISSIONS_BC) &&
        core->checkCapability(D_EMISSIONS_OC) &&
        core->checkCapability(D_EMISSIONS_SO2) &&
        core->checkCapability(D_EMISSIONS_NH3)) {

      // Aerosol-Radiation Interactions (RFari)
      // RFari was calculated using a simple linear relationship to emissions of
      // BC, OC, SO2, and NH3.
      // The rho parameters correspond to the radiative efficiencies reported in
      // the text of 7.SM.1.3.1 IPCC AR6, see there for more details.

      // ---------- Black carbon ----------
      double E_BC =
          core->sendMessage(M_GETDATA, D_EMISSIONS_BC, message_data(runToDate))
              .value(U_TG);
      double fbc = rho_bc * E_BC;
      forcings[D_RF_BC].set(fbc, U_W_M2);

      // ---------- Organic carbon ----------
      double E_OC =
          core->sendMessage(M_GETDATA, D_EMISSIONS_OC, message_data(runToDate))
              .value(U_TG);
      double foc = rho_oc * E_OC;
      forcings[D_RF_OC].set(foc, U_W_M2);

      // ---------- Sulphate Aerosols ----------
      unitval SO2_emission = core->sendMessage(M_GETDATA, D_EMISSIONS_SO2,
                                               message_data(runToDate));
      double fso2 = rho_so2 * SO2_emission.value(U_GG_S);
      forcings[D_RF_SO2].set(fso2, U_W_M2);

      // ---------- NH3 ----------
      double E_NH3 =
          core->sendMessage(M_GETDATA, D_EMISSIONS_NH3, message_data(runToDate))
              .value(U_TG);
      double fnh3 = rho_nh3 * E_NH3;
      forcings[D_RF_NH3].set(fnh3, U_W_M2);

      // ---------- RFaci ----------
      // ERF from aerosol-cloud interactions (RFaci)
      // Based on Equation 7.SM.1.2 from IPCC AR6 where
      double aci_rf =
          -1 * aci_beta *
          log(1 + (SO2_emission / s_SO2) + ((E_BC + E_OC) / s_BCOC));
      forcings[D_RF_ACI].set(aci_rf, U_W_M2);
    }

    // ---------- Terrestrial albedo ----------
    if (core->checkCapability(D_RF_T_ALBEDO)) {
      forcings[D_RF_T_ALBEDO] =
          core->sendMessage(M_GETDATA, D_RF_T_ALBEDO, message_data(runToDate));
    }

    // ---------- Volcanic forcings ----------
    if (core->checkCapability(D_VOLCANIC_SO2)) {
      // The volcanic forcings are read in from an ini file.
      forcings[D_RF_VOL] =
          core->sendMessage(M_GETDATA, D_VOLCANIC_SO2, message_data(runToDate));
    }

    // ---------- Miscellaneous forcings ----------
    // Miscellaneous forcings read in from an ini file.
    forcings[D_RF_MISC] = Fmisc_ts.get(runToDate);

    // ---------- Total ----------
    // Calculate based as the sum of the different radiative forcings or as the
    // user supplied constraint.
    unitval Ftot(0.0, U_W_M2); // W/m2
    for (auto f : forcings) {
      Ftot = Ftot + f.second;
    }

    // Otherwise if the user has supplied total forcing data, use that instead.
    if (Ftot_constrain.size() && runToDate <= Ftot_constrain.lastdate()) {
      H_LOG(logger, Logger::WARNING)
          << "** Overwriting total forcing with user-supplied value"
          << std::endl;
      forcings[D_RF_TOTAL] = Ftot_constrain.get(runToDate);
    } else {
      forcings[D_RF_TOTAL] = Ftot;
    }

    //---------- Change to relative forcing ----------
    // Note that the code below assumes model is always consistently run from
    // base-year forward. Results will not be consistent if parameters are
    // changed but base-year is not re-run. At this point, we've computed all
    // absolute forcings. If base year, save those values
    if (runToDate == baseyear) {
      H_LOG(logger, Logger::DEBUG)
          << "** At base year! Storing current forcing values" << std::endl;
      baseyear_forcings = forcings;
    }

    // Subtract base year forcing values from forcings, i.e. make them relative
    // to base year
    for (auto f : forcings) {
      forcings[f.first] = f.second - baseyear_forcings[f.first];
      H_LOG(logger, Logger::DEBUG)
          << "forcing " << f.first << " in " << runToDate << " is " << f.second
          << std::endl;
    }
    H_LOG(logger, Logger::DEBUG)
        << "forcing total is " << forcings[D_RF_TOTAL] << std::endl;

    // Store the forcings that we have calculated
    forcings_ts.set(runToDate, forcings);
  }
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval ForcingComponent::getData(const std::string &varName,
                                  const double date) {

  unitval returnval;
  double getdate = date; // This is why I hate declaring PBV args as const!

  // If requesting data not associated with a date aka a parameter,
  // return the parameter value.
  if (varName == D_DELTA_CH4) {
    returnval = delta_ch4;
  } else if (varName == D_DELTA_N2O) {
    returnval = delta_n2o;
  } else if (varName == D_DELTA_CO2) {
    returnval = delta_co2;
  } else if (varName == D_RHO_BC) {
    returnval = rho_bc;
  } else if (varName == D_RHO_OC) {
    returnval = rho_oc;
  } else if (varName == D_RHO_SO2) {
    returnval = rho_so2;
  } else if (varName == D_RHO_NH3) {
    returnval = rho_nh3;
  } else if (varName == D_RF_BASEYEAR) {
    returnval.set(baseyear, U_UNITLESS);
  } else {
    // All other data require a date
    H_ASSERT(date != Core::undefinedIndex(), "Date required for " + varName);

    // If before RF base year, return zero
    if (getdate < baseyear) {
      returnval.set(0.0, U_W_M2);
      return returnval;
    }

    // Look up the forcing name and value
    forcings_t forcings(forcings_ts.get(getdate));

    std::string forcing_name;
    auto forcit = forcing_name_map.find(varName);
    if (forcit != forcing_name_map.end()) {
      forcing_name = forcing_name_map[varName];
    } else {
      forcing_name = varName;
    }
    std::map<std::string, unitval>::const_iterator forcing =
        forcings.find(forcing_name);
    if (forcing != forcings.end()) {
      returnval = forcing->second; // from the forcing map
    } else {
      H_THROW("Caller is requesting unknown variable: " + varName);
    }
  }

  return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void ForcingComponent::reset(double time) {
  // Set the current year to the reset year, and drop outputs after the reset
  // year.
  currentYear = time;
  forcings_ts.truncate(time);
  H_LOG(logger, Logger::NOTICE)
      << getComponentName() << " reset to time= " << time << "\n";
}

//------------------------------------------------------------------------------
// documentation is inherited
void ForcingComponent::shutDown() {
  H_LOG(logger, Logger::DEBUG) << "goodbye " << getComponentName() << std::endl;
  logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void ForcingComponent::accept(AVisitor *visitor) { visitor->visit(this); }

} // namespace Hector
