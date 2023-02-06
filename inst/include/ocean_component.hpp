/* Hector -- A Simple Climate Model
   Copyright (C) 2022  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef OCEAN_COMPONENT_H
#define OCEAN_COMPONENT_H
/*
 *  ocean_component.hpp
 *  hector
 *
 *  Created by Corinne Hartin on 1/3/13.
 *
 */

#include "carbon-cycle-model.hpp"
#include "logger.hpp"
#include "ocean_csys.hpp"
#include "oceanbox.hpp"
#include "tseries.hpp"
#include "tvector.hpp"
#include "unitval.hpp"

#define OCEAN_MAX_TIMESTEP 1.0 //!< max/default timestep (yr)
#define OCEAN_MIN_TIMESTEP 0.3 //!< minimum timestep (yr)
#define OCEAN_TSR_FACTOR 0.5   //!< timestep reduction factor when necessary
#define OCEAN_TSR_TIMEOUT 20   //!< years we lock into reduced timestep
#define OCEAN_TSR_TRIGGER1                                                     \
  0.1 //!< trigger1 to reduce timestep:
      //!< absolute diff between successive annual fluxes (Pg C)

namespace Hector {

//------------------------------------------------------------------------------
/*! \brief Ocean model component.
 *
 *  A simple yet classy ocean model that handles surface carbon chemistry, flows
 *  between layers and latitudinal regions, etc.
 */
class OceanComponent : public CarbonCycleModel {
  friend class CSVFluxPoolVisitor;

public:
  OceanComponent();
  ~OceanComponent();

  //! IModelComponent methods
  virtual std::string getComponentName() const;

  virtual void init(Core *core);

  virtual unitval sendMessage(const std::string &message,
                              const std::string &datum,
                              const message_data info = message_data());

  virtual void setData(const std::string &varName, const message_data &data);

  virtual void prepareToRun();

  virtual void run(const double runToDate);

  virtual bool run_spinup(const int step);

  virtual void reset(double time);

  virtual void shutDown();

  //! IVisitable methods
  virtual void accept(AVisitor *visitor);

  // Carbon cycle model interface
  void getCValues(double t, double c[]);
  int calcderivs(double t, const double c[], double dcdt[]) const;
  void slowparameval(double t, const double c[]);
  void stashCValues(double t, const double c[]);
  void record_state(double t);
  void set_atmosphere_sources(fluxpool atm) { atmosphere_cpool = atm; };
  fluxpool get_oaflux() const;
  fluxpool get_aoflux() const;

private:
  virtual unitval getData(const std::string &varName, const double date);

  // Define high and low latitude
  // The cold high-latitude surface box makes up 15% of the total ocean surface
  // area and has latitude >55; the warm low-latitude surface box makes up the
  // rest.
  const double part_high = 0.15;
  const double part_low = 1 - part_high;

  /*****************************************************************
   * State variables for the component
   * All of these will need to be recorded at the end of a timestep,
   * except for the spinup flag.
   *****************************************************************/
  // Ocean boxes
  oceanbox surfaceHL; //!< surface high latitude box 100m
  oceanbox surfaceLL; //!< surface low latitude box 100m
  oceanbox inter;     //!< intermediate box 1000m
  oceanbox deep;      //!< deep box 3000m

  // Atmosphere conditions
  unitval SST;      //!< Ocean surface temperature anomaly, degC
  unitval CO2_conc; //!< Atmospheric CO2, ppm
  fluxpool atmosphere_cpool;

  // Atmosphere-ocean flux
  unitval annualflux_sum, annualflux_sumHL,
      annualflux_sumLL; //!< Running annual totals atm-ocean flux, for output
                        //!< reporting
  unitval lastflux_annualized; //!< Last atm-ocean flux when solver ordered us
                               //!< to 'stash' C values

  // Spinup mode flag
  bool in_spinup; //!< Are we currently in spinup?

  /*****************************************************************
   * Model parameters
   *****************************************************************/
  // Ocean circulation parameters
  unitval tt;  //!< m3/s thermohaline overturning
  unitval tu;  //!< m3/s high latitude overturning
  unitval twi; //!< m3/s warm-intermediate exchange
  unitval tid; //!< m3/s intermediate-deep exchange

  /*****************************************************************
   * Input data
   *****************************************************************/
  bool spinup_chem; //!< run chemistry during spinup?

  /*****************************************************************
   * Private helper functions
   *****************************************************************/
  fluxpool totalcpool() const;
  unitval annual_totalcflux(const double date, const unitval &CO2_conc,
                            const double cpoolscale = 1.0) const;

  /*****************************************************************
   * Adaptive timestep control
   * max_timestep and reduced_timestep_timeout will need to be recorded.
   * The timesteps counter is set to zero at the start of each year.
   *****************************************************************/
  double max_timestep; //!< Current maximum timestep allowed. This can change
  int reduced_timestep_timeout; //!< Timer that keeps track of how long we've
                                //!< had a reduced timestep
  int timesteps;                //!< Number of timesteps taken in current year

  /*****************************************************************
   * Recording variables
   * These are used to record the component state over time so that
   * we can reset to a previous time.
   *****************************************************************/
  // Ocean initial state
  unitval preind_C_surface; //!< Carbon in the preindustrial surface pool
  unitval
      preind_C_ID; //!< Carbon in the preindustrial intermediate and deep pool

  // Ocean boxes over time
  tvector<oceanbox> surfaceHL_tv;
  tvector<oceanbox> surfaceLL_tv;
  tvector<oceanbox> inter_tv;
  tvector<oceanbox> deep_tv;

  // Ocean conditions over time
  tseries<unitval> SST_ts;
  tseries<unitval> annualflux_sum_ts;
  tseries<unitval> annualflux_sumHL_ts;
  tseries<unitval> annualflux_sumLL_ts;
  tseries<unitval> lastflux_annualized_ts;
  tseries<unitval> Ca_ts;
  tseries<unitval> Ca_HL_ts;
  tseries<unitval> Ca_LL_ts;
  tseries<unitval> C_IO_ts;
  tseries<unitval> C_DO_ts;
  tseries<unitval> PH_HL_ts;
  tseries<unitval> PH_LL_ts;
  tseries<unitval> pco2_HL_ts;
  tseries<unitval> pco2_LL_ts;
  tseries<unitval> dic_HL_ts;
  tseries<unitval> dic_LL_ts;
  tseries<unitval> temp_HL_ts;
  tseries<unitval> temp_LL_ts;
  tseries<unitval> co3_HL_ts;
  tseries<unitval> co3_LL_ts;

  // timestep control
  tseries<double> max_timestep_ts;
  tseries<int> reduced_timestep_timeout_ts;

  //! logger
  Logger logger;
};

} // namespace Hector

#endif // OCEAN_COMPONENT_H
