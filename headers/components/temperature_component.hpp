/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef TEMP_COMPONENT_H
#define TEMP_COMPONENT_H
/*
 *  temperature_component.hpp
 *  hector
 *
 *  Created by Ben on 05/25/2011.
 *
 */

#include "components/forcing_component.hpp"
#include "components/imodel_component.hpp"
#include "core/logger.hpp"
#include "data/unitval.hpp"


namespace Hector {
  
//------------------------------------------------------------------------------
/*! \brief Temperature model component.
 *
 *  A component that computes mean global temperature from radiative forcing
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
    
    unitval tgaveq;     // equilibrium temperature without ocean heat flux
	unitval tgav;       //!< global temperature delta, deg C
	unitval S;          //!< climate sensitivity for 2xCO2, deg C
    
    unitval bc_b;
    unitval oc_b;
    unitval so2i_b;
    unitval so2d_b;

    tseries<unitval> tgav_constrain;        //! Temperature change can be supplied
    
    tseries<double> FCO2_record;            //!< Record of CO2 forcing effect
    
    //! pointers to other components and stuff
    Core*             core;
    
    //! logger
    Logger logger;

    //! persistent working space
    double internal_Ftot;       // W/m2
    double last_Ftot;           // W/m2
};

}

#endif // TEMP_COMPONENT_H
