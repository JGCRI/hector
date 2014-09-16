
#ifndef CH4_COMPONENT_H
#define CH4_COMPONENT_H
/*
 *  ch4_component.hpp
 *  hector
 *
 *  Created by Ben on 05/19/2011.
 *
 */

#include "components/imodel_component.hpp"
#include "core/logger.hpp"
#include "data/tseries.hpp"
#include "data/unitval.hpp"

//------------------------------------------------------------------------------
/*! \brief Methane model component.
 *
 *  This doesn't do much yet.
 */
class CH4Component : public IModelComponent {
    friend class INIRestartVisitor; // To allow creation of a restart file.
    
public:
    CH4Component();
    ~CH4Component();
    
    // IModelComponent methods
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
    
    // IVisitable methods
    virtual void accept( AVisitor* visitor );
    
	unitval M0;    // preindustrial methane
    
private:
    virtual unitval getData( const std::string& varName,
                            const double date ) throw ( h_exception );
    
    //! concentrations time series
    tseries<unitval> Ma;
    
    // logger
    Logger logger;

	Core *core;
    double oldDate;
};

#endif // CH4_COMPONENT_H

