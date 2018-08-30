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

#include <vector>
#include <string>
#include <map>
#include "math.h"
#include <stdio.h> 
#include <algorithm>

#include "logger.hpp"
#include "unitval.hpp"
#include "ocean_csys.hpp"

#define MEAN_GLOBAL_TEMP 15

namespace Hector {

class oceanbox {
    /*! /brief  An ocean box
     *
     *  Implements an ocean box, which can have arbitrary connections to
     *  other boxes, may (or not) exchange carbon and heat with the atmosphere,
     *  and may (or not) have active chemistry.
     */
private:
	unitval carbon;
	unitval CarbonToAdd;
	std::vector<oceanbox*> connection_list;  //<! a vector of ocean box pointers
	std::vector<double> connection_k;        //<! a vector of ocean k values (fraction)
	std::vector<double> carbonHistory;       //<! a vector of past C states
   	std::vector<double> carbonLossHistory;   //<! a vector of past C losses
	std::vector<int> connection_window;      //<! a vector of connection windows to average over

    double vectorHistoryMean( std::vector<double> v, int lookback ) const;
    
    unitval compute_connection_flux( int i, double yf ) const;
    
	std::string Name;
    
	void sens_parameters();
    
    unitval Ca;             //<! Atmospheric CO2, ppm
    unitval Tbox;           //<! box absolute temperature, degC
    unitval pco2_lastyear;  //
    unitval dic_lastyear;   //
    unitval compute_tabsC( const unitval Tgav ) const;
    
public:
	oceanbox (); // constructor
    
	std::map <oceanbox*, unitval> annual_box_fluxes;   //<! Map of our fluxes to other boxes
	
	void initbox( unitval C, std::string N="" );
    void make_connection( oceanbox* ob, const double k, const int window );
	void compute_fluxes( const unitval current_Ca, const double yf, const bool do_circ=true );
    void log_state();
	void update_state();
	void new_year( const unitval Tgav );

	void set_carbon( const unitval C );
	unitval get_carbon() const { return carbon; };
	void add_carbon( unitval C );
    
    bool oscillating( const unsigned lookback, const double maxamp, const int maxflips ) const;
    
    // Functions to get internal box data
    unitval get_Tbox() const { return Tbox; };
    
    unitval calc_revelle();

	unitval deltaT;     //<! difference between box temperature and global temperature
    unitval preindustrial_flux;
    bool surfacebox;
   
    double warmingfactor;        //!< regional warming relative to global (1.0=same)

    // Ocean box chemistry
    oceancsys mychemistry;      //<! box chemistry
	bool active_chemistry;      //<! box has active chemistry model?
	void chem_equilibrate( const unitval current_Ca );    //<! equilibrate chemistry model to a given flux
    double fmin( double alk, void *params );

    unitval atmosphere_flux;
    
	// logger
    Logger* logger;
};

}

#endif
