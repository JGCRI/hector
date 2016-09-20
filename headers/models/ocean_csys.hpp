/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef OCEAN_CSYS_CLASS_H
#define OCEAN_CSYS_CLASS_H
/*
 *  ocean_csys_class.hpp
 *
 *  Created by Corinne Hartin on 1/30/13.
 *
 */

#include <vector>
#include <string>

#include "data/unitval.hpp"

namespace Hector {
  
class oceancsys
{
    /*! /brief  Ocean Carbon Chemistry
     *
     *  Implements ocean carbon chemistry in surface boxes. 
     *  Based on the book by Zeebe and Wolf-Gladrow (2001) CO2 in seawater: equilibrium, kintetics, isotopes. 346 p Amsterdam: Elsevier
     *  http://www.soest.hawaii.edu/oceanography/faculty/zeebe_files/CO2_System_in_Seawater/csys.html
     *  Inputs: ALK, DIC, Temp
     *  Outputs: pH, Ca/Ar saturations, pCO2 (ocean), surface flux of carbon
     */
	
public:
	oceancsys();

	//ocean component will have to provide these values
	double S;       //<! salinity
	double As;      //<! area of box m2
	double Ks;      //<!gas transfer velocity m/yr 
	double volumeofbox;
	unitval OmegaCa;
	unitval OmegaAr;
	double U;       //<! average wind speed over each surface box
	double H;
	    
	//<! output variables
	unitval TCO2o;  //<! total CO2 (umol/kg)
	unitval HCO3;   //<! bicarbonate (umol/kg)
	unitval CO3;    //<! carbonate (umol/kg)
	unitval PCO2o;  //<! pCO2 of ocean waters
	unitval pH;     //<! ocean pH
    
	unitval convertToDIC( const unitval carbon );
	void ocean_csys_run( unitval tbox, unitval carbon );
    unitval calc_annual_surface_flux( const unitval& Ca, const double cpoolscale=1.0 ) const;
    unitval get_K0() const { return K0; };
    unitval get_Tr() const { return Tr; };

    void set_alk( double a ) { alk=a; };
    double get_alk() const { return alk; };
    
private:
    double calc_monthly_surface_flux( const unitval& Ca, const double cpoolscale=1.0 ) const;

	unitval K0;     //<! solubility of CO2 calculated from Weiss 1974 (mol * L-1 * atm-1)
	unitval Tr;     //<! gas transfer coefficient (gC m-2 month-1 uatm-1)
	unitval Kh;     //<! solubility of CO2 calculated from Weiss 1974 (mol*kg-1*atm-1)
	unitval Kw;     //<! equilirbium relationship of H+ and OH- (mol kg-1)
	unitval K1;     //<! equilibrium relationship of CO2 in seawater (mol kg-1)
	unitval K2;     //<! equilibrium relationship of CO2 in seawater (mol kg-1)
	unitval Kb;     //<! equilibrium relationship of boron in seawater (mol kg-1)
	unitval Sc;     //<! Schmidt Number from Wanninkhof 1992 (unitless)
	unitval Kspa;   //<! equilibrium relationship of aragonite in seawater (mol kg-1)
    unitval Kspc;   //<! equilibrium relationship of calcite in seawater (mol kg-1)
    
    double alk;     //<! alkilinity (umol/kg)

    // logger
    Logger* logger;

    // persistent workspace
    const int ncoeffs;
    std::vector<double> m_a; 

};

}

#endif
  
