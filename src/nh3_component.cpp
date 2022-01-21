/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  nh3_component.cpp
 *  hector
 *
 */

#include "nh3_component.hpp"
#include "core.hpp"
#include "h_util.hpp"
#include "avisitor.hpp"

namespace Hector {

using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 */
NH3Component::NH3Component() {
    NH3_emissions.allowInterp( true );
    NH3_emissions.name = NH3_COMPONENT_NAME;
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
NH3Component::~NH3Component() {
}

//------------------------------------------------------------------------------
// documentation is inherited
string NH3Component::getComponentName() const {
    const string name = NH3_COMPONENT_NAME;

    return name;
}

//------------------------------------------------------------------------------
// documentation is inherited
void NH3Component::init( Core* coreptr ) {
    logger.open( getComponentName(), false, coreptr->getGlobalLogger().getEchoToFile(), coreptr->getGlobalLogger().getMinLogLevel() );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;
    core = coreptr;

    // Inform core what data we can accept
    core->registerInput(D_EMISSIONS_NH3, getComponentName());
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval NH3Component::sendMessage( const std::string& message,
                                          const std::string& datum,
                                          const message_data info )
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
void NH3Component::setData( const string& varName,
                                    const message_data& data )
{
    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << data.date << "]=" << data.value_str << std::endl;

    try {
        if( varName ==  D_EMISSIONS_NH3  ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            NH3_emissions.set(data.date, data.getUnitval(U_TG));
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
void NH3Component::prepareToRun() {

    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
    oldDate = core->getStartDate();
}

//------------------------------------------------------------------------------
// documentation is inherited
void NH3Component::run( const double runToDate ) {
    H_ASSERT( !core->inSpinup() && runToDate-oldDate == 1, "timestep must equal 1" );
    oldDate = runToDate;
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval NH3Component::getData( const std::string& varName,
                                      const double date ) {

    unitval returnval;

    H_ASSERT( date != Core::undefinedIndex(), "Date required for nh3_component" );

    if( varName == D_EMISSIONS_NH3 ) {
        returnval = NH3_emissions.get( date );
    } else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }

    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void NH3Component::reset(double time)
{
    // Set time counter to requested date; there are no outputs to
    // reset.
    oldDate = time;
    H_LOG(logger, Logger::NOTICE)
        << getComponentName() << " reset to time= " << time << "\n";
}

//------------------------------------------------------------------------------
// documentation is inherited
void NH3Component::shutDown() {
	H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void NH3Component::accept( AVisitor* visitor ) {
    visitor->visit( this );
}

}
