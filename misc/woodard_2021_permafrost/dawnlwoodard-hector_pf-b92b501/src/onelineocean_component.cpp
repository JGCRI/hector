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

#include "onelineocean_component.hpp"
#include "core.hpp"
#include "h_util.hpp"
#include "avisitor.hpp"

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
    logger.open( getComponentName(), false, coreptr->getGlobalLogger().getEchoToFile(), coreptr->getGlobalLogger().getMinLogLevel() );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;
	
    core = coreptr;
    
    ocean_c.set(core->getStartDate(), unitval(38000.0, U_PGC));
    total_cflux.set(core->getStartDate(), unitval(0.0, U_PGC_YR));
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
            ocean_c.set(oldDate, data.getUnitval(U_PGC));
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

    oldDate = core->getStartDate();
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OneLineOceanComponent::run( const double runToDate ) throw ( h_exception ) {
    
    double atmos_c = core->sendMessage( M_GETDATA, D_ATMOSPHERIC_C ).value( U_PGC );
    unitval atmos_co2 = core->sendMessage( M_GETDATA, D_ATMOSPHERIC_CO2 );
    
    // This equation gives ~0 flux at atmosphere 588, ocean 38000, and ~2.8 at current day
    total_cflux.set(runToDate,  unitval(-( ( ocean_c.get(runToDate).value( U_PGC ) - 37412 ) - atmos_c  ) * 0.07, U_PGC_YR) );
    
    ocean_c.set(runToDate, unitval(ocean_c.get(runToDate).value( U_PGC ) + total_cflux.get(runToDate).value( U_PGC_YR ), U_PGC) );

    oldDate = runToDate;
    H_LOG( logger, Logger::DEBUG ) << runToDate
                                   << " ocean_c=" << ocean_c.get(runToDate)
                                   << " total_cflux=" << total_cflux.get(runToDate)
                                   << " atmos_c=" << atmos_c
                                   << " atmos_co2=" << atmos_co2
                                   << std::endl;
    
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval OneLineOceanComponent::getData( const std::string& varName,
                                       const double date ) throw ( h_exception ) {
    
    unitval returnval;
    
    double getdate = date;
    if(date == Core::undefinedIndex())
        getdate = oldDate;
    
    if( varName == D_OCEAN_CFLUX ) {
        returnval = total_cflux.get(getdate);
    } else if( varName == D_OCEAN_C ) {
        returnval = ocean_c.get(getdate);
    } else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }
    
    return returnval;
}


void OneLineOceanComponent::reset(double time) throw(h_exception)
{
    oldDate = time;
    H_LOG(logger, Logger::NOTICE)
        << getComponentName() << " reset to time= " << time << "\n";
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
