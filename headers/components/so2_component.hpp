/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef SULFUR_COMPONENT_H
#define SULFUR_COMPONENT_H
/*
 *  so2_component.hpp
 *  hector
 *
 *  Created by Corinne on 5/6/2013
 *
 */

#include "components/imodel_component.hpp"
#include "core/logger.hpp"
#include "data/tseries.hpp"
#include "data/unitval.hpp"


namespace Hector {
//------------------------------------------------------------------------------
/*! \brief Sulfur model component.
 *
 *  This doesn't do much yet.
 */
class SulfurComponent : public IModelComponent {
        
public:
    SulfurComponent();
    ~SulfurComponent();
    
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
    
    
	unitval S0;    // historical value of sulfur  (YEAR?)
	unitval SN;
	
private:
    virtual unitval getData( const std::string& varName,
                            const double date ) throw ( h_exception );
    
    //! Emissions time series
	
    tseries<unitval> SO2_emissions;
	tseries<unitval> SV;
    
    //! logger
    Logger logger;

	Core *core;
    double oldDate;
};

}

#endif  // SULFUR_COMPONENT_H
