/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef N2O_COMPONENT_H
#define N2O_COMPONENT_H
/*
 *  n2o_component.hpp
 *  hector
 *
 *  Created by Ben on 05/19/2011.
 *
 */

#include "components/imodel_component.hpp"
#include "core/logger.hpp"
#include "data/tseries.hpp"
#include "data/unitval.hpp"

namespace Hector {
  
//------------------------------------------------------------------------------
/*! \brief Nitrous oxide model component.
 *
 *  This doesn't do much yet.
 */
class N2OComponent : public IModelComponent {
    
public:
    N2OComponent();
    ~N2OComponent();
    
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
    
    virtual void reset(double time) throw(h_exception);

    virtual void shutDown();
    
    //! IVisitable methods
    virtual void accept( AVisitor* visitor );
    
private:
    virtual unitval getData( const std::string& varName,
                            const double date ) throw ( h_exception );
    
    unitval N0;    //! preindustrial N2O
    unitval UC_N2O;  //! conversion from emissions to concentration
    tseries<unitval> N2O_emissions; //! anthropogenic emissions time series
    tseries<unitval> N2ON_emissions; //! natural emissions time series
    tseries<unitval> N2O; //! N2O concentrations
    tseries<unitval> TAU_N2O;   //! N2O decay time constant (varies as a function of concentration).
    unitval TN2O0;  //! inital N2O lifetime (tau)
    
    //! logger
    Logger logger;

	Core *core;
    double oldDate;
};

}

#endif // N2O_COMPONENT_H
