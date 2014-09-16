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


//------------------------------------------------------------------------------
/*! \brief Temperature model component.
 *
 *  A component that computes mean global temperature from radiative forcing
 */
class TemperatureComponent : public IModelComponent {
    friend class INIRestartVisitor; //!< To allow creation of a restart file.
    
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
    
    tseries<unitval> tgav_constrain;       //! Temperature change can be supplied
    
    //! pointers to other components and stuff
    Core*             core;
    
    //! logger
    Logger logger;
};

#endif // TEMP_COMPONENT_H
