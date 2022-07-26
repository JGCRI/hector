/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef HALOCARBON_COMPONENT_HPP
#define HALOCARBON_COMPONENT_HPP
/*
 *  halocarbon_component.hpp
 *  hector
 *
 *  Created by Pralit Patel on 1/20/11.
 *
 */

#include "imodel_component.hpp"
#include "logger.hpp"
#include "tseries.hpp"
#include "unitval.hpp"

namespace Hector {

//------------------------------------------------------------------------------
/*! \brief Model component for a halocarbon.
 *
 *  A halocarbon model component that simply decays in the atmosphere.  Adapted
 *  from Bill Emanuel's python implementation.
 *
 */
class HalocarbonComponent : public IModelComponent {
  friend class CSVOutputStreamVisitor;

public:
  HalocarbonComponent(std::string g);
  virtual ~HalocarbonComponent();

  // IModelComponent methods
  virtual std::string getComponentName() const;

  virtual void init(Core *core);

  virtual unitval sendMessage(const std::string &message,
                              const std::string &datum,
                              const message_data info = message_data());

  virtual void setData(const std::string &varName, const message_data &data);

  virtual void prepareToRun();

  virtual void run(const double runToDate);

  virtual void reset(double time);

  virtual void shutDown();

  // IVisitable methods
  virtual void accept(AVisitor *visitor);

private:
  virtual unitval getData(const std::string &varName, const double valueIndex);

  //! Who are we?
  std::string myGasName;

  // Parameters
  double tau;    // lifetime in years
  unitval rho;   // radiative efficiencies W/m2/ppt
  unitval delta; // tropospheric adjustments scalar unitless

  //! Forcing [W/m^2]
  tseries<unitval> hc_forcing;
  tseries<unitval> emissions; //! Time series of emissions, pptv
  tseries<unitval> Ha_ts;     //! Time series of (ambient) concentration, pptv
  tseries<unitval> Ha_constrain; //! Concentration constraint, pptv
  unitval H0;                    //! Preindustrial concentration, pptv

  double molarMass;

  //! logger
  Logger logger;

  Core *core;
  double oldDate;
};

} // namespace Hector

#endif // HALOCARBON_COMPONENT_HPP
