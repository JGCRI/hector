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

namespace Hector {
  
//------------------------------------------------------------------------------
/*! \brief Methane model component.
 *
 *  This doesn't do much yet.
 */
class CH4Component : public IModelComponent {
    
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
    
	private:
    virtual unitval getData( const std::string& varName,
                            const double date ) throw ( h_exception );   
     //! emissions time series
    tseries<unitval> CH4_emissions;
    tseries<unitval> CH4;  // CH4 concentrations
    unitval M0;     // preindustrial methane
    unitval UC_CH4;  // conversion factor between emissions and concentration
    unitval CH4N;   // natural emissions
    unitval Tsoil;  // CH4 loss to soil
    unitval Tstrat; //  CH4 loss to stratosphere

    // logger
    Logger logger;

	Core *core;
    double oldDate;
};

}

#endif // CH4_COMPONENT_H

