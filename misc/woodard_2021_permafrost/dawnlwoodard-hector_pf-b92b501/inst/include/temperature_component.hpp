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
    
    virtual void reset(double date) throw(h_exception);

    virtual void shutDown();
    
    //! IVisitable methods
    virtual void accept( AVisitor* visitor );
    
private:
    virtual unitval getData( const std::string& varName,
                            const double date ) throw ( h_exception );
    void invert_1d_2x2_matrix( double * x, double * y);
    void setoutputs(int tstep);

    // Hard-coded DOECLIM parameters
    const int dt = 1;                     // years per timestep (this is implicit in Hector)
    int ns;                               // number of timesteps
    const double ak = 0.31;               // slope in climate feedback - land-sea heat exchange linear relationship
    const double bk = 1.59;               // offset in climate feedback - land-sea heat exchange linear relationship, W/m2/K
    const double csw = 0.13;              // specific heat capacity of seawater W*yr/m3/K
    const double earth_area = 5100656E8;  // m2
    const double kcon = 3155.0;           // conversion from cm2/s to m2/yr
    const double q2co = 3.7;              // radiative forcing for atmospheric CO2 doubling, W/m2
    const double rlam = 1.43;             // factor between land clim. sens. and sea surface clim. sens. T_L2x = rlam*T_S2x
    const double secs_per_Year = 31556926.0;
    const double zbot = 4000.0;           // bottom depth of diffusive ocean, m
    const double bsi = 1.3;               // warming factor for marine surface air over SST (due to retreating sea ice)
    const double cal = 0.52;               // heat capacity of land-troposphere system, W*yr/m2/K
    const double cas = 7.80;              // heat capacity of mixed layer-troposphere system, W*yr/m2/K
    const double flnd = 0.29;             // fractional land area
    const double fso = 0.95;              // ocean fractional area below 60m
    
    // DOECLIM parameters calculated from constants above
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
    unitval S;             //!< climate sensitivity for 2xCO2, deg C
    unitval diff;          //!< ocean heat diffusivity, cm2/s
    unitval alpha;	       //!< aerosol forcing factor, unitless
    unitval volscl;        //!< volcanic forcing scaling factor, unitless

    // Model outputs
    unitval tgav;          //!< global average surface air temperature anomaly, deg C
    unitval tgav_land;     //!< global average land surface air temperature anomaly, deg C
    unitval tgav_oceanair; //!< global average ocean surface air temperature anomaly, deg C
    unitval tgav_sst;      //!< global average ocean surface (water) temperature anomaly, deg C
    unitval tgaveq;        //!< equilibrium temp without ocean heat flux, currently set = tgav
    unitval flux_mixed;    //!< heat flux into mixed layer of ocean, W/m2
    unitval flux_interior; //!< heat flux into interior layer of ocean, W/m2
    unitval heatflux;      //!< heat flux into ocean, W/m2
	    
    tseries<unitval> tgav_constrain;        //! Temperature change can be supplied (not currently)
    
    //! pointers to other components and stuff
    Core*             core;
    
    //! logger
    Logger logger;
};

}

#endif // TEMPERATURE_COMPONENT_H
