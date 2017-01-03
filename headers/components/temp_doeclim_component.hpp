/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef TEMP_DOECLIM_COMPONENT_H
#define TEMP_DOECLIM_COMPONENT_H
/*
 *  temp_doeclim_component.hpp
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
 *  using Diffusion Ocean Energy balance CLImate Model (DOECLIM), (Kriegler, 2005).
 *  Adopted from C++ github implementation, cdice_doeclim (Garner et al., 2013).
 */
class TempDOECLIMComponent : public IModelComponent {
    
public:
    TempDOECLIMComponent();
    ~TempDOECLIMComponent();
    
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
    void sum_1d_2x2_matrix( double * x, double * y, double * z);
    
    //Hard-coded DOECLIM parameters
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
    
    //Intermediate DOECLIM parameters
    double B[4];
    double C[4];

    std::vector<double> KT0;
    std::vector<double> KTA1;
    std::vector<double> KTB1;
    std::vector<double> KTA2;
    std::vector<double> KTB2;
    std::vector<double> KTA3;
    std::vector<double> KTB3;
    
    std::vector<double> Ker;

    double A[4];
    double IB[4];
    
    //Dependent DOECLIM parameters
    double ocean_area;
    double cnum;
    double cden;
    double cfl;
    double cfs;
    double kls;
    double keff;
    double taubot;
    double powtoheat;
    double taucfs;
    double taucfl;
    double taudif;
    double tauksl;
    double taukls;
    
    //Time series arrays that are updated w/ each DOECLIM run, again using hard-coded max ns.
    std::vector<double> temp;
    std::vector<double> temp_landair;
    std::vector<double> temp_sst;
    std::vector<double> heatflux_mixed;
    std::vector<double> heatflux_interior;
    std::vector<double> heat_mixed;
    std::vector<double> heat_interior;
    std::vector<double> forcing;
    
	unitval tgav;          //!< global temperature delta, deg C
    unitval tgaveq;        //!< equilibrium temp without ocean heat flux, currently set = tgav
	unitval S;             //!< climate sensitivity for 2xCO2, deg C
    unitval alpha;         //!< ocean heat diffusivity, cm2/s
    
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

#endif // TEMP_DOECLIM_COMPONENT_H
