/* Hector -- A Simple Climate Model
   Copyright (C) 2022  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef FORCING_COMPONENT_H
#define FORCING_COMPONENT_H
/*
 *  forcing_component.hpp
 *  hector
 *
 *  Created by Ben on 02 March 2011.
 *
 */

#include "imodel_component.hpp"
#include "tseries.hpp"
#include "tvector.hpp"

namespace Hector {

// Need to forward declare the components which depend on each other
class SimpleNbox;
class HalocarbonComponent;

//------------------------------------------------------------------------------
/*! \brief The forcing component.
 *
 */
class ForcingComponent : public IModelComponent {
  friend class CSVOutputStreamVisitor;

public:
  ForcingComponent();
  ~ForcingComponent();

  //! IModelComponent methods
  virtual std::string getComponentName() const;

  virtual void init(Core *core);

  virtual unitval sendMessage(const std::string &message,
                              const std::string &datum,
                              const message_data info = message_data());

  virtual void setData(const std::string &varName, const message_data &data);

  virtual void prepareToRun();

  virtual void run(const double runToDate);

  virtual void reset(double date);

  virtual void shutDown();

  //! IVisitable methods
  virtual void accept(AVisitor *visitor);

  //! A list (map) of all computed forcings, with associated iterator
  typedef std::map<std::string, unitval> forcings_t;
  typedef std::map<std::string, unitval>::iterator forcingsIterator;

private:
  virtual unitval getData(const std::string &varName, const double valueIndex);

  //! Base year forcings
  forcings_t baseyear_forcings;
  //! Forcings by year
  tvector<forcings_t> forcings_ts;

  double baseyear;    //! Year which forcing calculations will start
  double currentYear; //! Tracks current year
  unitval C0;         //! Records base year atmospheric CO2
  tseries<unitval> Ftot_constrain; //! Total forcing can be supplied
  tseries<unitval> Fmisc_ts; //! Miscellaneous forcing can be supplied via ini

  // CO2 parameters
  const double a1 =
      -2.4785e-7; // (W m–2 ppm–2) IPCC AR6 parameter for CO2 SARF Table 7.SM.1
  const double b1 =
      7.5906e-4; // (W m–2 ppm–1) IPCC AR6 parameter for CO2 SARF Table 7.SM.1
  const double c1 = -2.1492e-3; // (W m–2 ppb–1/2) IPCC AR6 parameter for CO2
                                // SARF Table 7.SM.1
  const double d1 =
      5.2488;        // (W m–2) IPCC AR6 parameter for CO2 SARF Table 7.SM.1
  unitval delta_co2; // forcing tropospheric adjustment for CO2 see 7.3.2.1 of
                     // IPCC AR6

  // N2O parameters
  const double a2 =
      -3.4197e-4; // (W m–2 ppm–1) IPCC AR6 parameter for N2O SARF Table 7.SM.1
  const double b2 =
      2.5455e-4; // (W m–2 ppb–1) IPCC AR6 parameter for N2O SARF Table 7.SM.1
  const double c2 =
      -2.4357e-4; // (W m–2 ppb–1) IPCC AR6 parameter for N2O SARF Table 7.SM.1
  const double d2 =
      0.12173; // (W m-2 ppb–1/2) IPCC AR6 parameter for N2O SARF Table 7.SM.1
  unitval delta_n2o; // forcing tropospheric adjustment for N2O see 7.3.2.3 of
                     // IPCC AR6

  // CH4 parameters
  const double a3 =
      -8.9603e-5; // (W m–2 ppb–1) IPCC AR6 parameter for CH4 SARF Table 7.SM.1
  const double b3 =
      -1.2462e-4; // (W m–2 ppb–1) IPCC AR6 parameter for CH4 SARF Table 7.SM.1
  const double d3 =
      0.045194; // (W m–2 ppb–1/2) IPCC AR6 parameter for CH4 SARF Table 7.SM.1
  unitval delta_ch4; // forcing tropospheric adjustment for CH4 see 7.3.2.2 of
                     // IPCC AR6
  // Stratosphieric water vapor (strat. H2O) parameters
  const double stratH2O_H2 =
    0.00019; // W m −2 per Tg yr− 1 from Sand et al. 2023
    
    
    
  // Aerosol parameters for aerosol-radiation interactions (RFari) see
  // equation 7.SM.1.1 of IPCC AR6
  unitval rho_bc; // (W yr m–2 Tg–1) IPCC AR6 radiative efficiency BC 7.SM.1.3
  unitval rho_oc; // (W yr m–2 Tg–1) IPCC AR6 radiative efficiency OC 7.SM.1.3
  unitval
      rho_so2; // (W yr m–2 Gg–1) IPCC AR6 radiative efficiency SO2 7.SM.1.3.1
  unitval
      rho_nh3; // (W yr m–2 Tg–1) IPCC AR6 radiative efficiency SO2 7.SM.1.3.1

  // Aerosol parameters for aerosol-cloud interactions (RFaci) see equation
  // Equation 7.SM.1.2 of IPCC AR6
  double const aci_beta = 2.279759; //  Dorheim et al. 2024
  double const s_BCOC = 111.05064063; // (Tg C yr-1) IPCC AR6 7.SM.1.3.1
  double const s_SO2 =
      (260.34644166 * 1000) *
      (32.065 /
       64.066); // (Tg SO2 yr–1) IPCC AR6 7.SM.1.3.1 converted to (Gg S yr-1)

  Core *core;    //! Core
  Logger logger; //! Logger

  static const char
      *adjusted_halo_forcings[]; //! Capability strings for halocarbon forcings
  static const char
      *halo_forcing_names[]; //! Internal names of halocarbon forcings
  static std::map<std::string, std::string> forcing_name_map;
};

} // namespace Hector

#endif // FORCING_COMPONENT_H
