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

#include "halocarbon_component.hpp"
#include "core.hpp"
#include "h_util.hpp"
#include "avisitor.hpp"

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
    logger.open( getComponentName(), false, coreptr->getGlobalLogger().getEchoToFile(), coreptr->getGlobalLogger().getMinLogLevel() );
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
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
    oldDate = core->getStartDate();

    H_ASSERT( tau != -1 && tau != 0, "tau has bad value" );
    H_ASSERT( rho.units() != U_UNDEFINED, "rho has undefined units" );
    H_ASSERT( molarMass > 0, "molarMass must be >0" );
    
    Ha_ts.set(oldDate,H0);

    
    //! \remark concentration values will not be allowed to interpolate beyond years already read in
    //    concentration.allowPartialInterp( true );
}

//------------------------------------------------------------------------------
// documentation is inherited
void HalocarbonComponent::run( const double runToDate ) throw ( h_exception ) {
	H_ASSERT( !core->inSpinup() && runToDate-oldDate == 1, "timestep must equal 1" );
#define AtmosphereDryAirConstant 1.8
    
    const double timestep = 1.0;
    const double alpha = 1 / tau;

    // Compute the delta atmospheric concentration from current emissions
    double emissMol = emissions.get( runToDate ).value( U_GG ) / molarMass * timestep; // this is in U_GMOL
    unitval concDeltaEmiss;
    concDeltaEmiss.set( emissMol / ( 0.1 * AtmosphereDryAirConstant ), U_PPTV );
    
    // Update the atmospheric concentration, accounting for this delta and exponential decay
    double expfac = exp(-alpha);
    unitval Ha(Ha_ts.get(oldDate));
    Ha = Ha*expfac + concDeltaEmiss*tau * (1.0-expfac);
    H_LOG( logger, Logger::DEBUG ) << "date: " << runToDate << " concentration: "<< Ha << endl;
    Ha_ts.set(runToDate, Ha);

    // Calculate radiative forcing    TODO: this should be moved to forcing component
    unitval rf;
    rf.set( rho.value( U_W_M2_PPTV ) * Ha.value( U_PPTV ), U_W_M2 );
    hc_forcing.set( runToDate, rf );

    // Update time counter.
    oldDate = runToDate; 
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval HalocarbonComponent::getData( const std::string& varName,
                                     const double date ) throw ( h_exception ) {
    
    unitval returnval;
    double getdate = date;      // will be used for any variable where a date is allowed.
    if(getdate == Core::undefinedIndex()) {
        // If no date specified, return the last computed date
        getdate = oldDate;
    }
    
    if( varName == D_RF_PREFIX+myGasName ) {
        returnval = hc_forcing.get( getdate );
    }
    else if( varName == D_PREINDUSTRIAL_HC ) {
        // use date as input, not getdate, b/c there should be no date specified.
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for preindustrial hc" );
        returnval = H0;
    }
    else if( varName == D_HC_CONCENTRATION ) {
        returnval = Ha_ts.get(getdate);
    }
    else if( varName == D_HC_EMISSION ) {
        if( getdate >= emissions.firstdate() )
            returnval = emissions.get( getdate );
        else
            returnval.set( 0.0, U_GG );
    }
    else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }
    
    return returnval;
}


void HalocarbonComponent::reset(double time) throw(h_exception)
{
    // reset time counter and truncate outputs
    oldDate = time;
    hc_forcing.truncate(time);
    Ha_ts.truncate(time);
    H_LOG(logger, Logger::NOTICE)
        << getComponentName() << " reset to time= " << time << "\n";
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
