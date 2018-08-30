/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  slr_component.cpp
 *  hector
 *
 *  Created by Ben on 31 January 2012.
 *
 */

#include "components/slr_component.hpp"
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
slrComponent::slrComponent() {
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
slrComponent::~slrComponent() {
}

//------------------------------------------------------------------------------
// documentation is inherited
string slrComponent::getComponentName() const {
    const string name = SLR_COMPONENT_NAME;
    return name;
}

//------------------------------------------------------------------------------
// documentation is inherited
void slrComponent::init( Core* coreptr ) {
    
    logger.open( getComponentName(), false, Logger::getGlobalLogger().getEchoToFile(), Logger::getGlobalLogger().getMinLogLevel() );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;
    
    core = coreptr;
    
    // Mean global temperature is the only thing used to calculate sea level rise
    // Register our dependencies
    core->registerDependency( D_GLOBAL_TEMP, getComponentName() );
    
	refperiod_low = 1951;
	refperiod_high = 1980;
	normalize_year = 1990;
	slr.name = "slr";
	sl_rc.name = "sl_rc";
	tgav.name = "slr_tgav";
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval slrComponent::sendMessage( const std::string& message,
                                  const std::string& datum,
                                  const message_data info ) throw ( h_exception )
{
    unitval returnval;
    
    if( message==M_GETDATA ) {          //! Caller is requesting data
        return getData( datum, info.date );
        
    } else if( message==M_SETDATA ) {   //! Caller is requesting to set data
        H_THROW("SLR sendMessage not yet implemented for message=M_SETDATA.");
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
void slrComponent::setData( const string& varName,
                            const message_data& data ) throw ( h_exception )
{
    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << data.date << "]=" << data.value_str << std::endl;
    
    H_THROW( "Unknown variable name while parsing " + getComponentName() + ": "
            + varName );
}

//------------------------------------------------------------------------------
// documentation is inherited
void slrComponent::prepareToRun() throw ( h_exception ) {
    
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
    oldDate = core->getStartDate();
    H_ASSERT( refperiod_high >= refperiod_low, "bad refperiod" );
}

//------------------------------------------------------------------------------

tseries<double> tgav_vals;

//------------------------------------------------------------------------------
/*! \brief compute sea-level rise
 * from Vermeer and Rahmstorf (2009)
 * computes the first derivative of the global atmospheric temperature cure
 * uses first derivative to compute slr based on thermal expansion and ice melt
 */
void slrComponent::compute_slr( const double date ) {
    
    unitval T = tgav.get( date ) - refperiod_tgav;      // temperature relative to 1951-1980 mean
    
    // First need to compute dTdt, the first derivative of the temperature curve
    double dTdt_double = 0.0;
    if( tgav.size() > 2 ) {
        for( int i=tgav.firstdate(); i<=tgav.lastdate(); i++ ) {
            tgav_vals.set( i, tgav.get( i ).value( U_DEGC ) );
        }
        tgav_vals.allowInterp( true );		// deriv needs a continuous function
        dTdt_double = tgav_vals.get_deriv( date );
    }
    
    // These values and formula below are from:
    // Vermeer, M. and S. Rahmstorf (2009). "Global sea level linked to global temperature." PNAS 106(51): 21527-21532.
    // This incorporates both thermal expansion and ice melt
    const double a	= 0.56;		// cm / year / K
    const double b	= -4.9 ;	// cm / K
    unitval T0;
    T0.set( -0.41, U_DEGC );
    
    unitval dHdt;
    dHdt.set( a * ( T-T0 ).value( U_DEGC ) + b * dTdt_double, U_CM_YR );		// cm/yr
	sl_rc.set( date, dHdt );
	
    unitval dH;
    dH.set( dHdt.value( U_CM_YR ), U_CM );          // integrating to 1 year
    
    unitval slr_to_date;
    slr_to_date.set( 0.0, U_CM );
    if( slr.exists( date-1 ) ) {
        slr_to_date.set( slr.get( date-1 ).value( U_CM ), U_CM );
    }
    slr.set( date, slr_to_date + dH );
    
    unitval X = slr.get( date );
    H_LOG( logger, Logger::DEBUG ) << date << " T=" << T << " dTdt=" << dTdt_double << " dHdt=" << dHdt << " slr=" << X << std::endl ;
    
    // These values incorporate only thermal expansion
    const double a_no_ice	= 0.08;		// cm / year / degree C
    const double b_no_ice	= 2.5;		// cm / degree C
    unitval T0_no_ice;
    T0_no_ice.set (-0.375, U_DEGC );	// K
	
    unitval dHdt_no_ice;
    dHdt_no_ice.set( a_no_ice * ( T-T0_no_ice ).value( U_DEGC ) + b_no_ice * dTdt_double, U_CM_YR );		// cm/yr
    sl_rc_no_ice.set( date, dHdt_no_ice );
	
    unitval dH_no_ice;
    dH_no_ice.set( dHdt_no_ice.value( U_CM_YR ), U_CM );          // integrating to 1 year
    
    unitval slr_no_ice_to_date;
    slr_no_ice_to_date.set( 0.0, U_CM );
    if( slr_no_ice.exists( date-1 ) ) {
        slr_no_ice_to_date = slr_no_ice.get( date-1 );
    }
    
    slr_no_ice.set( date, slr_no_ice_to_date + dH_no_ice );
    
    unitval Y = slr_no_ice.get( date );
    H_LOG( logger, Logger::DEBUG ) << date << " T=" << T << " Dtdt=" << dTdt_double << " DHdt_no_ice=" << dHdt_no_ice << " slr_no_ice=" << Y << std::endl ;
}

//------------------------------------------------------------------------------
// documentation is inherited
void slrComponent::run( const double runToDate ) throw ( h_exception ) {
    
    H_LOG( logger, Logger::DEBUG ) << "SLR run " << runToDate << std::endl;
    
    // Sea level rise is different from some of the other model outputs, because the formula used here to compute it
    // depends on knowing a reference period temperature
	
    tgav.set( runToDate, core->sendMessage( M_GETDATA, D_GLOBAL_TEMP ) );	// store global temperature
    
    if( runToDate==refperiod_high ) {	// then compute reference period temperature
        H_LOG( logger, Logger::DEBUG ) << "Computing reference temperature" << std::endl;
        double sum = 0.0;
        for( int i=refperiod_low; i<=refperiod_high; i++ )
            sum += tgav.get( i ).value( U_DEGC );
        refperiod_tgav.set( sum / ( refperiod_high - refperiod_low + 1 ), U_DEGC );
        H_LOG( logger, Logger::DEBUG ) << "Computed reference temperature " 
                                       << refperiod_tgav.value( U_DEGC ) << " (" << refperiod_low << "-" << refperiod_high << ")" << std::endl;
        
        // Now we can compute and store data up to this point
        for( int i=tgav.firstdate(); i<=refperiod_high; i++ )
            compute_slr( i );
    }
    
    if( runToDate > refperiod_high )
        compute_slr( runToDate );
    oldDate = runToDate;
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval slrComponent::getData( const std::string& varName,
                              const double date ) throw ( h_exception ) {
    
    unitval returnval;
    
    H_ASSERT( date != Core::undefinedIndex(), "Date required for all slr data" );
    
    if( varName == D_SL_RC ) {
        returnval = sl_rc.get( date );
    } else if( varName == D_SL_RC_NO_ICE ) {
        returnval = sl_rc_no_ice.get( date );
    } else if( varName == D_SLR ) {
        returnval = slr.get( date );
    } else if( varName == D_SLR_NO_ICE ) {
        returnval = slr_no_ice.get( date );
    } else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }
    
    return returnval;
}

void slrComponent::reset(double time) throw(h_exception)
{
    oldDate = time;
    sl_rc.truncate(time);
    slr.truncate(time);
    sl_rc_no_ice.truncate(time);
    slr_no_ice.truncate(time);
    tgav.truncate(time);
    
    H_LOG(logger, Logger::NOTICE)
        << getComponentName() << " reset to time= " << time << "\n";
}


    
//------------------------------------------------------------------------------
// documentation is inherited
void slrComponent::shutDown() {
	H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void slrComponent::accept( AVisitor* visitor ) {
    visitor->visit( this );
}

}
