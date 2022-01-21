/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef NH3_COMPONENT_H
#define NH3_COMPONENT_H
/*
 *  nh3_component.hpp
 *  hector
 *
 *  Created by Kalyn D 2021.
 *
 */

#include "imodel_component.hpp"
#include "logger.hpp"
#include "tseries.hpp"
#include "unitval.hpp"

namespace Hector {

//------------------------------------------------------------------------------
/*! \brief NH3 model component.
 *
 *  This doesn't do much yet.
 */
class NH3Component : public IModelComponent {

public:
    NH3Component();
    ~NH3Component();

    // IModelComponent methods
    virtual std::string getComponentName() const;

    virtual void init( Core* core );

    virtual unitval sendMessage( const std::string& message,
                                const std::string& datum,
                                const message_data info=message_data() );

    virtual void setData( const std::string& varName,
                          const message_data& data );

    virtual void prepareToRun();

    virtual void run( const double runToDate );

    virtual void reset(double time);

    virtual void shutDown();

    // IVisitable methods
    virtual void accept( AVisitor* visitor );

private:
    virtual unitval getData( const std::string& varName,
                            const double date );

    //! Emissions time series
    tseries<unitval> NH3_emissions;

    //! logger
    Logger logger;

	Core *core;
    double oldDate;
};

}

#endif // NH3_COMPONENT_H
