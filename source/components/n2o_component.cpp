/*
 *  n2o_component.cpp
 *  hector
 *
 *  Created by Ben on 05/19/2011.
 *
 */

#include "components/n2o_component.hpp"
#include "core/core.hpp"
#include "visitors/avisitor.hpp"
#include "h_util.hpp"

namespace Hector {
  
using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 */
N2OComponent::N2OComponent() {
    Na.allowInterp( true );
    Na.name = N2O_COMPONENT_NAME;
	N0.set( 0.0, U_PPBV_N2O );
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
N2OComponent::~N2OComponent() {
}

//------------------------------------------------------------------------------
// documentation is inherited
string N2OComponent::getComponentName() const {
    const string name = N2O_COMPONENT_NAME;
    
    return name;
}

//------------------------------------------------------------------------------
// documentation is inherited
void N2OComponent::init( Core* coreptr ) {
    logger.open( getComponentName(), false, Logger::DEBUG );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;
    core = coreptr;

    // Inform core what data we can provide
    core->registerCapability( D_ATMOSPHERIC_N2O, getComponentName() );
    core->registerCapability( D_PREINDUSTRIAL_N2O, getComponentName() );
    // register data we can accept as input
    core->registerInput(D_EMISSIONS_N2O, getComponentName());
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval N2OComponent::sendMessage( const std::string& message,
                                  const std::string& datum,
                                  const message_data info ) throw ( h_exception )
{
    unitval returnval;
    
    if( message==M_GETDATA ) {          //! Caller is requesting data
        return getData( datum, info.date );
        
    } else if( message==M_SETDATA ) {   //! Caller is requesting to set data
        H_THROW("N2O sendMessage not yet implemented for message=M_SETDATA");
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
void N2OComponent::setData( const string& varName,
                            const message_data& data ) throw ( h_exception )
{
    try {
        if( varName == D_PREINDUSTRIAL_N2O ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            N0 = unitval::parse_unitval( data.value_str, data.units_str, U_PPBV_N2O );
        } else if( varName == D_ATMOSPHERIC_N2O ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            Na.set( data.date, unitval::parse_unitval( data.value_str, data.units_str, U_PPBV_N2O ) );
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
void N2OComponent::prepareToRun() throw ( h_exception ) {
    
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
	oldDate = core->getStartDate();
}

//------------------------------------------------------------------------------
// documentation is inherited
void N2OComponent::run( const double runToDate ) throw ( h_exception ) {
	H_ASSERT( !core->inSpinup() && runToDate-oldDate == 1, "timestep must equal 1" );
    oldDate = runToDate;
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval N2OComponent::getData( const std::string& varName,
                              const double date ) throw ( h_exception ) {
    
    unitval returnval;
    
    if( varName == D_ATMOSPHERIC_N2O ) {
        H_ASSERT( date != Core::undefinedIndex(), "Date required for atmospheric N2O" );
        returnval = Na.get( date );
    } else if( varName == D_PREINDUSTRIAL_N2O ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for preindustrial N2O" );
        returnval = N0;
    } else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void N2OComponent::shutDown() {
	H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void N2OComponent::accept( AVisitor* visitor ) {
    visitor->visit( this );
}

}
