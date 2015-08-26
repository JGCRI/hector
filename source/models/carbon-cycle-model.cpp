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
/*
 *  carbon-cycle-model.cpp
 *  hector
 *
 *  Created by Robert on March 2011.
 *
 */

#include "models/carbon-cycle-model.hpp"

namespace Hector {
  
//------------------------------------------------------------------------------
// documentation is inherited
void CarbonCycleModel::init( Core* core ) {
    logger.open( getComponentName(), false, Logger::DEBUG);
    H_LOG(logger, Logger::DEBUG) << getComponentName() << " initialized." << std::endl;
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval CarbonCycleModel::sendMessage( const std::string& message,
                         const std::string& datum,
                         const message_data info ) throw ( h_exception )
{
    H_THROW( "Should not be here yet!" );
    return unitval();
}

//------------------------------------------------------------------------------
// documentation is inherited
//! \remark The Carbon Cycle Model base class has no settable data.
void CarbonCycleModel::setData( const std::string& varName,
                                const message_data& data ) throw (h_exception)
{
    H_THROW( "Unknown variable name while parsing " + getComponentName() + ": "
            + varName );
}

//------------------------------------------------------------------------------
// documentation is inherited
void CarbonCycleModel::prepareToRun() throw(h_exception) {
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
}

//------------------------------------------------------------------------------
// documentation is inherited
void CarbonCycleModel::shutDown() {
	H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}

}
