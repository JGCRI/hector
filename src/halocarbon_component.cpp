/* Hector -- A Simple Climate Model
   Copyright (C) 2022  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  halocarbon_component.cpp
 *  hector
 *
 *  Created by Pralit Patel on 1/20/11.
 *
 * References
 * Hartin 2015: Hartin, C. A., Patel, P., Schwarber, A., Link, R. P., and
 * Bond-Lamberty, B. P.: A simple object-oriented and open-source model for
 * scientific and policy analyses of the global climate system – Hector v1.0,
 * Geosci. Model Dev., 8, 939–955, https://doi.org/10.5194/gmd-8-939-2015, 2015.
 * IPCC AR6: Need to add the citation!
 */

#include <math.h>

#include "avisitor.hpp"
#include "core.hpp"
#include "h_util.hpp"
#include "halocarbon_component.hpp"

namespace Hector {

using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 */
HalocarbonComponent::HalocarbonComponent(std::string g) : tau(-1) {
  myGasName = g;
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
HalocarbonComponent::~HalocarbonComponent() {}

//------------------------------------------------------------------------------
// documentation is inherited
string HalocarbonComponent::getComponentName() const {
  return myGasName + HALOCARBON_EXTENSION;
}

//------------------------------------------------------------------------------
// documentation is inherited
void HalocarbonComponent::init(Core *coreptr) {
  logger.open(getComponentName(), false,
              coreptr->getGlobalLogger().getEchoToFile(),
              coreptr->getGlobalLogger().getMinLogLevel());
  //    concentration.name = myGasName;
  core = coreptr;

  emissions.allowInterp(true);
  emissions.name = myGasName;
  molarMass = 0.0;
  H0.set(0.0, U_PPTV); //! Default is no preindustrial, but user can override

  // Register the data we can provide
  core->registerCapability(D_RF_PREFIX + myGasName,
                           getComponentName()); // can provide forcing data
  core->registerCapability(myGasName + CONCENTRATION_EXTENSION,
                           getComponentName()); // can provide concentrations
  core->registerCapability(
      myGasName + CONC_CONSTRAINT_EXTENSION,
      getComponentName()); // can provide concentration constraints
  core->registerCapability(D_HCRHO_PREFIX + myGasName,
                           getComponentName()); // can provide rho
  core->registerCapability(D_HCDELTA_PREFIX + myGasName,
                           getComponentName()); // can provide rho

  // Register the inputs we can receive from outside
  core->registerInput(
      myGasName + CONC_CONSTRAINT_EXTENSION,
      getComponentName()); // inform core that we can accept concentration
                           // constraints for this gas
  core->registerInput(myGasName + EMISSIONS_EXTENSION,
                      getComponentName()); // inform core that we can accept
                                           // emissions for this gas
  core->registerInput(myGasName + CONC_CONSTRAINT_EXTENSION,
                      getComponentName()); // inform the core we can accept
                                           // provide concentration constraints
  core->registerInput(
      D_HCRHO_PREFIX + myGasName,
      getComponentName()); // nform the core we can accept rho for each HC.
  core->registerInput(
      D_HCDELTA_PREFIX + myGasName,
      getComponentName()); // inform the core we can accept delta
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval HalocarbonComponent::sendMessage(const std::string &message,
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
void HalocarbonComponent::setData(const string &varName,
                                  const message_data &data) {
  H_LOG(logger, Logger::DEBUG) << "Setting " << varName << "[" << data.date
                               << "]=" << data.value_str << std::endl;

  try {
    const string emiss_var_name = myGasName + EMISSIONS_EXTENSION;
    const string conc_var_name = myGasName + CONC_CONSTRAINT_EXTENSION;

    if (varName == D_HC_TAU) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      tau = data.getUnitval(U_UNDEFINED);
    } else if (varName == D_HCRHO_PREFIX + myGasName) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      rho = data.getUnitval(U_W_M2_PPTV);
    } else if (varName == D_HCDELTA_PREFIX + myGasName) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      delta = data.getUnitval(U_UNITLESS);
    } else if (varName == D_HC_MOLARMASS) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      molarMass = data.getUnitval(U_UNDEFINED);
    } else if (varName == emiss_var_name) {
      H_ASSERT(data.date != Core::undefinedIndex(), "date required");
      emissions.set(data.date, data.getUnitval(U_GG));
    } else if (varName == conc_var_name) {
      H_ASSERT(data.date != Core::undefinedIndex(), "date required");
      Ha_constrain.set(data.date, data.getUnitval(U_PPTV));
    } else if (varName == D_PREINDUSTRIAL_HC) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      H0 = data.getUnitval(U_PPTV);
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
void HalocarbonComponent::prepareToRun() {
  H_LOG(logger, Logger::DEBUG) << "prepareToRun " << std::endl;
  oldDate = core->getStartDate();

  H_ASSERT(tau != -1 && tau != 0, "tau has bad value");
  H_ASSERT(rho.units() != U_UNDEFINED, "rho has undefined units");
  H_ASSERT(molarMass > 0, "molarMass must be >0");
  H_ASSERT(
      delta >= -1 && delta <= 1,
      "bad delta value"); // delta is a paramter that must be between -1 and 1

  Ha_ts.set(oldDate, H0);

  //! \remark concentration values will not be allowed to interpolate beyond
  //! years already read in
  //    concentration.allowPartialInterp( true );
}

//------------------------------------------------------------------------------
// documentation is inherited
void HalocarbonComponent::run(const double runToDate) {
  H_ASSERT(!core->inSpinup() && runToDate - oldDate == 1,
           "timestep must equal 1");
#define AtmosphereDryAirConstant 1.8

  unitval Ha(Ha_ts.get(oldDate));

  // If emissions-forced, calculate concentration from emissions and lifespan.
  if (Ha_constrain.size() && Ha_constrain.exists(runToDate)) {
    // Concentration-forced. Just grab the current value from the time series.
    Ha = Ha_constrain.get(runToDate);
  } else {
    const double timestep = 1.0;
    const double alpha = 1 / tau;

    // Compute the delta atmospheric concentration from current emissions
    double emissMol = emissions.get(runToDate).value(U_GG) / molarMass *
                      timestep; // this is in U_GMOL
    unitval concDeltaEmiss;
    concDeltaEmiss.set(emissMol / (0.1 * AtmosphereDryAirConstant), U_PPTV);

    // Update the atmospheric concentration, accounting for this delta and
    // exponential decay
    double expfac = exp(-alpha);
    Ha = Ha * expfac + concDeltaEmiss * tau * (1.0 - expfac);
  }

  H_LOG(logger, Logger::DEBUG)
      << "date: " << runToDate << " concentration: " << Ha << endl;
  Ha_ts.set(runToDate, Ha);

  // Calculate radiative forcing
  double adjusted_rf;
  double rf_unadjusted;
  unitval rf;

  // First calculate the stratospheric-temperature adjusted radiative
  // efficiencies using parameter values from IPCC AR6 & Equation 16 from Hartin
  // 2015.
  rf_unadjusted = rho.value(U_W_M2_PPTV) * Ha.value(U_PPTV);
  // Now calculate the effective radiative forcing value by adjusting the
  // radiative forcing by the tropospheric adjustments (the delta parameter).
  adjusted_rf = rf_unadjusted + delta.value(U_UNITLESS) * rf_unadjusted;
  rf.set(adjusted_rf, U_W_M2);
  hc_forcing.set(runToDate, rf);

  // Update time counter.
  oldDate = runToDate;
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval HalocarbonComponent::getData(const std::string &varName,
                                     const double date) {

  unitval returnval;
  double getdate =
      date; // will be used for any variable where a date is allowed.
  if (getdate == Core::undefinedIndex()) {
    // If no date specified, return the last computed date
    getdate = oldDate;
  }

  if (varName == D_RF_PREFIX + myGasName) {
    returnval = hc_forcing.get(getdate);
  } else if (varName == D_PREINDUSTRIAL_HC) {
    // use date as input, not getdate, b/c there should be no date specified.
    H_ASSERT(date == Core::undefinedIndex(),
             "Date not allowed for preindustrial hc");
    returnval = H0;
  } else if (varName == D_HCRHO_PREFIX + myGasName) {
    // use date as input, not getdate, b/c there should be no date specified.
    H_ASSERT(date == Core::undefinedIndex(), "Date not allowed for rho");
    returnval = rho;
  } else if (varName == D_HCDELTA_PREFIX + myGasName) {
    // use date as input, not getdate, b/c there should be no date specified.
    H_ASSERT(date == Core::undefinedIndex(), "Date not allowed for delta");
    returnval = delta;
  } else if (varName == myGasName + CONCENTRATION_EXTENSION) {
    H_ASSERT(date != Core::undefinedIndex(),
             "Date required for halocarbon concentration");
    returnval = Ha_ts.get(getdate);
  } else if (varName == myGasName + EMISSIONS_EXTENSION) {
    if (emissions.exists(getdate))
      returnval = emissions.get(getdate);
    else
      returnval.set(0.0, U_GG);
  } else if (varName == D_HC_CONCENTRATION) {
    returnval = Ha_ts.get(getdate);
  } else if (varName == myGasName + CONC_CONSTRAINT_EXTENSION) {
    H_ASSERT(date != Core::undefinedIndex(),
             "Date required for halocarbon constraint");
    if (Ha_constrain.exists(getdate)) {
      returnval = Ha_constrain.get(getdate);
    } else {
      H_LOG(logger, Logger::DEBUG)
          << "No CH4 constraint for requested date " << date
          << ". Returning missing value." << std::endl;
      returnval.set(MISSING_FLOAT, U_PPTV);
    }
  } else {
    H_THROW("Caller is requesting unknown variable: " + varName);
  }

  return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void HalocarbonComponent::reset(double time) {
  // reset time counter and truncate outputs
  oldDate = time;
  hc_forcing.truncate(time);
  Ha_ts.truncate(time);
  H_LOG(logger, Logger::NOTICE)
      << getComponentName() << " reset to time= " << time << "\n";
}

//------------------------------------------------------------------------------
// documentation is inherited
void HalocarbonComponent::shutDown() {
  H_LOG(logger, Logger::DEBUG) << "goodbye " << getComponentName() << std::endl;
  logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void HalocarbonComponent::accept(AVisitor *visitor) { visitor->visit(this); }

} // namespace Hector
