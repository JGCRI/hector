/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef OCEANBOX_HPP_
#define OCEANBOX_HPP_

/* oceanbox.hpp
 *
 * Header file for the oceanbox class.
 *
 * Created by Corinne Hartin on 1/31/13
 *
 */

#include "math.h"
#include <algorithm>
#include <map>
#include <stdio.h>
#include <string>
#include <vector>

#include "fluxpool.hpp"
#include "logger.hpp"
#include "ocean_csys.hpp"
#include "unitval.hpp"

// Mean absolute global tos temperature, preindustrial (deg C), this is used by
// the ocean component which, requires absolute temperature to calculate ocean
// chemistry, the CMIP6 multi model mean from 1850 - 1860. See hector_cmip6data
// for details.
#define MEAN_TOS_TEMP 18

namespace Hector {

class oceanbox {
  /*! /brief  An ocean box
   *
   *  Implements an ocean box, which can have arbitrary connections to
   *  other boxes, may (or not) exchange carbon and heat with the atmosphere,
   *  and may (or not) have active chemistry.
   */
private:
  fluxpool carbon;
  fluxpool CarbonAdditions, CarbonSubtractions;
  std::vector<oceanbox *> connection_list; ///< a vector of ocean box pointers
  std::vector<double> connection_k; ///< a vector of ocean k values (fraction)
  std::vector<int>
      connection_window; ///< a vector of connection windows to average over

  std::string Name;

  unitval CO2_conc;      ///< Atmospheric [CO2], ppm
  unitval Tbox;          ///< box absolute temperature, degC
  unitval pco2_lastyear; //
  unitval dic_lastyear;  //
  unitval compute_tabsC(const unitval SST) const;
  fluxpool ao_flux; //!< atmosphere -> ocean flux
  fluxpool oa_flux; //!< ocean -> atmosphere flux

public:
  oceanbox(); // constructor

  std::map<oceanbox *, unitval>
      annual_box_fluxes; ///< Map of our fluxes to other boxes

  void initbox(double C, std::string name = "");
  void make_connection(oceanbox *ob, const double k, const int window);
  void compute_fluxes(const unitval current_Ca, const fluxpool atmosphere_cpool,
                      const double yf, const bool do_circ = true);
  void log_state();
  void update_state();
  void new_year(const unitval SST);
  void separate_surface_fluxes(fluxpool atmosphere_pool);

  void set_carbon(const unitval C);
  fluxpool get_carbon() const { return carbon; };
  fluxpool get_oa_flux() const { return oa_flux; };
  fluxpool get_ao_flux() const { return ao_flux; };

  void add_carbon(fluxpool C);

  void start_tracking();

  // Functions to get internal box data
  unitval get_Tbox() const { return Tbox; };
  unitval calc_revelle();
  unitval deltaT; ///< difference between box temperature and global temperature
  unitval preindustrial_flux;
  bool surfacebox;

  // Ocean box chemistry
  oceancsys mychemistry; ///< box chemistry
  bool active_chemistry; ///< box has active chemistry model?
  void chem_equilibrate(const unitval current_Ca); ///< equilibrate chemistry
                                                   ///< model to a given flux
  double fmin(double alk, void *params);

  unitval atmosphere_flux; //!< positive is atmosphere -> ocean flux, negative
                           //!< ocean -> atmosphere

  // logger
  Logger *logger;
};

} // namespace Hector

#endif
