/* Hector -- A Simple Climate Model
 Copyright (C) 2022  Battelle Memorial Institute

 Please see the accompanying file LICENSE.md for additional licensing
 information.
 */
/*
 *  temperature_component.cpp
 *  hector
 *
 *  Created by Ben Vega-Westhoff on 11/1/16.
 *
 * DOECLIM is based on
 *  Kriegler, E. (2005) Imprecise probability analysis for Integrated Assessment
 * of climate change. Ph.D. dissertation. Potsdam Universität. 256 pp.
 * (http://opus.kobv.de/ubp/volltexte/2005/561/; DOECLIM introduced in Chapter 2
 * and Annexes A and B) Tanaka, K. & Kriegler, E. (2007) Aggregated carbon
 * cycle, atmospheric chemistry, and climate model (ACC2) – Description of the
 * forward and inverse modes – . Reports Earth Syst. Sci. 199. Garner, G., Reed,
 * P. & Keller, K. (2016) Climate risk management requires explicit
 * representation of societal trade-offs. Clim. Change 134, 713–723.
 *
 * Other References
 *  Meinshausen, M., Raper, S. C. B., and Wigley, T. M. L.: Emulating coupled
 * atmosphere-ocean and carbon cycle models with a simpler model, MAGICC6 – Part
 * 1: Model description and calibration, Atmos. Chem. Phys., 11, 1417–1456,
 * https://doi.org/10.5194/acp-11-1417-2011, 2011.
 *
 */

// some boost headers generate warnings under clang; not our problem, ignore
// 2023 and Boost 1.81.0_1: lexical_cast.hpp still generates many warnings
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <boost/lexical_cast.hpp>
#pragma clang diagnostic pop

#include <cmath>
#include <limits>

// The MinGW C++ compiler doesn't seem to pull in the cmath constants? (see
// #384) As a workaround, we define M_PI here if needed
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "avisitor.hpp"
#include "core.hpp"
#include "h_util.hpp"
#include "simpleNbox.hpp"
#include "temperature_component.hpp"

namespace Hector {

using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 */
TemperatureComponent::TemperatureComponent() {}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
TemperatureComponent::~TemperatureComponent() {}

//------------------------------------------------------------------------------
// documentation is inherited
string TemperatureComponent::getComponentName() const {
  const string name = TEMPERATURE_COMPONENT_NAME;

  return name;
}

//------------------------------------------------------------------------------
/*! \brief              Calculates inverse of x and stores in y
 *  \param[in] x        Assume x is setup like x = [a,b,c,d] -> x = |a, b|
 *                                                |c, d|
 *  \param[out] y        Inverted 1-d matrix
 */
void TemperatureComponent::invert_1d_2x2_matrix(double *x, double *y) {
  double temp_d = (x[0] * x[3] - x[1] * x[2]);

  if (temp_d == 0) {
    H_THROW("Temperature: Matrix inversion divide by zero.");
  }
  double temp = 1 / temp_d;
  y[0] = temp * x[3];
  y[1] = temp * -1 * x[1];
  y[2] = temp * -1 * x[2];
  y[3] = temp * x[0];

  return;
}

//------------------------------------------------------------------------------
// documentation is inherited
void TemperatureComponent::init(Core *coreptr) {
  logger.open(getComponentName(), false,
              coreptr->getGlobalLogger().getEchoToFile(),
              coreptr->getGlobalLogger().getMinLogLevel());
  H_LOG(logger, Logger::DEBUG) << "hello " << getComponentName() << std::endl;

  tas.set(0.0, U_DEGC, 0.0);
  flux_mixed.set(0.0, U_W_M2, 0.0);
  flux_interior.set(0.0, U_W_M2, 0.0);
  heatflux.set(0.0, U_W_M2, 0.0);
  lo_warming_ratio.set(0.0, U_UNITLESS, 0.0);

  core = coreptr;

  tas_constrain.allowInterp(true);
  tas_constrain.name = D_TAS_CONSTRAIN;

  // Register the data we can provide
  core->registerCapability(D_GLOBAL_TAS, getComponentName());
  core->registerCapability(D_LAND_TAS, getComponentName());
  core->registerCapability(D_OCEAN_TAS, getComponentName());
  core->registerCapability(D_SST, getComponentName());
  core->registerCapability(D_FLUX_MIXED, getComponentName());
  core->registerCapability(D_FLUX_INTERIOR, getComponentName());
  core->registerCapability(D_HEAT_FLUX, getComponentName());
  core->registerCapability(D_QCO2, getComponentName());
  core->registerCapability(D_LO_WARMING_RATIO, getComponentName());

  // Register our dependencies
  core->registerDependency(D_RF_TOTAL, getComponentName());
  core->registerDependency(D_RF_BC, getComponentName());
  core->registerDependency(D_RF_OC, getComponentName());
  core->registerDependency(D_RF_NH3, getComponentName());
  core->registerDependency(D_RF_SO2, getComponentName());
  core->registerDependency(D_RF_ACI, getComponentName());
  core->registerDependency(D_RF_VOL, getComponentName());

  // Register the inputs we can receive from outside
  core->registerInput(D_ECS, getComponentName());
  core->registerInput(D_QCO2, getComponentName());
  core->registerInput(D_DIFFUSIVITY, getComponentName());
  core->registerInput(D_AERO_SCALE, getComponentName());
  core->registerInput(D_VOLCANIC_SCALE, getComponentName());
  core->registerInput(D_LO_WARMING_RATIO, getComponentName());
  core->registerInput(D_TAS_CONSTRAIN, getComponentName());
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval TemperatureComponent::sendMessage(const std::string &message,
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
void TemperatureComponent::setData(const string &varName,
                                   const message_data &data) {
  using namespace boost;

  H_LOG(logger, Logger::DEBUG) << "Setting " << varName << "[" << data.date
                               << "]=" << data.value_str << std::endl;

  try {
    if (varName == D_ECS) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      S = data.getUnitval(U_DEGC);
    } else if (varName == D_DIFFUSIVITY) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      diff = data.getUnitval(U_CM2_S);
    } else if (varName == D_AERO_SCALE) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      alpha = data.getUnitval(U_UNITLESS);
    } else if (varName == D_VOLCANIC_SCALE) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      volscl = data.getUnitval(U_UNITLESS);
    } else if (varName == D_QCO2) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      qco2 = data.getUnitval(U_UNITLESS).value(U_UNITLESS);
    } else if (varName == D_TAS_CONSTRAIN) {
      H_ASSERT(data.date != Core::undefinedIndex(), "date required");
      tas_constrain.set(data.date, data.getUnitval(U_DEGC));
    } else if (varName == D_LO_WARMING_RATIO) {
      H_ASSERT(data.date == Core::undefinedIndex(), "date not allowed");
      lo_warming_ratio = data.getUnitval(U_UNITLESS);
    } else {
      H_THROW("Unknown variable name while parsing " + getComponentName() +
              ": " + varName);
    }
  } catch (bad_lexical_cast &castException) {
    H_THROW("Could not convert var: " + varName + ", value: " + data.value_str +
            ", exception: " + castException.what());
  } catch (h_exception &parseException) {
    H_RETHROW(parseException, "Could not parse var: " + varName);
  }
}

//------------------------------------------------------------------------------
// documentation is inherited
// TO DO: should we put these in the ini file instead?
void TemperatureComponent::prepareToRun() {

  H_LOG(logger, Logger::DEBUG) << "prepareToRun " << std::endl;

  if (tas_constrain.size()) {
    Logger &glog = core->getGlobalLogger();
    H_LOG(glog, Logger::WARNING)
        << "Temperature will be overwritten by user-supplied values!"
        << std::endl;
  }

  if (lo_warming_ratio != 0) {
    Logger &glog = core->getGlobalLogger();
    H_LOG(glog, Logger::WARNING)
        << "User supplied land-ocean warming ratio will be used to override "
           "air over land and air over ocean temperatures! User set land-ocean "
           "warming ratio: "
        << lo_warming_ratio << std::endl;
  }

  // Initializing all model components that depend on the number of timesteps
  // (ns)
  ns = core->getEndDate() - core->getStartDate() + 1;

  KT0 = std::vector<double>(ns, 0.0);
  KTA1 = std::vector<double>(ns, 0.0);
  KTB1 = std::vector<double>(ns, 0.0);
  KTA2 = std::vector<double>(ns, 0.0);
  KTB2 = std::vector<double>(ns, 0.0);
  KTA3 = std::vector<double>(ns, 0.0);
  KTB3 = std::vector<double>(ns, 0.0);

  Ker.resize(ns);

  temp.resize(ns);
  temp_landair.resize(ns);
  temp_sst.resize(ns);
  heatflux_mixed.resize(ns);
  heatflux_interior.resize(ns);
  heat_mixed.resize(ns);
  heat_interior.resize(ns);
  forcing.resize(ns);
  lo_temp_landair.resize(
      ns); //!< place to store land temp when lo is provided by users, deg C
  lo_temp_oceanair.resize(
      ns); //!< place to store land temp when lo is provided by users, deg C
  lo_sst.resize(
      ns); //!< place to store land temp when lo is provided by users, deg C

  for (int i = 0; i < 3; i++) {
    B[i] = 0.0;
    C[i] = 0.0;
  }

  // DOECLIM model parameters, based on constants set in the header
  //
  // Constants & conversion factors
  kcon = secs_per_Year / 10000; // conversion factor from cm2/s to m2/yr;
  ocean_area = (1.0 - flnd) * earth_area; // m2

  // Calculate climate feedback parameterisation
  cnum = rlam * flnd +
         bsi * (1.0 - flnd); // denominator used to calculate climate senstivity
                             // feedback parameters over land & sea
  cden = rlam * flnd -
         ak * (rlam - bsi); // another denominator use to calculate climate
                            // senstivity feedback parameters over land & sea
  cfl = flnd * cnum / cden * qco2 / S -
        bk * (rlam - bsi) / cden; // calculate the land climate feedback
                                  // parameter (W/(m2K)) eq A.19 Kriegler 2005
  cfs = (rlam * flnd - ak / (1.0 - flnd) * (rlam - bsi)) * cnum / cden * qco2 /
            S +
        rlam * flnd / (1.0 - flnd) * bk * (rlam - bsi) /
            cden; // calculate the sea climate feedback parameter (W/(m2K)) eq
                  // A.20 Kriegler 2005
  kls = bk * rlam * flnd / cden - ak * flnd * cnum / cden * qco2 /
                                      S; // land-sea heat exchange coefficient
                                         // (W/(m2K)) eq A.21 Kriegler 2005

  // Calculate ocean heat flux parameters & conversion factors
  keff = kcon * diff; // covert units of ocean heat diffusivity (m2/yr)
  powtoheat =
      ocean_area * secs_per_Year /
      pow(10.0,
          22); // conversion factor to convert total ocean heat flux to (m2*s)

  // Get the six different times scales used in the numerical aproximation of
  // the heat flux into the interior ocean See eq A.22 Kriegler 2005 for more
  // details.
  taubot = pow(zbot, 2) /
           keff; // number of years for the ocean to equilibrates, bottom water
                 // has warmed as much as the surface (yr)
  taucfs = cas / cfs; // sea climate feedback time scale (yr)
  taucfl = cal / cfl; // land climate feedback time scale (yr)
  taudif = pow(cas, 2) / pow(csw, 2) * M_PI /
           keff; // interior ocean heat uptake time scale (yr)
  tauksl = (1.0 - flnd) * cas / kls; // sea-land heat exchange time scale (yr)
  taukls = flnd * cal / kls;         // land-sea heat exchange time scale (yr)

  // Set up and solve the correction terms for the analytical solution for the
  // DOECLIM integrands.

  // Set Up
  // Components of the analytical solution to the integral found in the
  // temperature difference equation Third order bottom correction terms will be
  // "more than sufficient" for simulations out to 2500 (Equation A.25, EK05,
  // or 2.3.23, TK07)

  // First order
  KT0[ns - 1] = 4.0 - 2.0 * pow(2.0, 0.5);
  KTA1[ns - 1] =
      -8.0 * exp(-taubot / dt) + 4.0 * pow(2.0, 0.5) * exp(-0.5 * taubot / dt);
  KTB1[ns - 1] = 4.0 * pow((M_PI * taubot / dt), 0.5) *
                 (1.0 + erf(pow(0.5 * taubot / dt, 0.5)) -
                  2.0 * erf(pow(taubot / dt, 0.5)));

  // Second order
  KTA2[ns - 1] = 8.0 * exp(-4.0 * taubot / dt) -
                 4.0 * pow(2.0, 0.5) * exp(-2.0 * taubot / dt);
  KTB2[ns - 1] = -8.0 * pow((M_PI * taubot / dt), 0.5) *
                 (1.0 + erf(pow((2.0 * taubot / dt), 0.5)) -
                  2.0 * erf(2.0 * pow((taubot / dt), 0.5)));

  // Third order
  KTA3[ns - 1] = -8.0 * exp(-9.0 * taubot / dt) +
                 4.0 * pow(2.0, 0.5) * exp(-4.5 * taubot / dt);
  KTB3[ns - 1] = 12.0 * pow((M_PI * taubot / dt), 0.5) *
                 (1.0 + erf(pow((4.5 * taubot / dt), 0.5)) -
                  2.0 * erf(3.0 * pow((taubot / dt), 0.5)));

  // Calculate the kernel component vectors
  for (int i = 0; i < (ns - 1); i++) {

    // First order
    KT0[i] = 4.0 * pow(double(ns - i), 0.5) -
             2.0 * pow(double(ns + 1 - i), 0.5) -
             2.0 * pow(double(ns - 1 - i), 0.5);
    KTA1[i] =
        -8.0 * pow(double(ns - i), 0.5) * exp(-taubot / dt / double(ns - i)) +
        4.0 * pow(double(ns + 1 - i), 0.5) *
            exp(-taubot / dt / double(ns + 1 - i)) +
        4.0 * pow(double(ns - 1 - i), 0.5) *
            exp(-taubot / dt / double(ns - 1 - i));
    KTB1[i] = 4.0 * pow((M_PI * taubot / dt), 0.5) *
              (erf(pow((taubot / dt / double(ns - 1 - i)), 0.5)) +
               erf(pow((taubot / dt / double(ns + 1 - i)), 0.5)) -
               2.0 * erf(pow((taubot / dt / double(ns - i)), 0.5)));

    // Second order
    KTA2[i] = 8.0 * pow(double(ns - i), 0.5) *
                  exp(-4.0 * taubot / dt / double(ns - i)) -
              4.0 * pow(double(ns + 1 - i), 0.5) *
                  exp(-4.0 * taubot / dt / double(ns + 1 - i)) -
              4.0 * pow(double(ns - 1 - i), 0.5) *
                  exp(-4.0 * taubot / dt / double(ns - 1 - i));
    KTB2[i] = -8.0 * pow((M_PI * taubot / dt), 0.5) *
              (erf(2.0 * pow((taubot / dt / double(ns - 1 - i)), 0.5)) +
               erf(2.0 * pow((taubot / dt / double(ns + 1 - i)), 0.5)) -
               2.0 * erf(2.0 * pow((taubot / dt / double(ns - i)), 0.5)));

    // Third order
    KTA3[i] = -8.0 * pow(double(ns - i), 0.5) *
                  exp(-9.0 * taubot / dt / double(ns - i)) +
              4.0 * pow(double(ns + 1 - i), 0.5) *
                  exp(-9.0 * taubot / dt / double(ns + 1 - i)) +
              4.0 * pow(double(ns - 1 - i), 0.5) *
                  exp(-9.0 * taubot / dt / double(ns - 1 - i));
    KTB3[i] = 12.0 * pow((M_PI * taubot / dt), 0.5) *
              (erf(3.0 * pow((taubot / dt / double(ns - 1 - i)), 0.5)) +
               erf(3.0 * pow((taubot / dt / double(ns + 1 - i)), 0.5)) -
               2.0 * erf(3.0 * pow((taubot / dt / double(ns - i)), 0.5)));
  }

  // Sum up the kernel components
  for (int i = 0; i < ns; i++) {

    Ker[i] = KT0[i] + KTA1[i] + KTB1[i] + KTA2[i] + KTB2[i] + KTA3[i] + KTB3[i];
  }

  // Correction terms, remove oscillation artefacts due to short-term forcings
  // (Equation 2.3.27, TK07)
  C[0] = 1.0 / pow(taucfl, 2.0) + 1.0 / pow(taukls, 2.0) +
         2.0 / taucfl / taukls + bsi / taukls / tauksl;
  C[1] = -1 * bsi / pow(taukls, 2.0) - bsi / taucfl / taukls -
         bsi / taucfs / taukls - pow(bsi, 2.0) / taukls / tauksl;
  C[2] = -1 * bsi / pow(tauksl, 2.0) - 1.0 / taucfs / tauksl -
         1.0 / taucfl / tauksl - 1.0 / taukls / tauksl;
  C[3] = 1.0 / pow(taucfs, 2.0) + pow(bsi, 2.0) / pow(tauksl, 2.0) +
         2.0 * bsi / taucfs / tauksl + bsi / taukls / tauksl;

  for (int i = 0; i < 4; i++) {
    C[i] = C[i] * (pow(dt, 2.0) / 12.0);
  }

  //------------------------------------------------------------------
  // Matrices of difference equation system, see A.27 Kriegler 2005.
  // B*T(i+1) = Q(i) + A*T(i)
  // T = (TL,TS)
  // successor temperatures (TL;i+1; TS;i+1)
  B[0] = 1.0 + dt / (2.0 * taucfl) + dt / (2.0 * taukls);
  B[1] = -dt / (2.0 * taukls) * bsi;
  B[2] = -dt / (2.0 * tauksl);
  B[3] = 1.0 + dt / (2.0 * taucfs) + dt / (2.0 * tauksl) * bsi +
         2.0 * fso * pow((dt / taudif), 0.5);

  // predecessors temperatures
  A[0] = 1.0 - dt / (2.0 * taucfl) - dt / (2.0 * taukls);
  A[1] = dt / (2.0 * taukls) * bsi;
  A[2] = dt / (2.0 * tauksl);
  A[3] = 1.0 - dt / (2.0 * taucfs) - dt / (2.0 * tauksl) * bsi +
         Ker[ns - 1] * fso * pow((dt / taudif), 0.5);

  // The algorithm to integrate Model
  for (int i = 0; i < 4; i++) {
    B[i] = B[i] + C[i];
    A[i] = A[i] + C[i];
  }
  // Calculate the inverse of B
  invert_1d_2x2_matrix(B, IB);
}

//------------------------------------------------------------------------------
// documentation is inherited
void TemperatureComponent::run(const double runToDate) {
  H_LOG(logger, Logger::DEBUG) << "temperature run " << runToDate << std::endl;

  // Commented section below is for case of user-specified temperature record.
  // Temperature component can't handle that at the moment!

  //// We track total radiative forcing using internal variable `internal_Ftot`
  //// Need to do this because if we're subject to a user constraint (being
  /// forced / to match a temperature record), need to track the Ftot that
  ///*would* have / produced the observed temperature record. This way there's a
  /// smooth / transition when we exit the constraint period, after which
  /// internal_Ftot / will rise in parallel with the value reported by
  /// ForcingComponent.

  // If we never had any temperature constraint, `internal_Ftot` will match
  // `Ftot`.

  // Some needed inputs
  int tstep = runToDate - core->getStartDate();

  // Calculate the total aresol forcing from aerosol-radiation interactions and
  // the aerosol-cloud interactions so that that total aerosol forcing can be
  // adjusted by the aerosol forcing scaling factor.
  double aero_forcing =
      core->sendMessage(M_GETDATA, D_RF_BC, message_data(runToDate))
          .value(U_W_M2) +
      core->sendMessage(M_GETDATA, D_RF_OC, message_data(runToDate))
          .value(U_W_M2) +
      core->sendMessage(M_GETDATA, D_RF_NH3, message_data(runToDate))
          .value(U_W_M2) +
      core->sendMessage(M_GETDATA, D_RF_SO2, message_data(runToDate))
          .value(U_W_M2) +
      core->sendMessage(M_GETDATA, D_RF_ACI, message_data(runToDate))
          .value(U_W_M2);

  double volcanic_forcing =
      double(core->sendMessage(M_GETDATA, D_RF_VOL, message_data(runToDate)));

  // Adjust total forcing to account for the aerosol and volcanic forcing
  // scaling factor
  forcing[tstep] =
      double(core->sendMessage(M_GETDATA, D_RF_TOTAL, message_data(runToDate))
                 .value(U_W_M2)) -
      (1.0 - alpha) * aero_forcing - (1.0 - volscl) * volcanic_forcing;

  // Initialize variables for time-stepping through the model
  double DQ1 = 0.0;
  double DQ2 = 0.0;
  double QC1 = 0.0;
  double QC2 = 0.0;
  double DelQL = 0.0;
  double DelQO = 0.0;
  double DPAST1 = 0.0;
  double DPAST2 = 0.0;
  double DTEAUX1 = 0.0;
  double DTEAUX2 = 0.0;

  // Reset the endogenous varibales for this time step
  temp[tstep] = 0.0;
  temp_landair[tstep] = 0.0;
  temp_sst[tstep] = 0.0;
  heat_mixed[tstep] = 0.0;
  heat_interior[tstep] = 0.0;
  heatflux_mixed[tstep] = 0.0;
  heatflux_interior[tstep] = 0.0;

  // Assume land and ocean forcings are equal to global forcing
  std::vector<double> QL = forcing;
  std::vector<double> QO = forcing;

  if (tstep > 0) {

    DelQL = QL[tstep] - QL[tstep - 1];
    DelQO = QO[tstep] - QO[tstep - 1];

    // Assume linear forcing change between tstep and tstep+1
    QC1 = (DelQL / cal * (1.0 / taucfl + 1.0 / taukls) -
           bsi * DelQO / cas / taukls);
    QC2 = (DelQO / cas * (1.0 / taucfs + bsi / tauksl) - DelQL / cal / tauksl);
    QC1 = QC1 * pow(dt, 2.0) / 12.0;
    QC2 = QC2 * pow(dt, 2.0) / 12.0;

    // ----------------- Initial Conditions --------------------
    // Initialization of temperature and forcing vector:
    // Factor 1/2 in front of Q in Equation A.27, EK05, and Equation 2.3.27,
    // TK07 is a typo! Assumption: linear forcing change between n and n+1
    DQ1 = 0.5 * dt / cal * (QL[tstep] + QL[tstep - 1]);
    DQ2 = 0.5 * dt / cas * (QO[tstep] + QO[tstep - 1]);
    DQ1 = DQ1 + QC1;
    DQ2 = DQ2 + QC2;

    // ---------- SOLVE MODEL ------------------
    // Calculate temperatures
    for (int i = 0; i <= tstep; i++) {
      DPAST2 = DPAST2 + temp_sst[i] * Ker[ns - tstep + i - 1];
    }
    DPAST2 = DPAST2 * fso * pow((dt / taudif), 0.5);

    DTEAUX1 = A[0] * temp_landair[tstep - 1] + A[1] * temp_sst[tstep - 1];
    DTEAUX2 = A[2] * temp_landair[tstep - 1] + A[3] * temp_sst[tstep - 1];

    temp_landair[tstep] =
        IB[0] * (DQ1 + DPAST1 + DTEAUX1) + IB[1] * (DQ2 + DPAST2 + DTEAUX2);
    temp_sst[tstep] =
        IB[2] * (DQ1 + DPAST1 + DTEAUX1) + IB[3] * (DQ2 + DPAST2 + DTEAUX2);
  } else { // Handle the initial conditions
    temp_landair[0] = 0.0;
    temp_sst[0] = 0.0;
  }
  temp[tstep] =
      flnd * temp_landair[tstep] + (1.0 - flnd) * bsi * temp_sst[tstep];

  // If the user has supplied temperature data, use that instead
  if (tas_constrain.size() && runToDate >= tas_constrain.firstdate() &&
      runToDate <= tas_constrain.lastdate()) {
    H_LOG(logger, Logger::WARNING)
        << "** Overwriting temperatures with user-supplied value" << std::endl;
    temp[tstep] = tas_constrain.get(runToDate);

    // Now back-calculate the land and ocean values, overwriting what was
    // computed above
    temp_landair[tstep] =
        (temp[tstep] - (1.0 - flnd) * bsi * temp_sst[tstep]) / flnd;
    temp_sst[tstep] =
        (temp[tstep] - flnd * temp_landair[tstep]) / ((1.0 - flnd) * bsi);
  }

  // Calculate ocean heat uptake [W/m^2]
  // heatflux[tstep] captures in the heat flux in the period between tstep-1 and
  // tstep. Numerical implementation of Equation 2.7, EK05, or Equation 2.3.13,
  // TK07)
  // ------------------------------------------------------------------------
  if (tstep > 0) {
    heatflux_mixed[tstep] = cas * (temp_sst[tstep] - temp_sst[tstep - 1]);
    for (int i = 0; i < tstep; i++) {
      heatflux_interior[tstep] =
          heatflux_interior[tstep] + temp_sst[i] * Ker[ns - tstep + i];
    }
    heatflux_interior[tstep] =
        cas * fso / pow((taudif * dt), 0.5) *
        (2.0 * temp_sst[tstep] - heatflux_interior[tstep]);
    heat_mixed[tstep] =
        heat_mixed[tstep - 1] + heatflux_mixed[tstep] * (powtoheat * dt);
    heat_interior[tstep] = heat_interior[tstep - 1] +
                           heatflux_interior[tstep] * (fso * powtoheat * dt);
  }

  else { // Handle the initial conditions
    heatflux_mixed[0] = 0.0;
    heatflux_interior[0] = 0.0;
    heat_mixed[0] = 0.0;
    heat_interior[0] = 0.0;
  }

  setoutputs(tstep);
  H_LOG(logger, Logger::DEBUG)
      << " tas=" << tas << " in " << runToDate << std::endl;
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval TemperatureComponent::getData(const std::string &varName,
                                      const double date) {

  unitval returnval;

  // If a date is supplied, get the needed value from the
  // vectors.  Some values, such as model parameters, don't have
  // time-indexed values, so asking for one of those with a date
  // is an error.
  H_ASSERT(date <= core->getCurrentDate(), "Date must be <= current date.");
  int tstep = date - core->getStartDate();

  if (varName == D_GLOBAL_TAS) {
    if (date == Core::undefinedIndex()) {
      returnval = tas;
    } else {
      returnval = unitval(temp[tstep], U_DEGC);
    }
  } else if (varName == D_LAND_TAS) {
    if (lo_warming_ratio != 0) {
      if (date == Core::undefinedIndex()) {
        returnval = lo_tas_land;
      } else {
        returnval = unitval(lo_temp_landair[tstep], U_DEGC);
      }
    } else {
      if (date == Core::undefinedIndex()) {
        returnval = tas_land;
      } else {
        returnval = unitval(temp_landair[tstep], U_DEGC);
      }
    }
  } else if (varName == D_OCEAN_TAS) {
    if (lo_warming_ratio != 0) {
      if (date == Core::undefinedIndex()) {
        returnval = lo_tas_ocean;
      } else {
        returnval = unitval(lo_temp_oceanair[tstep], U_DEGC);
      }
    } else {
      if (date == Core::undefinedIndex()) {
        returnval = tas_ocean;
      } else {
        returnval = bsi * unitval(temp_sst[tstep], U_DEGC);
      }
    }
  } else if (varName == D_SST) {
    if (lo_warming_ratio != 0) {
      if (date == Core::undefinedIndex()) {
        returnval = lo_seast;
      } else {
        returnval = unitval(lo_sst[tstep], U_DEGC);
      }
    } else {
      if (date == Core::undefinedIndex()) {
        returnval = sst;
      } else {
        returnval = unitval(temp_sst[tstep], U_DEGC);
      }
    }
  } else if (varName == D_FLUX_MIXED) {
    if (date == Core::undefinedIndex()) {
      returnval = flux_mixed;
    } else {
      returnval = unitval(heatflux_mixed[tstep], U_W_M2);
    }
  } else if (varName == D_FLUX_INTERIOR) {
    if (date == Core::undefinedIndex()) {
      returnval = flux_interior;
    } else {
      returnval = unitval(heatflux_interior[tstep], U_W_M2);
    }
  } else if (varName == D_HEAT_FLUX) {
    if (date == Core::undefinedIndex()) {
      returnval = heatflux;
    } else {
      double value = heatflux_mixed[tstep] + fso * heatflux_interior[tstep];
      returnval = unitval(value, U_W_M2);
    }
  } else if (varName == D_TAS_CONSTRAIN) {
    H_ASSERT(date != Core::undefinedIndex(),
             "Date required for tas constraint");
    if (tas_constrain.exists(date)) {
      returnval = tas_constrain.get(date);
    } else {
      H_LOG(logger, Logger::DEBUG)
          << "No tas constraint for requested date " << date
          << ". Returning missing value." << std::endl;
      returnval = unitval(MISSING_FLOAT, U_DEGC);
    }
  } else if (varName == D_DIFFUSIVITY) {
    H_ASSERT(date == Core::undefinedIndex(),
             "Date not allowed for diffusivity");
    returnval = diff;
  } else if (varName == D_AERO_SCALE) {
    H_ASSERT(date == Core::undefinedIndex(),
             "Date not allowed for aero scaler");
    returnval = alpha;
  } else if (varName == D_ECS) {
    H_ASSERT(date == Core::undefinedIndex(), "Date not allowed for ECS");
    returnval = S;
  } else if (varName == D_VOLCANIC_SCALE) {
    H_ASSERT(date == Core::undefinedIndex(),
             "Date not allowed for volcanic scaler");
    returnval = volscl;
  } else if (varName == D_QCO2) {
    H_ASSERT(date == Core::undefinedIndex(), "Date not allowed for q2co2");
    returnval = unitval(qco2, U_W_M2);
  } else if (varName == D_LO_WARMING_RATIO) {
    H_ASSERT(date == Core::undefinedIndex(),
             "Date not allowed for land ocean warming ratio");
    returnval = lo_warming_ratio;
    return returnval;
  } else {
    H_THROW("Caller is requesting unknown variable: " + varName);
  }

  return returnval;
}
//------------------------------------------------------------------------------
// documentation is inherited
void TemperatureComponent::reset(double time) {
  // We take a slightly different approach in this component's reset method than
  // we have in other components.  The temperature component doesn't have its
  // own time counter, and it stores its history in a collection of double
  // vectors.  Therefore, all we do here is set the unitval versions of that
  // stored data to their values from the vectors.
  if (time < core->getStartDate()) // in this case, reset to the starting value.
    time = core->getStartDate();

  int tstep = time - core->getStartDate();
  setoutputs(tstep);
  H_LOG(logger, Logger::NOTICE)
      << getComponentName() << " reset to time= " << time << "\n";
}

//------------------------------------------------------------------------------
// documentation is inherited
void TemperatureComponent::shutDown() {
  H_LOG(logger, Logger::DEBUG) << "goodbye " << getComponentName() << std::endl;
  logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void TemperatureComponent::accept(AVisitor *visitor) { visitor->visit(this); }

void TemperatureComponent::setoutputs(int tstep) {
  double temp_oceanair;

  flux_mixed.set(heatflux_mixed[tstep], U_W_M2, 0.0);
  flux_interior.set(heatflux_interior[tstep], U_W_M2, 0.0);
  heatflux.set(heatflux_mixed[tstep] + fso * heatflux_interior[tstep], U_W_M2,
               0.0);
  tas.set(temp[tstep], U_DEGC, 0.0);
  tas_land.set(temp_landair[tstep], U_DEGC, 0.0);
  sst.set(temp_sst[tstep], U_DEGC, 0.0);
  temp_oceanair = bsi * temp_sst[tstep];
  tas_ocean.set(temp_oceanair, U_DEGC, 0.0);

  // If a user provided land-ocean warming ratio is provided, use it to over
  // write DOECLIM's land & ocean temperature.
  if (lo_warming_ratio != 0) {

    // Calculations using tas weighted average and ratio (land warming/ocean
    // warming = lo_warming_ratio)
    double temp_oceanair_constrain =
        temp[tstep] / ((lo_warming_ratio * flnd) + (1 - flnd));
    double temp_landair_constrain = temp_oceanair_constrain * lo_warming_ratio;
    double temp_sst_constrain = temp_oceanair_constrain / bsi;

    lo_temp_landair[tstep] = temp_landair_constrain;
    lo_temp_oceanair[tstep] = temp_oceanair_constrain;
    lo_sst[tstep] = temp_sst_constrain;

    // Store these the values, notes these are values with non dates.
    lo_tas_land.set(temp_landair_constrain, U_DEGC, 0.0);
    lo_seast.set(temp_sst_constrain, U_DEGC, 0.0);
    lo_tas_ocean.set(temp_oceanair_constrain, U_DEGC, 0.0);
  }

  H_LOG(logger, Logger::DEBUG)
      << "Land-ocean warming ratio: " << tas_land / tas_ocean << std::endl;
  H_LOG(logger, Logger::DEBUG) << "Global: " << tas << std::endl;
  H_LOG(logger, Logger::DEBUG) << "Land Temp: " << tas_land << std::endl;
  H_LOG(logger, Logger::DEBUG) << "Ocean Temp: " << tas_ocean << std::endl;
}

} // namespace Hector
