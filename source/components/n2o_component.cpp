/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  n2o_component.cpp
 *  hector
 *
 *  Created by Ben on 05/19/2011.
 *
 */

#include <math.h>
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
    N2O_emissions.allowInterp( true ); 
    N2O_emissions.name = N2O_COMPONENT_NAME; 
    N2ON_emissions.allowInterp( true ); 
    N2ON_emissions.name = D_NAT_EMISSIONS_N2O; 
    N2O.allowInterp( true );
    N2O.name = D_ATMOSPHERIC_N2O;
    TAU_N2O.allowInterp( true );
    TAU_N2O.name = D_LIFETIME_N2O;
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
    oldDate = core->getStartDate();  //old date will start wehere we begin. 

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
void N2OComponent::setData( const string& varName,
                            const message_data& data ) throw ( h_exception )
{
    try {
        if( varName == D_PREINDUSTRIAL_N2O ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            N0 = data.getUnitval(U_PPBV_N2O);
        } else if( varName == D_EMISSIONS_N2O ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            N2O_emissions.set(data.date, data.getUnitval(U_TG_N2O));
        } else if( varName == D_NAT_EMISSIONS_N2O ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            N2ON_emissions.set(data.date, data.getUnitval(U_TG_N2O));
        } else if( varName == D_CONVERSION_N2O ) {
            H_ASSERT( data.date == Core::undefinedIndex(), "date not allowed" );
            UC_N2O = data.getUnitval(U_TG_PPBV);
        } else if( varName == D_INITIAL_LIFETIME_N2O ) {
            H_ASSERT( data.date == Core::undefinedIndex(), "date not allowed" );
            TN2O0 = data.getUnitval(U_YRS);
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
    N2O.set(oldDate, N0);  // set the first year's value    
  }

//------------------------------------------------------------------------------
// documentation is inherited
void N2OComponent::run( const double runToDate ) throw ( h_exception ) {
    
	H_ASSERT( !core->inSpinup() && runToDate-oldDate == 1, "timestep must equal 1" );
   
    // modified from Ward and Mahowald, 2014
    double previous_n2o = N0.value( U_PPBV_N2O );

    if (runToDate!=oldDate)
    {
      previous_n2o = N2O.get( oldDate );
    }

    //Lifetime varys based on N2O concentrations
    TAU_N2O.set( runToDate, unitval( TN2O0.value( U_YRS ) * ( pow( previous_n2o/N0.value( U_PPBV_N2O ),-0.05 ) ), U_YRS ) ); 
    
    //varying natural emissions from preindustrial to 2000.  After 2000, we hold natural emissions constant
       
    const double current_n2oem = N2O_emissions.get( runToDate ).value( U_TG_N2O ) + N2ON_emissions.get( runToDate ).value( U_TG_N2O ); 
    
    double dN2O = current_n2oem/UC_N2O - previous_n2o/TAU_N2O.get( runToDate ).value( U_YRS );

     N2O.set( runToDate, unitval( previous_n2o + dN2O, U_PPBV_N2O ) );

    oldDate = runToDate;
    H_LOG( logger, Logger::DEBUG ) << "Year " << runToDate << " N2O concentraion = " << N2O.get( runToDate ) << std::endl;
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval N2OComponent::getData( const std::string& varName,
                              const double date ) throw ( h_exception ) {
    
    unitval returnval;
   
    if( varName == D_ATMOSPHERIC_N2O ) {
        H_ASSERT( date != Core::undefinedIndex(), "Date required for atmospheric N2O" );
        returnval = N2O.get( date );
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
