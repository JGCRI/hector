/*
 *  temp_component.cpp
 *  hector
 *
 *  Created by Ben on 05/25/2011.
 *
 */

#include "components/temperature_component.hpp"
#include "core/core.hpp"
#include "core/dependency_finder.hpp"
#include "h_util.hpp"
#include "visitors/avisitor.hpp"

namespace Hector {
  
using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 */
TemperatureComponent::TemperatureComponent() {
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
TemperatureComponent::~TemperatureComponent() {
}

//------------------------------------------------------------------------------
// documentation is inherited
string TemperatureComponent::getComponentName() const {
    const string name = TEMPERATURE_COMPONENT_NAME;
    
    return name;
}

//------------------------------------------------------------------------------
// documentation is inherited
void TemperatureComponent::init( Core* coreptr ) {
    logger.open( getComponentName(), false, Logger::DEBUG );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;
    
	tgaveq.set( 0.0, U_DEGC, 0.0 );
    tgav.set( 0.0, U_DEGC, 0.0 );
    core = coreptr;
    
    tgav_constrain.allowInterp( true );
    tgav_constrain.name = D_TGAV_CONSTRAIN;
    
    // Register the data we can provide
    core->registerCapability( D_GLOBAL_TEMP, getComponentName() );
    core->registerCapability( D_GLOBAL_TEMPEQ, getComponentName() );
    
    // Register our dependencies
    core->registerDependency( D_RF_TOTAL, getComponentName() );
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval TemperatureComponent::sendMessage( const std::string& message,
                                          const std::string& datum,
                                          const message_data info ) throw ( h_exception )
{
    unitval returnval;
    
    if( message==M_GETDATA ) {          //! Caller is requesting data
        return getData( datum, info.date );
        
    } else if( message==M_SETDATA ) {   //! Caller is requesting to set data
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
void TemperatureComponent::setData( const string& varName,
                                    const message_data& data ) throw ( h_exception )
{
    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << data.date << "]=" << data.value_str << std::endl;
    
    try {
        if( varName == D_ECS ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            S = unitval::parse_unitval( data.value_str, data.units_str, U_DEGC );
        } else if( varName == D_TGAV_CONSTRAIN ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            tgav_constrain.set( data.date, unitval::parse_unitval( data.value_str, data.units_str, U_DEGC ) );
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
void TemperatureComponent::prepareToRun() throw ( h_exception ) {
    
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
    
    if( tgav_constrain.size() ) {
        Logger& glog = Logger::getGlobalLogger();
        H_LOG( glog, Logger::WARNING ) << "Temperature will be overwritten by user-supplied values!" << std::endl;
    }
}

//------------------------------------------------------------------------------
// documentation is inherited
void TemperatureComponent::run( const double runToDate ) throw ( h_exception ) {
    
    // S is the equilibrium climate sensitivity for a doubling of CO2
    // this corresponds, according to Knutti and Hegerl (2008), to ~3.7 W/m2
    // So use this to compute a temperature resulting from total forcing
    unitval Ftot = core->sendMessage( M_GETDATA, D_RF_TOTAL );
	tgaveq.set( (S.value( U_DEGC ) / 3.7 ) * Ftot.value( U_W_M2 ), U_DEGC );
	unitval heatflux = core->sendMessage( M_GETDATA, D_HEAT_FLUX );
	tgav.set( ( S.value( U_DEGC ) / 3.7 ) * ( Ftot.value( U_W_M2 ) - heatflux.value( U_W_M2) ), U_DEGC );
    
    // If the user has supplied temperature data, use that (except if past its end)
    if( tgav_constrain.size() && runToDate < tgav_constrain.lastdate() ) {
        H_LOG( logger, Logger::WARNING ) << "** Overwriting temperature with user-supplied value" << std::endl;
        tgav = tgav_constrain.get( runToDate );
    }
    
	H_LOG( logger, Logger::DEBUG ) << "temperatureeq " << tgaveq << "Date"<< runToDate << std::endl;
	H_LOG( logger, Logger::DEBUG ) << "temperature " << tgav << "Date" << runToDate << std::endl;
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval TemperatureComponent::getData( const std::string& varName,
                                      const double date ) throw ( h_exception ) {
    unitval returnval;
    
    H_ASSERT( date == Core::undefinedIndex(), "Only current temperatures provided" );
    
    if( varName == D_GLOBAL_TEMP ) {
        returnval = tgav;
    } else if( varName == D_GLOBAL_TEMPEQ ) {
        returnval = tgaveq;
    } else {
        H_THROW( "Caller requested unknown datum: " + varName );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void TemperatureComponent::shutDown() {
	H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void TemperatureComponent::accept( AVisitor* visitor ) {
    visitor->visit( this );
}

}
