/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
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
TemperatureComponent::TemperatureComponent() : internal_Ftot(0.0), last_Ftot(0.0) {
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
TemperatureComponent::~TemperatureComponent() {
}

//------------------------------------------------------------------------------
// documentation is inherited
string TemperatureComponent::getComponentName() const 
{
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
    
        FCO2_record.allowInterp( true );
        
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
        H_THROW("Temperature: sendMessage not yet implemented for message=M_SETDATA.");
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
            S = data.getUnitval( U_DEGC );
        } else if( varName == D_TGAV_CONSTRAIN ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            tgav_constrain.set( data.date, data.getUnitval( U_DEGC ) );
            } else if( varName == D_SO2I_B ) {
                H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
                so2i_b = data.getUnitval( U_UNITLESS );
           } else if( varName == D_SO2D_B ) {
                H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
                so2d_b = data.getUnitval( U_UNITLESS );
            } else if( varName == D_OC_B ) {
                H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
                oc_b = data.getUnitval( U_UNITLESS );
        } else if( varName == D_BC_B ) {
                H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
                bc_b = data.getUnitval( U_UNITLESS );
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
    
        // We track total radiative forcing using internal variable `internal_Ftot`
        // Need to do this because if we're subject to a user constraint (being forced
        // to match a temperature record), need to track the Ftot that *would* have
        // produced the observed temperature record. This way there's a smooth
        // transition when we exit the constraint period, after which internal_Ftot
        // will rise in parallel with the value reported by ForcingComponent.
    
        // If we never had any temperature constraint, `internal_Ftot` will match `Ftot`.
        
        // If the user has supplied temperature data, use that (except if past its end)
        if( tgav_constrain.size() && runToDate <= tgav_constrain.lastdate() ) {
            H_LOG( logger, Logger::NOTICE ) << "** Using user-supplied temperature" << std::endl;
        tgav = tgav_constrain.get( runToDate );
            tgaveq = tgav;

            // Update last_Ftot and internal_Ftot for when we exit constraint
            last_Ftot = core->sendMessage( M_GETDATA, D_RF_TOTAL ).value( U_W_M2 );
            const double heatflux = core->sendMessage( M_GETDATA, D_HEAT_FLUX ).value( U_W_M2 );
            internal_Ftot = 3.7 * tgav / S + heatflux;  // inverting normal T=S/3.7*F eqn
        } else {  // not constrained
            // Not subject to a constraint. Get total forcing and update our
            // internal tracking variables `internal_Ftot` (parallels Ftot, may
            // be identical if never had a constraint) and `last_Ftot`.
          double Ftot = core->sendMessage( M_GETDATA, D_RF_TOTAL ).value( U_W_M2 );
            
            // A few forcing agents occur disproportionately over land, and thus exert
            // a stronger effect than their global mean RF would suggest. In the future,
            // we want to compute RF separately for land and ocean; for now, add a 'bonus'
            // for these forcing agents.
            
            // NOTE: this is commented out and not used for now.
            /*
            const double fbc = core->sendMessage( M_GETDATA, D_RF_BC ).value( U_W_M2 );
            Ftot += fbc * bc_b; // % extra
            const double foc = core->sendMessage( M_GETDATA, D_RF_OC ).value( U_W_M2 );
            Ftot += foc * oc_b; // % extra
            const double fso2d = core->sendMessage( M_GETDATA, D_RF_SO2d ).value( U_W_M2 );
            Ftot += fso2d * so2d_b; // % extra
            const double fso2i = core->sendMessage( M_GETDATA, D_RF_SO2i ).value( U_W_M2 );
            Ftot += fso2i * so2i_b; // % extra
            */
            
            // CO2 is subject to a lag effect, coded empirically here because a simple model
            // like Hector doesn't account for all buffers and processes in the earth system
            // We use a window (size CO2_WINDOW) CO2_LAG years in the past
            const double FCO2_current = core->sendMessage( M_GETDATA, D_RF_CO2 ).value( U_W_M2 );
            FCO2_record.set( runToDate, FCO2_current );

            #define CO2_LAG 20
            #define CO2_WINDOW 10
            double FCO2_lagged_mean = 0.0;       /* window mean of FCO2_past */
            if( runToDate > core->getStartDate() + CO2_LAG ) {
                for( int i=runToDate-CO2_LAG-CO2_WINDOW; i<runToDate-CO2_LAG; i++ ) {
                    FCO2_lagged_mean += FCO2_record.get( i );
                }
                FCO2_lagged_mean /= CO2_WINDOW;
    }
    
            // Subtract off the current FCO2 computed by ForcingComponent, and use the lagged
            // window mean computed above instead
            Ftot = Ftot - FCO2_current + FCO2_lagged_mean;

            // At this point, adjust out internal Ftot tracking variable by the delta between
            // Ftot and last_Ftot
            internal_Ftot += ( Ftot - last_Ftot );
            last_Ftot = Ftot;

            // S is the equilibrium climate sensitivity for a doubling of CO2
            // this corresponds, according to Knutti and Hegerl (2008), to ~3.7 W/m2
            // Use this to compute a temperature resulting from total forcing
            tgaveq = S / 3.7 * internal_Ftot;
            const double heatflux = core->sendMessage( M_GETDATA, D_HEAT_FLUX ).value( U_W_M2 );
            tgav = S / 3.7 * ( internal_Ftot - heatflux );
        }
        
        H_LOG( logger, Logger::DEBUG ) << " internal_Ftot=" << internal_Ftot << " tgav=" << tgav << " in " << runToDate << std::endl;
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
