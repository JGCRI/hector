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
#ifndef HALOCARBON_COMPONENT_HPP
#define HALOCARBON_COMPONENT_HPP
/*
 *  halocarbon_component.hpp
 *  hector
 *
 *  Created by Pralit Patel on 1/20/11.
 *
 */

#include "core/logger.hpp"
#include "data/tseries.hpp"
#include "data/unitval.hpp"
#include "imodel_component.hpp"

namespace Hector {
  
//------------------------------------------------------------------------------
/*! \brief Model component for a halocarbon.
 *
 *  A halocarbon model component that simply decays in the atmosphere.  Adapted
 *  from Bill Emanuel's python implementation.
 *
 */
class HalocarbonComponent : public IModelComponent {
    friend class CSVOutputStreamVisitor;
    
public:
    HalocarbonComponent( std::string g );
    virtual ~HalocarbonComponent();
    
    
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
                            const double valueIndex ) throw ( h_exception );
    
    //! Who are we?
    std::string myGasName;
    
    //! Rate coefficient of loss [relates to year?]
    double tau;
    
    //! Radiative forcing efficiency [W/m^2/pptv]
    unitval rho;
    
    //! Forcing [W/m^2]
    tseries<unitval> hc_forcing;
    
    tseries<unitval> emissions;     //! Time series of emissions, pptv
    unitval Ha;                     //! Current (ambient) concentration, pptv
    unitval H0;                     //! Preindustrial concentration, pptv
    
    double molarMass;
    
    //! The last calculated date value
    double calcDate;
    
    //! logger
    Logger logger;

	Core *core;
    double oldDate;
};

}

#endif // HALOCARBON_COMPONENT_HPP
