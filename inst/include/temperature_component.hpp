/* Hector -- A Simple Climate Model
   Copyright (C) 2022  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef TEMPERATURE_COMPONENT_H
#define TEMPERATURE_COMPONENT_H
/*
 *  temperature_component.hpp
 *  hector
 *
 *  Created by BenV-W on 11/01/2016.
 *
 */

#include "forcing_component.hpp"
#include "imodel_component.hpp"
#include "logger.hpp"
#include "tseries.hpp"
#include "unitval.hpp"

namespace Hector {

//------------------------------------------------------------------------------
/*! \brief Temperature component.
 *
 *  A component that computes mean global temperature from radiative forcing
 *  using Diffusion Ocean Energy balance CLIMate model
 *  (DOECLIM; Kriegler, 2005; Tanaka and Kriegler, 2007).
 *  Adopted with permission from C++ implementation
 *  (https://github.com/scrim-network/cdice_doeclim; Garner et al., 2016).
 *
 *  Kriegler, E. (2005) Imprecise probability analysis for Integrated Assessment
 * of climate change. Ph.D. dissertation. Potsdam Universität. 256 pp.
 * (http://opus.kobv.de/ubp/volltexte/2005/561/; DOECLIM introduced in Chapter 2
 * and Annexes A and B) Tanaka, K. & Kriegler, E. (2007) Aggregated carbon
 * cycle, atmospheric chemistry, and climate model (ACC2) – Description of the
 * forward and inverse modes – . Reports Earth Syst. Sci. 199. Garner, G., Reed,
 * P. & Keller, K. (2016) Climate risk management requires explicit
 * representation of societal trade-offs. Clim. Change 134, 713–723.
 */
class TemperatureComponent : public IModelComponent {

public:
  TemperatureComponent();
  ~TemperatureComponent();

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

private:
  virtual unitval getData(const std::string &varName, const double date);
  void invert_1d_2x2_matrix(double *x, double *y);
  void setoutputs(int tstep);

  // Hard-coded DOECLIM parameters
  const double dt = 1;    // years per timestep (this is implicit in Hector)
  int ns;                 // number of timesteps
  const double ak = 0.31; // slope in climate feedback - land-sea heat exchange
                          // linear relationship (W/m2/K)
  const double bk = 1.59; // offset in climate feedback - land-sea heat exchange
                          // linear relationship,(W/m2/K)
  const double csw = 0.13; // specific heat capacity of seawater (Wyr/(m3K))
  const double earth_area = 5100656E8; // (m2)
  const double secs_per_Year =
      60.0 * 60.0 * 24.0 *
      365.2422; //  secs. * min. * hrs. * tropical calendar days (seconds)
  const double rlam = 1.43;   // factor between land clim. sens. and sea surface
                              // clim. sens. T_L2x = rlam*T_S2x (unitless)
  const double zbot = 4000.0; // bottom depth of the interior ocean (m)
  const double bsi = 1.3; // warming factor for marine surface air over SST due
                          // to retreating sea ice, (unitless)
  const double cal =
      0.52; // effective heat capacity of land-troposphere system (W*yr/m2/K)
  const double cas = 7.80; // effective heat capacity of mixed layer-troposphere
                           // system (W*yr/m2/K)
  const double flnd = 0.29; // fractional land area (unitless)
  const double fso = 0.95;  // ocean fractional area below 60m (unitless)

  // DOECLIM parameters calculated from constants above
  double kcon;       // conversion from cm2/s to m2/yr
  double ocean_area; // m2
  double cnum; // factor from sea-surface climate sensitivity to global mean
  double cden; // denominator used to figure out the climate senstivity feedback
               // parameters over land & sea
  double cfl;  // land climate feedback parameter, W/m2/K
  double cfs;  // sea climate feedback parameter, W/m2/K
  double kls;  // land-sea heat exchange coefficient, W/m2/K
  double keff; // ocean heat diffusivity, m2/yr
  double taubot;    // ocean bottom diffusion time scale, yr
  double powtoheat; // convert flux to total ocean heat 1E22 m2*s
  double taucfs;    // sea climate feedback time scale, yr
  double taucfl;    // land climate feedback time scale, yr
  double taudif;    // interior ocean heat uptake time scale, yr
  double tauksl;    // sea-land heat exchange time scale, yr
  double taukls;    // land-sea heat exchange time scale, yr
  double qco2;      // radiative forcing for atmospheric CO2 doubling

  std::vector<double> KT0;
  std::vector<double> KTA1;
  std::vector<double> KTB1;
  std::vector<double> KTA2;
  std::vector<double> KTB2;
  std::vector<double> KTA3;
  std::vector<double> KTB3;

  // Components of the difference equation system B*T(i+1) = Q(i) + A*T(i)
  double B[4];
  double C[4];
  std::vector<double> Ker;
  double A[4];
  double IB[4];

  // Time series arrays that are updated with each DOECLIM time-step
  std::vector<double> temp;
  std::vector<double> temp_landair;
  std::vector<double> temp_sst;
  std::vector<double> heatflux_mixed;
  std::vector<double> heatflux_interior;
  std::vector<double> heat_mixed;
  std::vector<double> heat_interior;
  std::vector<double> forcing;

  // Model parameters
  unitval S;      //!< climate sensitivity for 2xCO2, deg C
  unitval diff;   //!< ocean heat diffusivity, cm2/s
  unitval alpha;  //!< aerosol forcing factor, unitless
  unitval volscl; //!< volcanic forcing scaling factor, unitless

  // Model outputs
  unitval tas;       //!< global average surface air temperature anomaly, deg C
  unitval tas_land;  //!< average air temperature anomaly over land, deg C
  unitval tas_ocean; //!< global average ocean surface air temperature anomaly,
                     //!< deg C
  unitval
      sst; //!< global average ocean surface (water) temperature anomaly, deg C
  unitval flux_mixed;    //!< heat flux into mixed layer of ocean, W/m2
  unitval flux_interior; //!< heat flux into interior layer of ocean, W/m2
  unitval heatflux;      //!< heat flux into ocean, W/m2

  // Potential user defined model parameters
  unitval
      lo_warming_ratio; //!< user defined land ocean warming ratio, unit-less
                        //!< ratio defined as ΔTL/ΔTO; see
                        //!< https://agupubs.onlinelibrary.wiley.com/doi/full/10.1002/grl.50971

  // Create a place to store temp time series that are over written when
  // a user provided land ocean warming ratio is being used.
  unitval lo_tas_land;  //!< place to store land temp when lo is provided by
                        //!< users, deg C
  unitval lo_tas_ocean; //!< place to store ocean air temp when lo is provided
                        //!< by users, deg C
  unitval lo_seast; //!< place to store sst when lo is provided by users, deg C
  std::vector<double> lo_temp_landair;  //!< place to store land temp when lo is
                                        //!< provided by users, deg C
  std::vector<double> lo_temp_oceanair; //!< place to store land temp when lo is
                                        //!< provided by users, deg C
  std::vector<double>
      lo_sst; //!< place to store land temp when lo is provided by users, deg C

  tseries<unitval>
      tas_constrain; //! Temperature change to constrain model to, degC

  //! pointers to other components and stuff
  Core *core;

  //! logger
  Logger logger;
};

} // namespace Hector

#endif // TEMPERATURE_COMPONENT_H
