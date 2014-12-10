/*
 *  ozone_component.cpp
 *  hector
 *
 *  Created by Adria Schwarber on 10/24/2013.
 *
 */

#include <math.h>

#include "components/o3_component.hpp"
#include "core/core.hpp"
#include "h_util.hpp"
#include "visitors/avisitor.hpp"

namespace Hector {
  
using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 */
OzoneComponent::OzoneComponent() {
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
OzoneComponent::~OzoneComponent() {
}

//------------------------------------------------------------------------------
// documentation is inherited
string OzoneComponent::getComponentName() const {
    const string name = OZONE_COMPONENT_NAME;

    return name;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OzoneComponent::init( Core* coreptr ) {
    logger.open( getComponentName(), false, Logger::DEBUG );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;
    core = coreptr;

    CO.allowInterp( true );
    NMVOC.allowInterp( true );
    NOX.allowInterp( true );//Inputs like CO and NMVOC and NOX,
    Ma.allowInterp( true );
    O3.set( 0.0, U_DU_O3 );

   
    
	// Inform core what data we can provide
    core->registerCapability(  D_ATMOSPHERIC_O3, getComponentName() );
    //core->registerCapability(  D_PREINDUSTRIAL_O3, getComponentName() );
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval OzoneComponent::sendMessage( const std::string& message,
                                    const std::string& datum,
                                    const message_data info ) throw ( h_exception )
{
    unitval returnval;
    
    if( message==M_GETDATA ) {          //! Caller is requesting data
        return getData( datum, info.date );
        
    } else if( message==M_SETDATA ) {   //! Caller is requesting to set data
        setData(datum, info);
        //TODO: change core so that parsing is routed through sendMessage
        //TODO: make setData private
        
    } else {                        //! We don't handle any other messages
        H_THROW( "Caller sent unknown message: "+message );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OzoneComponent::setData( const string& varName,
                              const message_data& data ) throw ( h_exception )
{
    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << data.date << "]=" << data.value_str << std::endl;
    
    try {
       if( varName == D_EMISSIONS_NOX ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            NOX.set( data.date, unitval::parse_unitval( data.value_str, data.units_str, U_TG_N ) );
        } else if( varName == D_EMISSIONS_CO ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
           CO.set( data.date, unitval::parse_unitval( data.value_str, data.units_str, U_TG_CO ) );
        } else if( varName == D_EMISSIONS_NMVOC ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            NMVOC.set( data.date, unitval::parse_unitval( data.value_str, data.units_str, U_TG_NMVOC ) );
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
void OzoneComponent::prepareToRun() throw ( h_exception ) {
    
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
	oldDate = core->getStartDate();
}

//------------------------------------------------------------------------------
// documentation is inherited
void OzoneComponent::run( const double runToDate ) throw ( h_exception ) {

	// Calculate O3 based on NOX, CO, NMVOC, CH4.

	const double current_nox = NOX.get( runToDate ).value( U_TG_N );
	const double current_co = CO.get( runToDate ).value( U_TG_CO );
	const double current_nmvoc = NMVOC.get( runToDate ).value( U_TG_NMVOC );
	const double current_ch4 = core->sendMessage( M_GETDATA, D_ATMOSPHERIC_CH4 );

    O3.set( ( 5*log(current_ch4 ) ) + ( 0.125*current_nox ) + ( 0.0011*current_co )
               + ( 0.0033*current_nmvoc ), U_DU_O3 );
	H_LOG( logger, Logger::DEBUG ) << "Year " << runToDate << " O3 concentration = " << O3 << std::endl;
}


//------------------------------------------------------------------------------
// documentation is inherited
unitval OzoneComponent::getData( const std::string& varName,
                                 const double date ) throw ( h_exception ) {
    
    unitval returnval;
    
    if( varName == D_ATMOSPHERIC_O3 ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date is not allowed" ); //== is a comparision
		returnval = O3;
    }  else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OzoneComponent::shutDown() {
    H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void OzoneComponent::accept( AVisitor* visitor ) {
    visitor->visit( this );
}

}
