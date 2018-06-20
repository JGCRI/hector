/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
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
    logger.open( getComponentName(), false, Logger::getGlobalLogger().getEchoToFile(), Logger::getGlobalLogger().getMinLogLevel() );
    H_LOG(logger, Logger::DEBUG) << getComponentName() << " initialized." << std::endl;
}


}
