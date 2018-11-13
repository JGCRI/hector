/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  bc_component.cpp
 *  hector
 *
 *  Created by Ben on 05/26/2011.
 *
 */

#include "bc_component.hpp"
#include "core.hpp"
#include "h_util.hpp"
#include "avisitor.hpp"

namespace Hector {
  
using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 */
BlackCarbonComponent::BlackCarbonComponent() {
    BC_emissions.allowInterp( true );
    BC_emissions.name = BLACK_CARBON_COMPONENT_NAME;
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
BlackCarbonComponent::~BlackCarbonComponent() {
}

//------------------------------------------------------------------------------
// documentation is inherited
string BlackCarbonComponent::getComponentName() const {
    const string name = BLACK_CARBON_COMPONENT_NAME;
    
    return name;
}

//------------------------------------------------------------------------------
// documentation is inherited
void BlackCarbonComponent::init( Core* coreptr ) {
    logger.open( getComponentName(), false, coreptr->getGlobalLogger().getEchoToFile(), coreptr->getGlobalLogger().getMinLogLevel() );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;
    core = coreptr;
    
    // Inform core what data we can accept
    core->registerInput(D_EMISSIONS_BC, getComponentName());
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval BlackCarbonComponent::sendMessage( const std::string& message,
                                          const std::string& datum,
                                          const message_data info ) throw ( h_exception )
{
    unitval returnval;
    
    if( message==M_GETDATA ) {          //! Caller is requesting data
        return getData( datum, info.date );
        
    } else if( message==M_SETDATA ) {   //! Caller is requesting to set data
        //TODO: change core so that parsing is routed through sendMessage
        //TODO: make setData private
        setData(datum, info);
        
    } else {                        //! We don't handle any other messages
        H_THROW( "Caller sent unknown message: "+message );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void BlackCarbonComponent::setData( const string& varName,
                                    const message_data& data ) throw ( h_exception )
{
    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << data.date << "]=" << data.value_str << std::endl;
    
    try {
        if( varName ==  D_EMISSIONS_BC  ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            BC_emissions.set(data.date, data.getUnitval(U_TG));
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
void BlackCarbonComponent::prepareToRun() throw ( h_exception ) {
    
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
    oldDate = core->getStartDate();
}

//------------------------------------------------------------------------------
// documentation is inherited
void BlackCarbonComponent::run( const double runToDate ) throw ( h_exception ) {
    H_ASSERT( !core->inSpinup() && runToDate-oldDate == 1, "timestep must equal 1" );
    oldDate = runToDate;
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval BlackCarbonComponent::getData( const std::string& varName,
                                      const double date ) throw ( h_exception ) {
    
    unitval returnval;
    
    H_ASSERT( date != Core::undefinedIndex(), "Date required for bc_component" );
    
    if( varName == D_EMISSIONS_BC ) {
        returnval = BC_emissions.get( date );
    } else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }
    
    return returnval;
}

void BlackCarbonComponent::reset(double time) throw(h_exception)
{
    // Set time counter to requested date; there are no outputs to
    // reset.
    oldDate = time;
    H_LOG(logger, Logger::NOTICE)
        << getComponentName() << " reset to time= " << time << "\n";
}


//------------------------------------------------------------------------------
// documentation is inherited
void BlackCarbonComponent::shutDown() {
	H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void BlackCarbonComponent::accept( AVisitor* visitor ) {
    visitor->visit( this );
}

}
