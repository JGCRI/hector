/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef BLACK_CARBON_COMPONENT_H
#define BLACK_CARBON_COMPONENT_H
/*
 *  bc_component.hpp
 *  hector
 *
 *  Created by Ben on 05/26/2011.
 *
 */

#include "components/imodel_component.hpp"
#include "core/logger.hpp"
#include "data/tseries.hpp"
#include "data/unitval.hpp"

namespace Hector {
  
//------------------------------------------------------------------------------
/*! \brief Black carbon model component.
 *
 *  This doesn't do much yet.
 */
class BlackCarbonComponent : public IModelComponent {
    
public:
    BlackCarbonComponent();
    ~BlackCarbonComponent();
    
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

    virtual void reset(double time) throw(h_exception);
    
    virtual void shutDown();
    
    // IVisitable methods
    virtual void accept( AVisitor* visitor );
    
private:
    virtual unitval getData( const std::string& varName,
                            const double date ) throw ( h_exception );
    
    //! Emissions time series
    tseries<unitval> BC_emissions;
    
    //! logger
    Logger logger;
    
    Core *core;
    double oldDate;
};

}

#endif // BLACK_CARBON_COMPONENT_H
