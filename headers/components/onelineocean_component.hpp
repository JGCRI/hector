/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef ONELINEOCEAN_COMPONENT_H
#define ONELINEOCEAN_COMPONENT_H
/*
 *  onelineocean_component.hpp
 *  hector
 *
 *  Created by Ben on 7 March 2013.
 *
 */

#include "components/imodel_component.hpp"
#include "core/logger.hpp"
#include "data/unitval.hpp"
#include "data/tseries.hpp"

namespace Hector {
  
//------------------------------------------------------------------------------
/*! \brief A one-line ocean model component.
 *
 */

class OneLineOceanComponent : public IModelComponent {
    
public:
    OneLineOceanComponent();
    ~OneLineOceanComponent();
    
    
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
    
    tseries<unitval> total_cflux;        //!< Ocean-to-atmosphere flux, PgC/yr
    tseries<unitval> ocean_c;            //!< Total C in the ocean, PgC
    
    //! pointers to other components and stuff
    Core* core;
    
    //! logger
    Logger logger;

    double oldDate;

};

}

#endif // ONELINEOCEAN_COMPONENT_H
