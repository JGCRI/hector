/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  so2_component.cpp
 *  hector
 *
 *  Created by Corinne on 5/6/2013
 *
 */

#include "components/so2_component.hpp"
#include "core/core.hpp"
#include "h_util.hpp"
#include "visitors/avisitor.hpp"

namespace Hector {
  
using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 */
SulfurComponent::SulfurComponent() {
    SO2_emissions.allowInterp( true );
	SV.allowInterp( true );
    SO2_emissions.name = SULFUR_COMPONENT_NAME;
	SV.name = SULFUR_COMPONENT_NAME;
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
SulfurComponent::~SulfurComponent() {
}

//------------------------------------------------------------------------------
// documentation is inherited
string SulfurComponent::getComponentName() const {
    const string name = SULFUR_COMPONENT_NAME;
    
    return name;
}

//------------------------------------------------------------------------------
// documentation is inherited
void SulfurComponent::init( Core* coreptr ) {
    logger.open( getComponentName(), false, Logger::getGlobalLogger().getEchoToFile(), Logger::getGlobalLogger().getMinLogLevel() );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;
    core = coreptr;

    // Inform core what data we can provide
    core->registerCapability( D_NATURAL_SO2, getComponentName() );
    core->registerCapability( D_2000_SO2, getComponentName() );
    core->registerCapability( D_EMISSIONS_SO2, getComponentName() );
    core->registerCapability( D_VOLCANIC_SO2, getComponentName() );
    // accept anthro emissions and volcanic emissions as inputs
    core->registerInput(D_EMISSIONS_SO2, getComponentName());
    core->registerInput(D_VOLCANIC_SO2, getComponentName());
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval SulfurComponent::sendMessage( const std::string& message,
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
void SulfurComponent::setData( const string& varName,
                               const message_data& data ) throw ( h_exception )
{
    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << data.date << "]=" << data.value_str << std::endl;
    
    try {
        if( varName ==  D_EMISSIONS_SO2 ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            SO2_emissions.set(data.date, data.getUnitval(U_GG_S));
        }
		else if( varName ==  D_2000_SO2  ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            S0 = data.getUnitval(U_GG_S);
        }
		else if( varName ==  D_NATURAL_SO2  ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            SN = data.getUnitval(U_GG_S);
        }
		else if( varName ==  D_VOLCANIC_SO2  ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            SV.set(data.date, data.getUnitval(U_W_M2));
        }
		else {
            H_THROW( "Unknown variable name while parsing " + getComponentName() + ": "
                    + varName );
        }
    } catch( h_exception& parseException ) {
        H_RETHROW( parseException, "Could not parse var: "+varName );
    }
}

//------------------------------------------------------------------------------
// documentation is inherited
void SulfurComponent::prepareToRun() throw ( h_exception ) {
    
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
    oldDate = core->getStartDate();
}

//------------------------------------------------------------------------------
// documentation is inherited
void SulfurComponent::run( const double runToDate ) throw ( h_exception ) {
    H_ASSERT( !core->inSpinup() && runToDate-oldDate == 1, "timestep must equal 1" );
    oldDate = runToDate;
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval SulfurComponent::getData( const std::string& varName,
                                 const double date ) throw ( h_exception ) {
    
    unitval returnval;
    
    if( varName == D_EMISSIONS_SO2 ) {
        H_ASSERT( date != Core::undefinedIndex(), "Date required for SO2 emissions" );
        returnval = SO2_emissions.get( date );
    }
	else if( varName == D_2000_SO2 ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not supported for SO2 in 2000" );
        returnval = S0;
    }
	else if( varName == D_NATURAL_SO2 ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not supported for natural SO2" );
        returnval = SN;
    }
	else if( varName == D_VOLCANIC_SO2 ) {
        H_ASSERT( date != Core::undefinedIndex(), "Date required for volcanic SO2" );
        if( SV.size() ) returnval = SV.get( date );
           else returnval = unitval( 0.0, U_W_M2 );
    }
	else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }
    
    return returnval;
}

void SulfurComponent::reset(double time) throw(h_exception)
{
    // This component doesn't calculate anything, so all we have to do
    // is reset the time counter.
    oldDate = time;
    H_LOG(logger, Logger::NOTICE)
        << getComponentName() << " reset to time= " << time << "\n";
}



//------------------------------------------------------------------------------
// documentation is inherited
void SulfurComponent::shutDown() {
	H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void SulfurComponent::accept( AVisitor* visitor ) {
    visitor->visit( this );
}

}
