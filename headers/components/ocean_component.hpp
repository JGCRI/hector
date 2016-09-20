/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

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

#include "core/logger.hpp"
#include "data/tseries.hpp"
#include "data/unitval.hpp"
#include "models/carbon-cycle-model.hpp"
#include "models/ocean_csys.hpp"
#include "models/oceanbox.hpp"

#define OCEAN_MAX_TIMESTEP      1.0     //!< max/default timestep (yr)
#define OCEAN_MIN_TIMESTEP      0.3     //!< minimum timestep (yr)
#define OCEAN_TSR_FACTOR        0.5     //!< timestep reduction factor when necessary
#define OCEAN_TSR_TIMEOUT       20      //!< years we lock into reduced timestep
#define OCEAN_TSR_TRIGGER1      0.1     //!< trigger1 to reduce timestep:
                                        //!< absolute diff between successive annual fluxes (Pg C)

namespace Hector {
  
//------------------------------------------------------------------------------
/*! \brief Ocean model component.
 *
 *  A simple yet classy ocean model that handles surface carbon chemistry, flows
 *  between layers and latitudinal regions, etc.
 */
class OceanComponent : public CarbonCycleModel {
    
public:
    OceanComponent();
    ~OceanComponent();
    
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
    
    virtual bool run_spinup( const int step ) throw ( h_exception );
    
    virtual void shutDown();
    
    //! IVisitable methods
    virtual void accept( AVisitor* visitor );

    // Carbon cycle model interface
    void getCValues( double t, double c[]);
    int  calcderivs( double t, const double c[], double dcdt[] ) const;
    void slowparameval( double t, const double c[] );
    void stashCValues( double t, const double c[] );

    void run1( const double runToDate ) throw ( h_exception );

private:
    virtual unitval getData( const std::string& varName,
                            const double date ) throw ( h_exception );
    void calcHeatflux( const double runToDate );
    
    // Ocean boxes
    oceanbox surfaceHL; //!< surface high latitude box 100m
	oceanbox surfaceLL; //!< surface low latitude box 100m
	oceanbox inter;     //!< intermediate box 1000m
	oceanbox deep;      //!< deep box 3000m

    // Ocean conditions
    unitval Tgav;           //!< Global temperature anomaly, degC
    unitval Ca;             //!< Atmospheric CO2, ppm
    bool in_spinup;         //!< Are we currently in spinup?
    bool spinup_chem;       //!< run chemistry during spinup?
    tseries<unitval> oceanflux_constrain;      //!< atmosphere->ocean C flux data to constrain to
    
    // Heat flux
	unitval heatflux;
	unitval k_max;           //!< maximum ocean heat uptake efficiency, W/m2/K
    unitval k_min;          //!< minimum ocean heat uptake efficiency, W/m2/K
    unitval slope;           //!< slope of the curve, 1/K
//	unitval cp;             //!< Specific heat
    unitval t_mid;          //!< global temperature at which kappa has declined by 50%
    unitval kappa;              //!< ocean heat uptake efficiency, W/m2/K
    
    // Ocean circulation
    unitval tt;          //!< m3/s thermohaline overturning
    unitval tu;          //!< m3/s high latitude overturning
    unitval twi;         //!< m3/s warm-intermediate exchange
    unitval tid;         //!< m3/s intermediate-deep exchange
   
    // Carbon values
    unitval carbon_HL;
    unitval carbon_LL;
    unitval carbon_IO;
    unitval carbon_DO;

    // Private helper functions
    unitval totalcpool() const;
    unitval annual_totalcflux( const double date, const unitval& Ca, const double cpoolscale=1.0 ) const;
    unitval annualflux_sum, annualflux_sumHL, annualflux_sumLL;     //!< Running annual totals atm-ocean flux, for output reporting
    unitval lastflux_annualized;        //!< Last atm-ocean flux when solver ordered us to 'stash' C values

    double max_timestep;                //!< Current maximum timestep allowed. This can change
    int reduced_timestep_timeout;       //!< Timer that keeps track of how long we've had a reduced timestep
    int timesteps;                      //!< Number of timesteps taken in current year
    double m_min_k_so_far;              //!< Smallest kappa value seen in this run.
    
    //! logger
    Logger logger;
};

}

#endif // OCEAN_COMPONENT_H
