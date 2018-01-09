/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  halocarbon_component.cpp
 *  hector
 *
 *  Created by Pralit Patel on 1/20/11.
 *
 */

#include <math.h>

#include "components/halocarbon_component.hpp"
#include "core/core.hpp"
#include "h_util.hpp"
#include "visitors/avisitor.hpp"

namespace Hector {
  
using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 */
HalocarbonComponent::HalocarbonComponent( std::string g )
:tau( -1 )
{
    myGasName = g;
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
HalocarbonComponent::~HalocarbonComponent() {
}

//------------------------------------------------------------------------------
// documentation is inherited
string HalocarbonComponent::getComponentName() const {
    return myGasName + HALOCARBON_EXTENSION;
}

//------------------------------------------------------------------------------
// documentation is inherited
void HalocarbonComponent::init( Core* coreptr ) {
    logger.open( getComponentName(), false, Logger::DEBUG );
    //    concentration.name = myGasName;
    core = coreptr;

    emissions.allowInterp( true );
    emissions.name = myGasName;
    molarMass = 0.0;
    H0.set( 0.0, U_PPTV );      //! Default is no preindustrial, but user can override
    
    //! \remark Inform core that we can provide forcing data
    core->registerCapability( D_RF_PREFIX+myGasName, getComponentName() );
    // inform core that we can accept emissions for this gas
    core->registerInput(myGasName+EMISSIONS_EXTENSION, getComponentName());
    
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval HalocarbonComponent::sendMessage( const std::string& message,
                                         const std::string& datum,
                                         const message_data info ) throw ( h_exception )
{
    unitval returnval;
    
    if( message==M_GETDATA ) {          //! Caller is requesting data
        return getData( datum, info.date );
        
    } else if( message==M_SETDATA ) {   //! Caller is requesting to set data
        setData( datum, info );
        //TODO: change core so that parsing is routed through sendMessage
        //TODO: make setData private
        
    } else {                        //! We don't handle any other messages
        H_THROW( "Caller sent unknown message: "+message );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void HalocarbonComponent::setData( const string& varName,
                                   const message_data& data ) throw ( h_exception )
{
    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << data.date << "]=" << data.value_str << std::endl;
    
    try {
        const string emiss_var_name = myGasName + EMISSIONS_EXTENSION;
        
        if( varName == D_HC_TAU ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            tau = data.getUnitval(U_UNDEFINED);
        } else if( varName == D_HC_RHO ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            rho = data.getUnitval(U_W_M2_PPTV);
        } else if( varName == D_HC_MOLARMASS ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            molarMass = data.getUnitval(U_UNDEFINED);
        } else if( varName == emiss_var_name ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            emissions.set(data.date, data.getUnitval(U_GG));
        } else if( varName == D_PREINDUSTRIAL_HC ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            H0 = data.getUnitval(U_PPTV);
        } else {
            H_LOG( logger, Logger::DEBUG ) << "Unknown variable " << varName << std::endl;
            H_THROW( "Unknown variable name while parsing "+ getComponentName() + ": "
                    + varName );
        }
    } catch( h_exception& parseException ) {
        H_RETHROW( parseException, "Could not parse var: "+varName );
    }
}

//------------------------------------------------------------------------------
// documentation is inherited
void HalocarbonComponent::prepareToRun() throw ( h_exception ) {
    oldDate = core->getStartDate();
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
    oldDate = core->getStartDate();

    H_ASSERT( tau != -1 && tau != 0, "tau has bad value" );
    H_ASSERT( rho.units() != U_UNDEFINED, "rho has undefined units" );
    //   H_ASSERT( concentration.size() > 0 );
    H_ASSERT( molarMass > 0, "molarMass must be >0" );
    
    Ha = H0;

    
    //! \remark concentration values will not be allowed to interpolate beyond years already read in
    //    concentration.allowPartialInterp( true );
}

//------------------------------------------------------------------------------
// documentation is inherited
void HalocarbonComponent::run( const double runToDate ) throw ( h_exception ) {
	H_ASSERT( !core->inSpinup() && runToDate-oldDate == 1, "timestep must equal 1" );
    oldDate = runToDate;
    
#define AtmosphereDryAirConstant 1.8
    
    const double timestep = 1.0;
    const double alpha = 1 / tau;

    // Compute the delta atmospheric concentration from current emissions
    double emissMol = emissions.get( runToDate ).value( U_GG ) / molarMass * timestep; // this is in U_GMOL
    unitval concDeltaEmiss;
    concDeltaEmiss.set( emissMol / ( 0.1 * AtmosphereDryAirConstant ), U_PPTV );
    
    // Update the atmospheric concentration, accounting for this delta and exponential decay
    double expfac = exp(-alpha);
    Ha = Ha*expfac + concDeltaEmiss*tau * (1.0-expfac);
    H_LOG( logger, Logger::DEBUG ) << "date: " << runToDate << " concentration: "<< Ha << endl;

    // Calculate radiative forcing    TODO: this should be moved to forcing component
    unitval rf;
    rf.set( rho.value( U_W_M2_PPTV ) * Ha.value( U_PPTV ), U_W_M2 );
    hc_forcing.set( runToDate, rf );
    
    /*
     if( runToDate <= calcDate ) {
     unitval temp;
     temp.set( rho.value( U_W_M2_PPTV ) * concentration.get( runToDate ).value( U_PPTV ), U_W_M2 );
     hc_forcing.set( runToDate, temp );
     return;
     } // just use this code to take the emissions and convert to concentrations and then get the forcing.  keep running track of what the concentration is
     for( ; calcDate < runToDate; ++calcDate ) {
     double currDate = calcDate + 1;
     H_LOG( logger, Logger::DEBUG ) << "date: " << currDate << endl;
     unitval concentrationBack = rk( calcDate, currDate );
     H_LOG( logger, Logger::DEBUG ) << "concentration: " << concentrationBack << endl;
     concentration.set( currDate, concentrationBack );
     unitval forcingBack;
     forcingBack.set( rho.value( U_W_M2_PPTV ) * concentrationBack.value( U_PPTV ), U_W_M2 );
     H_LOG( logger, Logger::DEBUG ) << "forcing: " << forcingBack << endl;
     hc_forcing.set( currDate, forcingBack );
     }
     */
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval HalocarbonComponent::getData( const std::string& varName,
                                     const double date ) throw ( h_exception ) {
    
    unitval returnval;
    
    if( varName == D_RF_PREFIX+myGasName ) {
        H_ASSERT( date != Core::undefinedIndex(), "Date required for hc_forcing" );
        returnval = hc_forcing.get( date );
    } else if( varName == D_PREINDUSTRIAL_HC ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for preindustrial hc" );
        returnval = H0;
    } else if( varName == D_HC_CONCENTRATION ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for hc concentration" );
        returnval = Ha;
    } else if( varName == D_HC_EMISSION ) {
        H_ASSERT( date != Core::undefinedIndex(), "Date required for hc emissions" );
        if( date >= emissions.firstdate() )
            returnval = emissions.get( date );
        else
            returnval.set( 0.0, U_GG );
    } else if( varName == D_HC_CALCDATE ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for calcdate" );
        returnval.set( calcDate, U_UNITLESS );
    } else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void HalocarbonComponent::shutDown() {
	H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void HalocarbonComponent::accept( AVisitor* visitor ) {
    visitor->visit( this );
}


//------------------------------------------------------------------------------
/* \brief                       Calculate the derivative of this halocarbon at the
 *                              current date given the current concentration
 * \param[in] currConcentration The current concentration at the current date
 * \param[in] currDate          The current date.
 * \return                      change from current concentration
 *
 *  The change from the current concentration is computed by by adding emissions
 *  and assuming a constant decay of the halocarbons currently in the atmosphere.
 */
/*
 unitval HalocarbonComponent::getChangeInConcentration( unitval currConcentration, double currDate ) const
 {
 #define AtmosphereDryAirConstant 1.8
 
 const double timestep = 1.0;
 const double alpha = 1 / tau;
 
 unitval cumulativeEmissMol;
 cumulativeEmissMol.set( emissions.get( currDate ).value( U_GG ) / molarMass * timestep, U_GMOL );
 
 unitval concFromEmiss;
 concFromEmiss.set( cumulativeEmissMol.value( U_GMOL ) / ( 0.1 * AtmosphereDryAirConstant ), U_PPTV );
 
 return currConcentration * -alpha + concFromEmiss;
 }
 */

//------------------------------------------------------------------------------
/* \brief               Runge-Kutta approximation
 * \param[in] time1     The start year which we assume we have already have a concentration
 * \param[in] time2     The end year in which we want to approximate a concentration
 * \return              The approximate solution
 *
 * A simple Runge-Kutta to approximate the concentration over some time step.
 */
/*
 unitval HalocarbonComponent::rk( double time1, double time2 ) const {
 const double h = 1; // relates to time2?
 unitval currConcentration = concentration.get( time1 );
 unitval k0 = getChangeInConcentration( currConcentration, time1 );
 unitval k1 = getChangeInConcentration( currConcentration + k0 * 0.5, time1 + 0.5 * h );
 unitval k2 = getChangeInConcentration( currConcentration + k1 * 0.5, time1 + 0.5 * h );
 unitval k3 = getChangeInConcentration( currConcentration + k2, time1 + h );
 return currConcentration + ( k0 + k1 * 2.0 + k2 * 2.0 + k3 ) / 6.0;
 }
 */

}
