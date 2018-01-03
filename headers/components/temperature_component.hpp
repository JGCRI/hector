/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

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

#include "components/forcing_component.hpp"
#include "components/imodel_component.hpp"
#include "core/logger.hpp"
#include "data/tseries.hpp"
#include "data/unitval.hpp"


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
 *  Kriegler, E. (2005) Imprecise probability analysis for Integrated Assessment of climate change. Ph.D. dissertation. Potsdam Universität. 256 pp. (http://opus.kobv.de/ubp/volltexte/2005/561/; DOECLIM introduced in Chapter 2 and Annexes A and B)
 *  Tanaka, K. & Kriegler, E. (2007) Aggregated carbon cycle, atmospheric chemistry, and climate model (ACC2) – Description of the forward and inverse modes – . Reports Earth Syst. Sci. 199.
 *  Garner, G., Reed, P. & Keller, K. (2016) Climate risk management requires explicit representation of societal trade-offs. Clim. Change 134, 713–723.
 */
class TemperatureComponent : public IModelComponent {
    
public:
    TemperatureComponent();
    ~TemperatureComponent();
    
    //! IModelComponent methods
    virtual std::string getComponentName() const;
    
    virtual void init( Core* core );
    
    virtual unitval sendMessage( const std::string& message,
                                const std::string& datum,
                                const message_data info=message_data() ) throw ( h_exception );
    
    virtual void setData( const std::string& varName,
                          const message_data& data ) throw ( h_exception );
    
    virtual void prepareToRun() throw ( h_exception );
    
    virtual void run( const double runToDate ) throw ( h_exception );
    
    virtual void shutDown();
    
    //! IVisitable methods
    virtual void accept( AVisitor* visitor );
    
private:
    virtual unitval getData( const std::string& varName,
                            const double date ) throw ( h_exception );
    void invert_1d_2x2_matrix( double * x, double * y);
    
    // Hard-coded DOECLIM parameters
    int dt;                  // years per timestep (this is hard-coded into Hector)
    int ns;                  // number of timesteps
    double ak;               // slope in climate feedback - land-sea heat exchange linear relationship
    double bk;               // offset in climate feedback - land-sea heat exchange linear relationship, W/m2/K
    double csw;              // specific heat capacity of seawater W*yr/m3/K
    double earth_area;       // m2
    double kcon;             // conversion from cm2/s to m2/yr
    double q2co;             // radiative forcing for atmospheric CO2 doubling, W/m2
    double rlam;             // factor between land clim. sens. and sea surface clim. sens. T_L2x = rlam*T_S2x
    double secs_per_Year;
    double zbot;             // bottom depth of diffusive ocean, m
    double bsi;              // warming factor for marine surface air over SST (due to retreating sea ice)
    double cal;              // heat capacity of land-troposphere system, W*yr/m2/K
    double cas;              // heat capacity of mixed layer-troposphere system, W*yr/m2/K
    double flnd;             // fractional land area
    double fso;              // ocean fractional area below 60m
    
    // Intermediate DOECLIM parameters
    double ocean_area;       // m2
    double cnum;             // factor from sea-surface climate sensitivity to global mean
    double cden;             // intermediate parameter
    double cfl;              // land climate feedback parameter, W/m2/K
    double cfs;              // sea climate feedback parameter, W/m2/K
    double kls;              // land-sea heat exchange coefficient, W/m2/K
    double keff;             // ocean heat diffusivity, m2/yr
    double taubot;           // ocean bottom diffusion time scale, yr
    double powtoheat;        // convert flux to total ocean heat 1E22 m2*s
    double taucfs;           // sea climate feedback time scale, yr
    double taucfl;           // land climate feedback time scale, yr
    double taudif;           // interior ocean heat uptake time scale, yr
    double tauksl;           // sea-land heat exchange time scale, yr
    double taukls;           // land-sea heat exchange time scale, yr

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


	std::vector<double> KT0;
	std::vector<double> KTA1;
	std::vector<double> KTB1;
	std::vector<double> KTA2;
	std::vector<double> KTB2;
	std::vector<double> KTA3;
	std::vector<double> KTB3;
	    
    unitval tgav;          //!< global temperature delta, deg C
    unitval tgaveq;        //!< equilibrium temp without ocean heat flux, currently set = tgav
    unitval S;             //!< climate sensitivity for 2xCO2, deg C
    unitval diff;          //!< ocean heat diffusivity, cm2/s
    unitval alpha;	       //!< aerosol forcing factor, unitless
    unitval flux_mixed;    //!< heat flux into mixed layer of ocean, W/m2
    unitval flux_interior; //!< heat flux into interior layer of ocean, W/m2
    unitval heatflux;      //!< heat flux into ocean, W/m2
	    
    tseries<unitval> tgav_constrain;        //! Temperature change can be supplied (not currently)
    
    //! pointers to other components and stuff
    Core*             core;
    
    //! logger
    Logger logger;

    //! persistent working space
    double internal_Ftot;       // W/m2
    double last_Ftot;           // W/m2
};

}

#endif // TEMPERATURE_COMPONENT_H
