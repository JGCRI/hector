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
    
    virtual void shutDown();
    
    //! IVisitable methods
    virtual void accept( AVisitor* visitor );
    
private:
    virtual unitval getData( const std::string& varName,
                            const double date ) throw ( h_exception );
    
	unitval total_cflux;        //!< Ocean-to-atmosphere flux, PgC/yr
    unitval ocean_c;            //!< Total C in the ocean, PgC
    
    //! pointers to other components and stuff
    Core* core;
    
    //! logger
    Logger logger;
};

}

#endif // ONELINEOCEAN_COMPONENT_H
