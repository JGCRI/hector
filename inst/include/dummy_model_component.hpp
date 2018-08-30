/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef DUMMY_MODEL_COMPONENT_H
#define DUMMY_MODEL_COMPONENT_H
/*
 *  dummy_model_component.h
 *  hector
 *
 *  Created by Pralit Patel on 10/29/10.
 *
 */

#include "imodel_component.hpp"
#include "logger.hpp"
#include "tseries.hpp"
#include "unitval.hpp"

namespace Hector {
  
//------------------------------------------------------------------------------
/*! \brief A Dummy model component.
 *
 *  A dummy model for testing.  No science...unless you consider debugging a
 *  science.
 */
class DummyModelComponent : public IModelComponent {
    
public:
    DummyModelComponent();
    ~DummyModelComponent();
    
    double getY() const;
    
    const tseries<double>& getC() const;
    
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
                            const double valueIndex ) throw ( h_exception );
    
    //! input var
    double slope;
    
    //! state var
    double prevX;
    
    //! result at state
    double y;
    
    //! input time series var
    tseries<double> c;
    
    // logger
    Logger logger;
};

}

#endif // DUMMY_MODEL_COMPONENT_H
