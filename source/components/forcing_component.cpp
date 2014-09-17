/*
 *  forcing_component.cpp
 *  hector
 *
 *  Created by Ben on 02 March 2011.
 *
 */

#include <boost/lexical_cast.hpp>
#include <math.h>

#include "components/forcing_component.hpp"
#include "components/halocarbon_component.hpp"
#include "core/core.hpp"
#include "core/dependency_finder.hpp"
#include "h_util.hpp"
#include "visitors/avisitor.hpp"

namespace Hector {
  
using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 */
ForcingComponent::ForcingComponent() {
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
ForcingComponent::~ForcingComponent() {
}

//------------------------------------------------------------------------------
// documentation is inherited
string ForcingComponent::getComponentName() const {
    const string name = FORCING_COMPONENT_NAME;
    return name;
}

//------------------------------------------------------------------------------
// documentation is inherited
void ForcingComponent::init( Core* coreptr ) {
    
    logger.open( getComponentName(), false, Logger::DEBUG );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;
    
    core = coreptr;
    
    baseyear = 0.0;
    
    Ftot_constrain.allowInterp( true );
    Ftot_constrain.name = D_RF_TOTAL;
    
    // Register the data we can provide
    core->registerCapability( D_RF_TOTAL, getComponentName() );
    core->registerCapability( D_RF_BASEYEAR, getComponentName() );
    
    // Register our dependencies

    core->registerDependency( D_ATMOSPHERIC_CH4, getComponentName() );
    core->registerDependency( D_ATMOSPHERIC_CO2, getComponentName() );
    core->registerDependency( D_ATMOSPHERIC_O3, getComponentName() );
    core->registerDependency( D_EMISSIONS_BC, getComponentName() );
    core->registerDependency( D_EMISSIONS_OC, getComponentName() );
    core->registerDependency( D_ATMOSPHERIC_SO2, getComponentName() );
    core->registerDependency( D_ATMOSPHERIC_N2O, getComponentName() );
    
    core->registerDependency( D_RF_CF4, getComponentName() );
    core->registerDependency( D_RF_C2F6, getComponentName() );
	core->registerDependency( D_RF_HFC23, getComponentName() );
	core->registerDependency( D_RF_HFC32, getComponentName() );
	core->registerDependency( D_RF_HFC4310, getComponentName() );
	core->registerDependency( D_RF_HFC125, getComponentName() );
    core->registerDependency( D_RF_HFC134a, getComponentName() );
    core->registerDependency( D_RF_HFC143a, getComponentName() );
    core->registerDependency( D_RF_HFC227ea, getComponentName() );
    core->registerDependency( D_RF_HFC245fa, getComponentName() );
    core->registerDependency( D_RF_SF6, getComponentName() );
	core->registerDependency( D_RF_CFC11, getComponentName() );
	core->registerDependency( D_RF_CFC12, getComponentName() );
	core->registerDependency( D_RF_CFC113, getComponentName() );
	core->registerDependency( D_RF_CFC114, getComponentName() );
	core->registerDependency( D_RF_CFC115, getComponentName() );
	core->registerDependency( D_RF_CCl4, getComponentName() );
	core->registerDependency( D_RF_CH3CCl3, getComponentName() );
	core->registerDependency( D_RF_HCF22, getComponentName() );
	core->registerDependency( D_RF_HCF141b, getComponentName() );
	core->registerDependency( D_RF_HCF142b, getComponentName() );
	core->registerDependency( D_RF_halon1211, getComponentName() );
	core->registerDependency( D_RF_halon1301, getComponentName() );
	core->registerDependency( D_RF_halon2402, getComponentName() );
	core->registerDependency( D_RF_CH3Br, getComponentName() );
	core->registerDependency( D_RF_CH3Cl, getComponentName() );
    core->registerDependency( D_RF_T_ALBEDO, getComponentName() );
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval ForcingComponent::sendMessage( const std::string& message,
                                      const std::string& datum,
                                      const message_data info ) throw ( h_exception )
{
    unitval returnval;
    
    if( message==M_GETDATA ) {          //! Caller is requesting data
        return getData( datum, info.date );
        
    } else if( message==M_SETDATA ) {   //! Caller is requesting to set data
        H_THROW("Forcing: sendMessage() is not yet implemented for message=M_SETDATA");
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
void ForcingComponent::setData( const string& varName,
                                const message_data& data ) throw ( h_exception )
{
    
    using namespace boost;
    
    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << data.date << "]=" << data.value_str << std::endl;
    
    try {
        if( varName == D_RF_BASEYEAR ) {
            H_ASSERT( data.date == Core::undefinedIndex(), "date not allowed" );
            baseyear = lexical_cast<double>( data.value_str );
        } else if( varName == D_FTOT_CONSTRAIN ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            Ftot_constrain.set( data.date, unitval::parse_unitval( data.value_str, data.units_str, U_W_M2 ) );
		} else {
            H_LOG( logger, Logger::DEBUG ) << "Unknown variable " << varName << std::endl;
            H_THROW( "Unknown variable name while parsing "+ getComponentName() + ": "
                    + varName );
        }
    } catch( bad_lexical_cast& castException ) {
        H_LOG( logger, Logger::DEBUG ) << "Could not convert " << varName << std::endl;
        H_THROW( "Could not convert var: "+varName+", value: " + data.value_str + ", exception: "
                +castException.what() );
    } catch( h_exception& parseException ) {
        H_RETHROW( parseException, "Could not parse var: "+varName );
    }
}

//------------------------------------------------------------------------------
// documentation is inherited
void ForcingComponent::prepareToRun() throw ( h_exception ) {
    
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
    
    if( baseyear==0.0 )
        baseyear = core->getStartDate() + 1;        // default, if not supplied by user
    H_LOG( logger, Logger::DEBUG ) << "Base year for reporting is " << baseyear << std::endl;
	
    H_ASSERT( baseyear > core->getStartDate(), "Base year must be >= model start date" );
    
    if( Ftot_constrain.size() ) {
        Logger& glog = Logger::getGlobalLogger();
        H_LOG( glog, Logger::WARNING ) << "Total forcing will be overwritten by user-supplied values!" << std::endl;
    }
    
    baseyear_forcings.clear();
}

//------------------------------------------------------------------------------
// documentation is inherited
void ForcingComponent::run( const double runToDate ) throw ( h_exception ) {
    
    // Calculate instantaneous radiative forcing for any & all agents
    // As each is computed, push it into 'forcings' map for Ftot calculation
    H_LOG( logger, Logger::DEBUG ) << "-----------------------------" << std::endl;
    forcings.clear();
    currentYear = runToDate;
    
    if( runToDate < baseyear ) {
        H_LOG( logger, Logger::DEBUG ) << "not yet at baseyear" << std::endl;
    } else {
        // ---------- CO2 ----------
        // Instantaneous radiative forcings for CO2, CH4, and N2O from http://www.esrl.noaa.gov/gmd/aggi/
        // These are in turn from IPCC (2001)
        
        // This is identical to that of MAGICC; see Meinshausen et al. (2011)
        unitval Ca = core->sendMessage( M_GETDATA, D_ATMOSPHERIC_CO2 );
        if( runToDate==baseyear )
            C0 = Ca;
        forcings[ "FCO2" ].set( 5.35 * log( Ca/C0 ), U_W_M2 );
        
        // ---------- Terrestrial albedo ----------
        if( core->checkCapability( D_RF_T_ALBEDO ) ) {
            forcings[ D_RF_T_ALBEDO ] = core->sendMessage( M_GETDATA, D_RF_T_ALBEDO, message_data( runToDate ) );
        }
        
        // ---------- N2O and CH4 ----------
        // currently using concentrations.  Equations from Joos et al., 2001
        if( core->checkCapability( D_ATMOSPHERIC_CH4 ) && core->checkCapability( D_ATMOSPHERIC_N2O ) ) {
            
            #define f(M,N) 0.47 * log( 1 + 2.01 * 1e-5 * pow( M * N, 0.75 ) + 5.31 * 1e-15 * M * pow( M * N, 1.52 ) )
            double Ma = core->sendMessage( M_GETDATA, D_ATMOSPHERIC_CH4, message_data( runToDate ) ).value( U_PPBV_CH4 );
            double M0 = core->sendMessage( M_GETDATA, D_PREINDUSTRIAL_CH4 ).value( U_PPBV_CH4 );
            double Na = core->sendMessage( M_GETDATA, D_ATMOSPHERIC_N2O, message_data( runToDate ) ).value( U_PPBV_N2O );
            double N0 = core->sendMessage( M_GETDATA, D_PREINDUSTRIAL_N2O ).value( U_PPBV_N2O );
            
            double fch4 =  0.036 * ( sqrt( Ma ) - sqrt( M0 ) ) - ( f( Ma, N0 ) - f( M0, N0 ) );
            forcings[ "Fch4" ].set( fch4, U_W_M2 );
            
            double fn2o =  0.12 * ( sqrt( Na ) - sqrt( N0 ) ) - ( f( M0, Na ) - f( M0, N0 ) );
            forcings[ "Fn2o" ].set( fn2o, U_W_M2 );
            
            // ---------- Stratospheric H2O from CH4 oxidation ----------		
            //from Tanaka et al, 2007, but using Joos et al., 2001 value of 0.05
            const double fh2o = 0.05 * ( 0.036 * ( sqrt( Ma ) - sqrt( M0 ) ) );
            forcings[ "Fh2o" ].set( fh2o, U_W_M2 );
        }
        
		// ---------- Troposheric Ozone ----------
        if( core->checkCapability( D_ATMOSPHERIC_O3 ) ) {
            //from Tanaka et al, 2007, but using Joos et al., 2001 value of 0.042
            const double ozone = core->sendMessage( M_GETDATA, D_ATMOSPHERIC_O3 ).value( U_DU_O3 );
            const double fo3 = 0.042 * (ozone - 25.0);
            forcings[ "fo3" ].set( fo3, U_W_M2 );
        }
        
		// ---------- Halocarbons ----------
        // Forcing values are actually computed by the halocarbon itself
        forcings[ D_RF_CF4 ] = core->sendMessage( M_GETDATA, D_RF_CF4, message_data( runToDate ) );
        forcings[ D_RF_C2F6 ] = core->sendMessage( M_GETDATA, D_RF_C2F6, message_data( runToDate ) );
        forcings[ D_RF_HFC23 ] = core->sendMessage( M_GETDATA, D_RF_HFC23, message_data( runToDate ) );
        forcings[ D_RF_HFC32 ] = core->sendMessage( M_GETDATA, D_RF_HFC32, message_data( runToDate ) );
        forcings[ D_RF_HFC4310 ] = core->sendMessage( M_GETDATA, D_RF_HFC4310, message_data( runToDate ) );
        forcings[ D_RF_HFC125 ] = core->sendMessage( M_GETDATA, D_RF_HFC125, message_data( runToDate ) );
        forcings[ D_RF_HFC134a ] = core->sendMessage( M_GETDATA, D_RF_HFC134a, message_data( runToDate ) );
        forcings[ D_RF_HFC143a ] = core->sendMessage( M_GETDATA, D_RF_HFC143a, message_data( runToDate ) );
        forcings[ D_RF_HFC227ea ] = core->sendMessage( M_GETDATA, D_RF_HFC227ea, message_data( runToDate ) );
        forcings[ D_RF_HFC245fa ] = core->sendMessage( M_GETDATA, D_RF_HFC245fa, message_data( runToDate ) );
        forcings[ D_RF_SF6 ] = core->sendMessage( M_GETDATA, D_RF_SF6, message_data( runToDate ) );
        forcings[ D_RF_CFC11 ] = core->sendMessage( M_GETDATA, D_RF_CFC11, message_data( runToDate ) );
        forcings[ D_RF_CFC12 ] = core->sendMessage( M_GETDATA, D_RF_CFC12, message_data( runToDate ) );
        forcings[ D_RF_CFC113 ] = core->sendMessage( M_GETDATA, D_RF_CFC113, message_data( runToDate ) );
        forcings[ D_RF_CFC114 ] = core->sendMessage( M_GETDATA, D_RF_CFC114, message_data( runToDate ) );
        forcings[ D_RF_CFC115 ] = core->sendMessage( M_GETDATA, D_RF_CFC115, message_data( runToDate ) );
        forcings[ D_RF_CCl4 ] = core->sendMessage( M_GETDATA, D_RF_CCl4, message_data( runToDate ) );
        forcings[ D_RF_CH3CCl3 ] = core->sendMessage( M_GETDATA, D_RF_CH3CCl3, message_data( runToDate ) );
        forcings[ D_RF_HCF22 ] = core->sendMessage( M_GETDATA, D_RF_HCF22, message_data( runToDate ) );
        forcings[ D_RF_HCF141b ] = core->sendMessage( M_GETDATA, D_RF_HCF141b, message_data( runToDate ) );
        forcings[ D_RF_HCF142b ] = core->sendMessage( M_GETDATA, D_RF_HCF142b, message_data( runToDate ) );
        forcings[ D_RF_halon1211 ] = core->sendMessage( M_GETDATA, D_RF_halon1211, message_data( runToDate ) );
        forcings[ D_RF_halon1301 ] = core->sendMessage( M_GETDATA, D_RF_halon1301, message_data( runToDate ) );
        forcings[ D_RF_halon2402 ] = core->sendMessage( M_GETDATA, D_RF_halon2402, message_data( runToDate ) );
        forcings[ D_RF_CH3Cl ] = core->sendMessage( M_GETDATA, D_RF_CH3Cl, message_data( runToDate ) );
        forcings[ D_RF_CH3Br ] = core->sendMessage( M_GETDATA, D_RF_CH3Br, message_data( runToDate ) );
        
        // ---------- Black carbon ----------
        if( core->checkCapability( D_EMISSIONS_BC ) ) {
            double fbc = 0.0743e-9 * core->sendMessage( M_GETDATA, D_EMISSIONS_BC, message_data( runToDate ) ).value( U_KG );
            forcings[ "Fbc" ].set( fbc, U_W_M2 );
            // includes both indirect and direct forcings from Bond et al 2013, Journal of Geophysical Research Atmo
            // PI value from RCP sceanrios is subtracted off all emissions
          }
        
        // ---------- Organic carbon ----------
        if( core->checkCapability( D_EMISSIONS_OC ) ) {
            double foc = -0.0128e-9 * core->sendMessage( M_GETDATA, D_EMISSIONS_OC, message_data( runToDate ) ).value( U_KG );
            forcings[ "Foc" ].set( foc, U_W_M2 );
            // includes both indirect and direct forcings from Bond et al 2013, Journal of Geophysical Research Atmo (table C1). 
            // The fossil fuel and biomass are weighted (-4.5) then added to the snow and clouds for a total of -12.8.  
            // PI value from RCP scenarios is subtracted off all emissions
          }
        
        // ---------- Sulphate Aerosols ----------
        if( core->checkCapability( D_ATMOSPHERIC_SO2 ) && core->checkCapability( D_EMISSIONS_SO2 ) ) {
            // calculated slightly different than BC/OC
            unitval S0 = core->sendMessage( M_GETDATA, D_PREINDUSTRIAL_SO2 );
            unitval SN = core->sendMessage( M_GETDATA, D_ATMOSPHERIC_SO2 );
            
            // Our forcing calculation returns an absolute value. We store the first (preindustrial) value,
            // and use that to compute a delta forcing, which is what we really want.
            H_ASSERT( S0.value( U_GG ) >0, "S0 is 0" );
            unitval emission = core->sendMessage( M_GETDATA, D_EMISSIONS_SO2, message_data( runToDate ) );
            emission = ((emission/32e-9)*64e-9); // convert from Ggrams of sulfur to Ggrams of SO2. 
            forcings[ "Fso2d" ].set( -0.4 * emission/S0, U_W_M2 );
            
            // includes only direct forcings from Forster etal 2007 (IPCC)
            // emissions from RCP
            
            // indirect aerosol effect via changes in cloud properties
            //double SN = 42000; //Gg of S value take from Joos et al 2001. global biogeochem cycles
            double fso2i = -0.8 * ( log( SN.value( U_GG ) + emission.value( U_GG ) ) / SN.value( U_GG ) ) / ( ( log( SN.value( U_GG ) + S0.value( U_GG ) )/SN.value( U_GG ) ) );
            forcings[ "Fso2i" ].set( fso2i, U_W_M2 );
            //includes only indirect forcing
            //double fso2 = fso2i + fso2;
        }
        
        if( core->checkCapability( D_VOLCANIC_SO2 ) ) {
           // volcanic forcings
            forcings[ "Fvol" ] = core->sendMessage( M_GETDATA, D_VOLCANIC_SO2, message_data( runToDate ) );
        }
        
        // ---------- Total ----------
        unitval Ftot( 0.0, U_W_M2 );  // W/m2
        for( forcingsIterator it = forcings.begin(); it != forcings.end(); ++it ) {
            Ftot = Ftot + ( *it ).second;
            H_LOG( logger, Logger::DEBUG ) << "forcing " << ( *it).first << " in " << runToDate << " is " << ( *it ).second << std::endl;
        }
        
        // If the user has supplied total forcing data, use that
        if( Ftot_constrain.size() ) {
            H_LOG( logger, Logger::WARNING ) << "** Overwriting total forcing with user-supplied value" << std::endl;
            forcings[ D_RF_TOTAL ] = Ftot_constrain.get( runToDate );
        } else {
            forcings[ D_RF_TOTAL ] = Ftot;
        }
        H_LOG( logger, Logger::DEBUG ) << "forcing total is " << forcings[ D_RF_TOTAL ] << std::endl;
        
        //---------- Change to relative forcing ----------
        // At this point, we've computed all absolute forcings. If base year, save those values
        if( runToDate==baseyear ) {
            H_LOG( logger, Logger::DEBUG ) << "** At base year! Storing current forcing values" << std::endl;
            baseyear_forcings = forcings;
        }
        
        // Subtract base year forcing values from forcings, i.e. make them relative to base year
        for( forcingsIterator it = forcings.begin(); it != forcings.end(); ++it ) {
            forcings[ ( *it ).first ] = ( *it ).second - baseyear_forcings[ ( *it ).first ];
        }
        
     }
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval ForcingComponent::getData( const std::string& varName,
                                  const double date ) throw ( h_exception ) {
    
    
    unitval returnval;
    
    H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for forcing data" );
    
    if ( forcings.find( varName ) == forcings.end() && baseyear > currentYear ) {
        // No forcing exists. This probably means we haven't yet reached baseyear,
        // so create an entry of 0.0 to return below.
        forcings[ varName ].set( 0.0, U_W_M2 );
    }
    
    if( varName == D_RF_BASEYEAR ) {
        returnval.set( baseyear, U_UNITLESS );
	} else if ( forcings.find( varName ) != forcings.end() ) {  // from the forcing map
        returnval = forcings[ varName ];
    } else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void ForcingComponent::shutDown() {
	H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void ForcingComponent::accept( AVisitor* visitor ) {
    visitor->visit( this );
}

}
