/*
 *  halocarbon_comp.cpp
 *  hector
 *
 *  Created by Corinne on 5/29/2013
 *
 */

#include <boost/lexical_cast.hpp>

#include "core/core.hpp"
#include "core/halocarbon_comp.hpp"

#include "core/avisitor.hpp"
#include "h_util.hpp"

using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 */
HalocarbonComponent::HalocarbonComponent() {
    halocarbon_concentrations.allowInterp( true );
    halocarbon_concentrations.name = HALOCARBON_COMPONENT_NAME;
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
HalocarbonComponent::~HalocarbonComponent() {
}

//------------------------------------------------------------------------------
// documentation is inherited
string HalocarbonComponent::getComponentName() const {
    const string name = HALOCARBON_COMPONENT_NAME;
    
    return name;
}

//------------------------------------------------------------------------------
// documentation is inherited
void HalocarbonComponent::init( Core* core ) {
    logger.open( getComponentName(), false, Logger::DEBUG );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;

    // Inform core that we are a sulfur model
    core->registerCapability( CAP_HALOCARBON, getComponentName() );

    // Inform core what data we can provide
    core->registerCapability( D_RHO, getComponentName() );
    core->registerCapability( D_TAU, getComponentName() );
    core->registerCapability( D_CONC_HALOCARBON, getComponentName() );
}

//------------------------------------------------------------------------------
// documentation is inherited
void HalocarbonComponent::setData( const string& varName,
                                   const string& value,
                                   const double valueIndex ) throw ( h_exception )
{
    using namespace boost;

    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << valueIndex << "]=" << value << std::endl;

    try {
        if( varName ==  D_CONC_HALOCARBON ) {
            H_ASSERT( valueIndex != Core::undefinedIndex() );
            unitval temp;
            temp.set( lexical_cast<double>( value ), U_PPTV );
            halocarbon_concentrations.set( valueIndex, temp );
        } 
		else if( varName ==  D_RHO  ) {
            H_ASSERT( valueIndex == Core::undefinedIndex() );
            rho.set( lexical_cast<double>( value ), U_W_M2_PPTV );
        }
		else if( varName ==  D_TAU  ) {
            H_ASSERT( valueIndex == Core::undefinedIndex() );
            tau.set( lexical_cast<double>( value ), U_YR );
        }
		else {
            H_THROW( "Unknown variable name while parsing " + getComponentName() + ": "
                     + varName );
        }
    } catch( bad_lexical_cast& castException ) {
        H_THROW( "Could not convert var: "+varName+", value: " + value + ", exception: "
                 +castException.what() );
    }
}

//------------------------------------------------------------------------------
// documentation is inherited
void HalocarbonComponent::prepareToRun() throw ( h_exception ) {

    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
}

//------------------------------------------------------------------------------
// documentation is inherited
void HalocarbonComponent::run( const double runToDate ) throw ( h_exception ) {
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval HalocarbonComponent::getData( const std::string& varName,
                             const double date ) throw ( h_exception ) {
    
    unitval returnval;
    
    if( varName == D_CONC_HALOCARBON ) {
        returnval = halocarbon_concentrations.get( date );
    } 
	else if( varName == D_RHO ) {
        returnval = rho;
    }
	else if( varName == D_TAU ) {
        returnval = tau;
    }
	else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void SulfurComponent::shutDown() {
    logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void SulfurComponent::accept( AVisitor* visitor ) {
    visitor->visit( this );
}