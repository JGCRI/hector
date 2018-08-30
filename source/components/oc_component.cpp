/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  oc_component.cpp
 *  hector
 *
 *  Created by Ben on 05/26/2011.
 *
 */

#include "components/oc_component.hpp"
#include "core/core.hpp"
#include "h_util.hpp"
#include "visitors/avisitor.hpp"

namespace Hector {
  
using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 */
OrganicCarbonComponent::OrganicCarbonComponent() {
    OC_emissions.allowInterp( true );
    OC_emissions.name = ORGANIC_CARBON_COMPONENT_NAME;
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
OrganicCarbonComponent::~OrganicCarbonComponent() {
}

//------------------------------------------------------------------------------
// documentation is inherited
string OrganicCarbonComponent::getComponentName() const {
    const string name = ORGANIC_CARBON_COMPONENT_NAME;
    
    return name;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OrganicCarbonComponent::init( Core* coreptr ) {
    logger.open( getComponentName(), false, Logger::getGlobalLogger().getEchoToFile(), Logger::getGlobalLogger().getMinLogLevel() );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;
  	core = coreptr;

    // Inform core what data we can provide
    core->registerCapability( D_EMISSIONS_OC, getComponentName() );
    // OC emissions are also an input
    core->registerInput(D_EMISSIONS_OC, getComponentName());
}


//------------------------------------------------------------------------------
// documentation is inherited
unitval OrganicCarbonComponent::sendMessage( const std::string& message,
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
        setData(datum, info);
        
    } else {                        //! We don't handle any other messages
        H_THROW( "Caller sent unknown message: "+message );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OrganicCarbonComponent::setData( const string& varName,
                                      const message_data& data ) throw ( h_exception )
{
    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << data.date << "]=" << data.value_str << std::endl;
    
    try {
        if( varName ==  D_EMISSIONS_OC ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            OC_emissions.set(data.date, data.getUnitval(U_TG));
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
void OrganicCarbonComponent::prepareToRun() throw ( h_exception ) {
    
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
	oldDate = core->getStartDate();
}

//------------------------------------------------------------------------------
// documentation is inherited
void OrganicCarbonComponent::run( const double runToDate ) throw ( h_exception ) {
	H_ASSERT( !core->inSpinup() && runToDate-oldDate == 1, "timestep must equal 1" );
    oldDate = runToDate;
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval OrganicCarbonComponent::getData( const std::string& varName,
                                        const double date ) throw ( h_exception ) {
    
    unitval returnval;
    
    H_ASSERT( date != Core::undefinedIndex(), "Date required for oc_component" );
    
    if( varName ==  D_EMISSIONS_OC  ) {
        returnval = OC_emissions.get( date );
    } else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }
    
    return returnval;
}

void OrganicCarbonComponent::reset(double time) throw(h_exception)
{
    oldDate = time;
    H_LOG(logger, Logger::NOTICE)
        << getComponentName() << " reset to time= " << time << "\n";
}

//------------------------------------------------------------------------------
// documentation is inherited
void OrganicCarbonComponent::shutDown() {
	H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void OrganicCarbonComponent::accept( AVisitor* visitor ) {
    visitor->visit( this );
}

}
