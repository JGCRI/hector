/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  onelineocean_component.cpp
 *  hector
 *
 *  Created by Ben on 7 March 2013.
 *
 */

#include "components/onelineocean_component.hpp"
#include "core/core.hpp"
#include "h_util.hpp"
#include "visitors/avisitor.hpp"

namespace Hector {
  
using namespace std;

//------------------------------------------------------------------------------
// documentation is inherited
OneLineOceanComponent::OneLineOceanComponent() {
}

//------------------------------------------------------------------------------
// documentation is inherited
OneLineOceanComponent::~OneLineOceanComponent() {
}

//------------------------------------------------------------------------------
// documentation is inherited
string OneLineOceanComponent::getComponentName() const {
    const string name = ONELINEOCEAN_COMPONENT_NAME;
    
    return name;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OneLineOceanComponent::init( Core* coreptr ) {
    logger.open( getComponentName(), false, Logger::DEBUG );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;
	
    core = coreptr;
    
    ocean_c.set( 38000.0, U_PGC );
    total_cflux.set( 0.0, U_PGC_YR );
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval OneLineOceanComponent::sendMessage( const std::string& message,
                                           const std::string& datum,
                                           const message_data info ) throw ( h_exception )
{
    unitval returnval;
    
    if( message==M_GETDATA ) {          //! Caller is requesting data
        return getData( datum, info.date );
        
    } else if( message==M_SETDATA ) {   //! Caller is requesting to set data
        H_THROW("OneLineOcean sendMessage not yet implemented for message=M_SETDATA.");
        //TODO: call setData below
        //TODO: change core so that parsing is routed through sendMessage
        //TODO: make setData private
        
    } else {                        //! We don't handle any other messages
        H_THROW( "Caller sent unknown message: "+message );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OneLineOceanComponent::setData( const string& varName,
                                     const message_data& data ) throw ( h_exception )
{
    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << data.date << "]=" << data.value_str << std::endl;
    
    try {
        if( varName == D_OCEAN_C ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            ocean_c = data.getUnitval(U_PGC);
        } else {
            H_THROW( "Unknown variable name while parsing " + getComponentName() + ": "
                    + varName );
        }
    } catch( h_exception& parseException ) {
        H_RETHROW( parseException, "Could not parse var: "+varName );
    }
}

//------------------------------------------------------------------------------
// documentation is inherited
void OneLineOceanComponent::prepareToRun() throw ( h_exception ) {
    
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OneLineOceanComponent::run( const double runToDate ) throw ( h_exception ) {
    
    double atmos_c = core->sendMessage( M_GETDATA, D_ATMOSPHERIC_C ).value( U_PGC );
    unitval atmos_co2 = core->sendMessage( M_GETDATA, D_ATMOSPHERIC_CO2 );
    
    // This equation gives ~0 flux at atmosphere 588, ocean 38000, and ~2.8 at current day
    total_cflux.set( -( ( ocean_c.value( U_PGC ) - 37412 ) - atmos_c  ) * 0.07, U_PGC_YR );
    
    ocean_c.set( ocean_c.value( U_PGC ) + total_cflux.value( U_PGC_YR ), U_PGC );
    
    H_LOG( logger, Logger::DEBUG ) << runToDate << " ocean_c=" << ocean_c << " total_cflux=" << total_cflux << " atmos_c=" << atmos_c << " atmos_co2=" << atmos_co2 << std::endl;
    
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval OneLineOceanComponent::getData( const std::string& varName,
                                       const double date ) throw ( h_exception ) {
    
    unitval returnval;
    
    H_ASSERT( date == Core::undefinedIndex(), "Date not available for onelineocean" );
    
    if( varName == D_OCEAN_CFLUX ) {
		returnval = total_cflux;
    } else if( varName == D_OCEAN_C ) {
        returnval = ocean_c;
    } else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OneLineOceanComponent::shutDown() {
	H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void OneLineOceanComponent::accept( AVisitor* visitor ) {
    visitor->visit( this );
}

}
