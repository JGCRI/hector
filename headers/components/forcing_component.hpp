/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef FORCING_COMPONENT_H
#define FORCING_COMPONENT_H
/*
 *  forcing_component.hpp
 *  hector
 *
 *  Created by Ben on 02 March 2011.
 *
 */

#include "components/imodel_component.hpp"
#include "components/ch4_component.hpp"
#include "components/n2o_component.hpp"
#include "components/bc_component.hpp"
#include "components/oc_component.hpp"
#include "components/so2_component.hpp"
//#include "core/o3_component.hpp"
#include "core/logger.hpp"
#include "data/tseries.hpp"
#include "data/unitval.hpp"
#include "models/carbon-cycle-model.hpp"

namespace Hector {
  
// Need to forward declare the components which depend on each other
class SimpleNbox;
class HalocarbonComponent;


//------------------------------------------------------------------------------
/*! \brief The forcing component.
 *
 */
class ForcingComponent : public IModelComponent {
    
public:
	
    ForcingComponent();
    ~ForcingComponent();
    
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
    
    //! A list (map) of all computed forcings, with associated iterator
    std::map<std::string, unitval > forcings, baseyear_forcings;
    typedef std::map<std::string, unitval >::iterator forcingsIterator;
    
	
    
private:
    virtual unitval getData( const std::string& varName,
                            const double valueIndex ) throw ( h_exception );
    
    double baseyear;        //! Year which forcing calculations will start
    double currentYear;     //! Tracks current year
    unitval C0;             //! Records base year atmospheric CO2
    
    tseries<unitval> Ftot_constrain;       //! Total forcing can be supplied
    
    Core* core;             //! Core
    double oldDate;
    Logger logger;          //! Logger
};

}

#endif // FORCING_COMPONENT_H
