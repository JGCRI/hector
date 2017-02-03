/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  SimpleNbox.cpp
 *  hector
 *
 *  Created by Ben on 5/9/11.
 *
 */

#include "boost/algorithm/string.hpp"

#include "core/dependency_finder.hpp"
#include "models/simpleNbox.hpp"
#include "visitors/avisitor.hpp"

namespace Hector {

using namespace boost;

//------------------------------------------------------------------------------
/*! \brief constructor
 */
SimpleNbox::SimpleNbox() : CarbonCycleModel( 6 ), m_last_tempferts(0.0) {
    anthroEmissions.allowInterp( true );
    anthroEmissions.name = "anthroEmissions";
    lucEmissions.allowInterp( true );
    lucEmissions.name = "lucEmissions";
    Ftalbedo.allowInterp( true );
    Ftalbedo.name = "albedo";
    
    // earth_c keeps track of how much fossil C is pulled out
    // so that we can do a mass-balance check throughout the run
    earth_c.set( 0.0, U_PGC );
}

//------------------------------------------------------------------------------
// documentation is inherited
void SimpleNbox::init( Core* coreptr ) {
    CarbonCycleModel::init( coreptr );
    
    core = coreptr;
    
    // Defaults
    co2fert[ SNBOX_DEFAULT_BIOME ] = 1.0;
    warmingfactor[ SNBOX_DEFAULT_BIOME ] = 1.0;
    residual.set( 0.0, U_PGC );
//    q10_detritus[ SNBOX_DEFAULT_BIOME ] = 2.0;
//    q10_soil[ SNBOX_DEFAULT_BIOME ] = 2.0;
    tempfertd[ SNBOX_DEFAULT_BIOME ] = 1.0;
    tempferts[ SNBOX_DEFAULT_BIOME ] = 1.0;
    // Tgav_sum[ SNBOX_DEFAULT_BIOME ] = 0.0;
    
    Tgav_record.allowInterp( true );
    
    // Register the data we can provide
    core->registerCapability( D_ATMOSPHERIC_CO2, getComponentName() );
    core->registerCapability( D_ATMOSPHERIC_C, getComponentName() );
    core->registerCapability( D_PREINDUSTRIAL_CO2, getComponentName() );
    core->registerCapability( D_RF_T_ALBEDO, getComponentName() );
    core->registerCapability( D_LAND_CFLUX, getComponentName() );
    
    // Register our dependencies
    core->registerDependency( D_OCEAN_CFLUX, getComponentName() );

    // Register the inputs we can receive from outside
    core->registerInput( D_ANTHRO_EMISSIONS, getComponentName() );
    core->registerInput( D_LUC_EMISSIONS, getComponentName() );
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval SimpleNbox::sendMessage( const std::string& message,
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
void SimpleNbox::setData( const std::string &varName,
                          const message_data& data ) throw( h_exception )
{
    // Does the varName contain our parse character? If so, split it
    std::vector<std::string> splitvec;
    boost::split( splitvec, varName, is_any_of( SNBOX_PARSECHAR ) );
    H_ASSERT( splitvec.size() < 3, "max of one separator allowed in variable names" );
    
    std::string biome = SNBOX_DEFAULT_BIOME;
    std::string varNameParsed = varName;
    if( splitvec.size() == 2 ) {    // i.e., in form <biome>.<varname>
        biome = splitvec[ 0 ];
        varNameParsed = splitvec[ 1 ];
    }

    if (data.isVal) {
        H_LOG( logger, Logger::DEBUG ) << "Setting " << biome << "." << varNameParsed << "[" << data.date << "]=" << data.value_unitval << std::endl;
    }
    else {
        H_LOG( logger, Logger::DEBUG ) << "Setting " << biome << "." << varNameParsed << "[" << data.date << "]=" << data.value_str << std::endl;
    }
    try {
        // Initial pools
        if( varNameParsed == D_ATMOSPHERIC_C ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            H_ASSERT( biome == SNBOX_DEFAULT_BIOME, "atmospheric C must be global" );
            atmos_c = data.getUnitval(U_PGC);
            C0.set( atmos_c.value( U_PGC ) * PGC_TO_PPMVCO2, U_PPMV_CO2 );
        }
        else if( varNameParsed == D_PREINDUSTRIAL_CO2 ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            H_ASSERT( biome == SNBOX_DEFAULT_BIOME, "preindustrial C must be global" );
            C0 = data.getUnitval( U_PPMV_CO2 );
            atmos_c.set( C0.value( U_PPMV_CO2 ) / PGC_TO_PPMVCO2, U_PGC );
        }
        else if( varNameParsed == D_VEGC ) {
            H_ASSERT( data.date == Core::undefinedIndex(), "date not allowed" );
            veg_c[ biome ] = data.getUnitval( U_PGC );
        }
        else if( varNameParsed == D_DETRITUSC ) {
            H_ASSERT( data.date == Core::undefinedIndex(), "date not allowed" );
            detritus_c[ biome ] = data.getUnitval( U_PGC );
        }
        else if( varNameParsed == D_SOILC ) {
            H_ASSERT( data.date == Core::undefinedIndex(), "date not allowed" );
            soil_c[ biome ] = data.getUnitval( U_PGC );
        }
        
        // Albedo effect
        else if( varNameParsed == D_RF_T_ALBEDO ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            Ftalbedo.set( data.date, data.getUnitval( U_W_M2 ) );
        }
        
        // Partitioning
        else if( varNameParsed == D_F_NPPV ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            f_nppv = data.getUnitval(U_UNDEFINED);
        }
        else if( varNameParsed == D_F_NPPD ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            f_nppd = data.getUnitval(U_UNDEFINED);
        }
        else if( varNameParsed == D_F_LITTERD ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            f_litterd = data.getUnitval(U_UNDEFINED);
        }
        else if( varNameParsed == D_F_LUCV ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            f_lucv = data.getUnitval(U_UNDEFINED);
        }
        else if( varNameParsed == D_F_LUCD ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            f_lucd = data.getUnitval(U_UNDEFINED);
        }
        
        // Initial fluxes
        else if( varNameParsed == D_NPP_FLUX0 ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            npp_flux0[ biome ] = data.getUnitval( U_PGC_YR );
        }
        
        // Anthropogenic contributions--time series.  There are two
        // message versions for each of these: one for string data
        // read from an input file, and another for actual values
        // passed from another part of the program.
        else if( varNameParsed == D_ANTHRO_EMISSIONS ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            H_ASSERT( biome == SNBOX_DEFAULT_BIOME, "anthro emissions must be global" );
            anthroEmissions.set( data.date, data.getUnitval( U_PGC_YR ) );
        } 
        else if( varNameParsed == D_LUC_EMISSIONS ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            lucEmissions.set( data.date, data.getUnitval( U_PGC_YR ) );
        } 
        // Atmospheric CO2 record to constrain model to (optional)
        else if( varNameParsed == D_CA_CONSTRAIN ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            H_ASSERT( biome == SNBOX_DEFAULT_BIOME, "atmospheric constraint must be global" );
            Ca_constrain.set( data.date, data.getUnitval( U_PPMV_CO2 ) );
        }
        
        // Fertilization
        else if( varNameParsed == D_BETA ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            beta[ biome ] = data.getUnitval(U_UNDEFINED);
        }
        else if( varNameParsed == D_WARMINGFACTOR ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            warmingfactor[ biome ] = data.getUnitval(U_UNDEFINED);
        }
        else if( varNameParsed == D_Q10_RH ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            q10_rh = data.getUnitval(U_UNDEFINED);
        }
     
        else {
            H_LOG( logger, Logger::DEBUG ) << "Unknown variable " << varName << std::endl;
            H_THROW( "Unknown variable name while parsing "+ getComponentName() + ": "
                    + varName );
        }
    } catch( h_exception& parseException ) {
        H_RETHROW( parseException, "Could not parse var: "+varName );
    }
}

//------------------------------------------------------------------------------
/*! \brief      Sanity checks
 *  \exception  If any of the sanity checks fails
 *
 *  This is called internally throughout the model run and performs sanity checks.
 *  For example, the main carbon pools (except earth) should always be positive;
 *  partitioning coefficients should not exceed 1; etc.
 */
void SimpleNbox::sanitychecks() throw( h_exception )
{
    unitval_stringmap::const_iterator it;
    double_stringmap::const_iterator itd;
    
    // Make a few sanity checks here, and then return.
    H_ASSERT( atmos_c.value( U_PGC ) > 0.0, "atmos_c pool <=0" );
    
    for( it = veg_c.begin(); it != veg_c.end(); it++ )
        H_ASSERT( it->second.value( U_PGC ) > 0.0, "veg_c pool <=0" );
    
    for( it = detritus_c.begin(); it != detritus_c.end(); it++ )
        H_ASSERT( it->second.value( U_PGC ) > 0.0, "detritus_c pool <=0" );
 
    for( it = soil_c.begin(); it != soil_c.end(); it++ )
        H_ASSERT( it->second.value( U_PGC ) > 0.0, "soil_c pool <=0" );
 
    H_ASSERT( f_nppv >= 0.0, "f_nppv <0" );
    H_ASSERT( f_nppd >= 0.0, "f_nppd <0" );
    H_ASSERT( f_nppv + f_nppd <= 1.0, "f_nppv + f_nppd >1" );
    H_ASSERT( f_litterd >= 0.0 && f_litterd <= 1.0, "f_litterd <0 or >1" );
    H_ASSERT( f_lucv >= 0.0, "f_lucv <0" );
    H_ASSERT( f_lucd >= 0.0, "f_lucd <0" );
    H_ASSERT( f_lucv + f_lucd <= 1.0, "f_lucv + f_lucd >1" );
    
    for( it = npp_flux0.begin(); it != npp_flux0.end(); it++ )
        H_ASSERT( it->second.value( U_PGC_YR ) > 0.0, "npp_flux0 <=0" );
    
    H_ASSERT( C0.value( U_PPMV_CO2 ) > 0.0, "C0 <= 0" );
    H_ASSERT( Ca.value( U_PPMV_CO2 ) > 0.0, "Ca <= 0" );
}

//------------------------------------------------------------------------------
/*! \brief      Sum a string->unitval map
 *  \param      pool to sum over
 *  \returns    Sum of the unitvals in the map
 *  \exception  If the map is empty
 */
unitval SimpleNbox::sum_map( unitval_stringmap pool ) const
{
    H_ASSERT( pool.size(), "can't sum an empty map" );
    unitval sum( 0.0, pool.begin()->second.units() );
    for( unitval_stringmap::const_iterator it = pool.begin(); it != pool.end(); it++ )
        sum = sum + it->second;
    return sum;
}

//------------------------------------------------------------------------------
/*! \brief      Sum a string->double map
 *  \param      pool to sum over
 *  \returns    Sum of the unitvals in the map
 *  \exception  If the map is empty
 */
double SimpleNbox::sum_map( double_stringmap pool ) const
{
    H_ASSERT( pool.size(), "can't sum an empty map" );
    double sum = 0.0;
    for( double_stringmap::const_iterator it = pool.begin(); it != pool.end(); it++ )
        sum = sum + it->second;
    return sum;
}

//------------------------------------------------------------------------------
/*! \brief      Log pool states
 *  \param      current date
 */
void SimpleNbox::log_pools( const double t )
{
    // Log pool states
    H_LOG( logger,Logger::DEBUG ) << "---- simpleNbox pool states at t=" << t << " ----" << std::endl;
    H_LOG( logger,Logger::DEBUG ) << "Atmos = " << atmos_c << std::endl;
    H_LOG( logger,Logger::DEBUG ) << "Biome \tveg_c \t\tdetritus_c \tsoil_c" << std::endl;
    for( unitval_stringmap::const_iterator it = veg_c.begin(); it != veg_c.end(); it++ ) {
        std::string biome = it->first;
        H_LOG( logger,Logger::DEBUG ) << biome << "\t" << veg_c[ biome ] << "\t" <<
        detritus_c[ biome ] << "\t\t" << soil_c[ biome ] << std::endl;
    }
    H_LOG( logger,Logger::DEBUG ) << "Earth = " << earth_c << std::endl;
}

//------------------------------------------------------------------------------
// documentation is inherited
void SimpleNbox::prepareToRun() throw( h_exception )
{
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
 
    // TODO: if any 'global' settings, there shouldn't also be regional!
    
    // Everything in veg_c map should occur in soil and detritus
    H_ASSERT( veg_c.size() == detritus_c.size(), "veg_c and detritus_c data not same size" );
    H_ASSERT( veg_c.size() == soil_c.size(), "veg_c and soil_c data not same size" );
    H_ASSERT( veg_c.size() == npp_flux0.size(), "veg_c and npp_flux0 not same size" );
    for( unitval_stringmap::const_iterator it=veg_c.begin(); it != veg_c.end(); it++ ) {
        H_LOG( logger, Logger::DEBUG ) << "Checking that " << it->first << " data complete" << std::endl;
        H_ASSERT( detritus_c.count( it->first ), "no biome data for detritus_c" );
        H_ASSERT( soil_c.count( it->first ), "no biome data for soil_c" );
        H_ASSERT( npp_flux0.count( it->first ), "no biome data for npp_flux0" );
        
        if( !beta.count( it->first ) ) {
            H_LOG( logger, Logger::DEBUG ) << "Beta does not exist for this biome; using global value" << std::endl;
            beta[ it->first ] = beta.at( SNBOX_DEFAULT_BIOME );
        }
        
//        Tgav_sum[ it->first ] = 0.0;
   }

    // Save a pointer to the ocean model in use
    omodel = static_cast<CarbonCycleModel*>( core->getComponentByCapability( D_OCEAN_C ) );

    if( !Ftalbedo.size() ) {          // if no albedo data, assume constant
        unitval alb( -0.2, U_W_M2 ); // default is MAGICC value
        Ftalbedo.set( core->getStartDate(), alb );
        Ftalbedo.set( core->getEndDate(), alb );
    }
    
    Ca.set( C0.value( U_PPMV_CO2 ), U_PPMV_CO2 );
    
    if( Ca_constrain.size() ) {
        Ca_constrain.allowPartialInterp( true );
        Logger& glog = Logger::getGlobalLogger();
        H_LOG( glog, Logger::WARNING ) << "Atmospheric CO2 will be constrained to user-supplied values!" << std::endl;
    }
    
    // One-time checks
    double_stringmap::const_iterator itd;
    for( itd = beta.begin(); itd != beta.end(); itd++ ) {
        H_ASSERT( itd->second >= 0.0, "beta < 0" );
    }
    H_ASSERT( q10_rh>0.0, "q10_rh <= 0.0" );
    sanitychecks();
}

//------------------------------------------------------------------------------
/*! \brief                  Run code, called from core
 *  \param[in] runToDate    Date to which to run to, double
 *
 *  This run method doesn't do much, because it's the carbon-cycle-solver
 *  run that does all the work.
 */
void SimpleNbox::run( const double runToDate ) throw ( h_exception )
{
    in_spinup = core->inSpinup();
    sanitychecks();
    
    Tgav_record.set( runToDate, core->sendMessage( M_GETDATA, D_GLOBAL_TEMP ).value( U_DEGC ) );
}

//------------------------------------------------------------------------------
/*! \brief                  Spinup run code, called from core
 *  \param[in] step         Spinup step number
 *
 *  This run_spinup method doesn't do much, because it's the carbon-cycle-solver
 *  run that does all the work.
 */
bool SimpleNbox::run_spinup( const int step ) throw ( h_exception )
{
    sanitychecks();
    in_spinup = true;
    return true;        // solver will really be the one signalling
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval SimpleNbox::getData( const std::string& varName,
                            const double date ) throw ( h_exception ) {
    
    unitval returnval;
    
    if( varName == D_ATMOSPHERIC_C ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for atmospheric C" );
        returnval = atmos_c;
    } else if( varName == D_ATMOSPHERIC_CO2 ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for atmospheric CO2" );
        returnval = Ca;
    } else if( varName == D_ATMOSPHERIC_C_RESIDUAL ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for atmospheric C residual" );
        returnval = residual;
    } else if( varName == D_PREINDUSTRIAL_CO2 ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for preindustrial CO2" );
        returnval = C0;
    } else if( varName == D_LAND_CFLUX ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for atm-land flux" );
        returnval = sum_npp() - sum_rh() - lucEmissions.get( ODEstartdate );
        
    } else if( varName == D_RF_T_ALBEDO ) {
        H_ASSERT( date != Core::undefinedIndex(), "Date required for albedo forcing" );
        returnval = Ftalbedo.get( date );
        
    } else if( varName == D_EARTHC ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for earth C" );
        returnval = earth_c;
    } else if( varName == D_VEGC ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for veg C" );
        returnval = sum_map( veg_c );
    } else if( varName == D_DETRITUSC ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for detritus C" );
        returnval = sum_map( detritus_c );
    } else if( varName == D_SOILC ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for soil C" );
        returnval = sum_map( soil_c );
    } else if( varName == D_ANTHRO_EMISSIONS ) {
        H_ASSERT( date != Core::undefinedIndex(), "Date required for anthro emissions" );
        returnval = anthroEmissions.get( date );
    } else if( varName == D_LUC_EMISSIONS ) {
        H_ASSERT( date != Core::undefinedIndex(), "Date required for luc emissions" );
        returnval = lucEmissions.get( date );
    } else if( varName == D_NPP ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for npp" );
        returnval = sum_npp();
    } else if( varName == D_RH ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for rh" );
        returnval = sum_rh();
    }else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void SimpleNbox::shutDown()
{
	H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}

//------------------------------------------------------------------------------
/*! \brief visitor accept code
 */
void SimpleNbox::accept( AVisitor* visitor ) {
    visitor->visit( this );
}

//------------------------------------------------------------------------------
/*! \brief            transfer model pools to flat array (for ODE solver)
 *  \param[in] t  time, double, the date from which ODE solver is starting
 *  \param[in] c  flat array of carbon pools (no units)
 */
void SimpleNbox::getCValues( double t, double c[] )
{
    c[ SNBOX_ATMOS ] = atmos_c.value( U_PGC );
    c[ SNBOX_VEG ] = sum_map( veg_c ).value( U_PGC );
    c[ SNBOX_DET ] = sum_map( detritus_c ).value( U_PGC );
    c[ SNBOX_SOIL ] = sum_map( soil_c ).value( U_PGC );
    omodel->getCValues( t, c );
    c[ SNBOX_EARTH ] = earth_c.value( U_PGC );
    
    ODEstartdate = t;
}

//------------------------------------------------------------------------------
/*! \brief                  Transfer new model pools from ODE solver array back to model pools
 *  \param[in] t            Time, double, the ending date of the solver
 *  \param[in] double       Flat array of carbon pools (no units)
 *  \exception h_exception  If ocean model diverges from our pool tracking all ocean C
 *  \exception h_exception  If mass is not conserved
 *
 *  \details Transfer solver pools (no units) back to our pools (with units), and run
 *  a sanity check to make sure mass has been conserved.
 */
void SimpleNbox::stashCValues( double t, const double c[] )
{
    // Solver has gone from ODEstartdate to t
    const double yf = ( t - ODEstartdate );
    H_ASSERT( yf >= 0 && yf <= 1, "yearfraction out of bounds" );
    
    H_LOG( logger,Logger::DEBUG ) << "Stashing at t=" << t << ", solver pools at " << t << ": " << c[ 0 ]
    << " " << c[ 1 ] << " " << c[ 2 ] << " " << c[ 3 ] << " " << c[ 4 ] << " " << c[ 5 ] << std::endl;

    log_pools( t );

    // Store solver pools into our internal variables
    
    atmos_c.set( c[ SNBOX_ATMOS ], U_PGC );
    
    // The solver just knows about one vegetation box, one detritus, and one
    // soil. So we need to apportion new veg C pool (set by the solver) to
    // as many biomes as we have. This is not ideal.
    // TODO: Solver actually solves all boxes in multi-biome system

    // Apportioning is done by NPP and RH
    // i.e., biomes with higher values get more of any C change
    const unitval npp_rh_total = sum_npp() + sum_rh(); // these are both positive
    const unitval newveg( c[ SNBOX_VEG ], U_PGC );
    const unitval newdet( c[ SNBOX_DET ], U_PGC );
    const unitval newsoil( c[ SNBOX_SOIL ], U_PGC );
    unitval veg_delta = newveg - sum_map( veg_c );  // TODO: make const
    unitval det_delta = newdet - sum_map( detritus_c );  // TODO: make const
    unitval soil_delta = newsoil - sum_map( soil_c );  // TODO: make const
    H_LOG( logger,Logger::DEBUG ) << "veg_delta = " << veg_delta << std::endl;
    H_LOG( logger,Logger::DEBUG ) << "det_delta = " << det_delta << std::endl;
    H_LOG( logger,Logger::DEBUG ) << "soil_delta = " << soil_delta << std::endl;
    for( unitval_stringmap::const_iterator it = veg_c.begin(); it != veg_c.end(); it++ ) {
        std::string biome = it->first;
        const double wt     = ( npp( biome ) + rh( biome ) ) / npp_rh_total;
        veg_c[ biome ]      = veg_c[ biome ] + veg_delta * wt;
        detritus_c[ biome ] = detritus_c[ biome ] + det_delta * wt;
        soil_c[ biome ]     = soil_c[ biome ] + soil_delta * wt;
        H_LOG( logger,Logger::DEBUG ) << "Biome " << biome << " weight = " << wt << std::endl;
    }

    log_pools( t );
    
    omodel->stashCValues( t, c );   // tell ocean model to store new C values
    earth_c.set( c[ SNBOX_EARTH ], U_PGC );

    log_pools( t );
    
    // Each time the model pools are updated, check that mass has been conserved
    static double lastsum = 0.0;
    double sum=0.0;
    for( int i=0; i<ncpool(); i++ ) {
        sum += c[ i ];
    }

    const double diff = fabs( sum - lastsum );
    H_LOG( logger,Logger::DEBUG ) << "lastsum = " << lastsum << ", sum = " << sum << ", diff = " << diff << std::endl;
    if( lastsum && diff > MB_EPSILON ) {
        H_LOG( logger,Logger::SEVERE ) << "Mass not conserved in " << getComponentName() << std::endl;
        H_LOG( logger,Logger::SEVERE ) << "lastsum = " << lastsum << ", sum = " << sum << ", diff = " << diff << std::endl;
        H_THROW( "Mass not conserved! (See log.)" );
    }
    lastsum = sum;

    // If user has supplied Ca values, adjust atmospheric C to match
    if( !core->inSpinup() && Ca_constrain.size() && t <= Ca_constrain.lastdate() ) {
        
        H_LOG( logger, Logger::WARNING ) << "** Constraining atmospheric CO2 to user-supplied value" << std::endl;
        
        unitval atmos_cpool_to_match( Ca_constrain.get( t ).value( U_PPMV_CO2 ) / PGC_TO_PPMVCO2, U_PGC );
        residual = atmos_c - atmos_cpool_to_match;
        H_LOG( logger,Logger::DEBUG ) << t << "- have " << Ca << " want " << Ca_constrain.get( t ).value( U_PPMV_CO2 ) << std::endl;
        H_LOG( logger,Logger::DEBUG ) << t << "- have " << atmos_c << " want " << atmos_cpool_to_match << "; residual = " << residual << std::endl;
        
        // Transfer C from atmosphere to deep ocean and update our C and Ca variables
        H_LOG( logger,Logger::DEBUG ) << "Sending residual of " << residual << " to deep ocean" << std::endl;
        core->sendMessage( M_DUMP_TO_DEEP_OCEAN, D_OCEAN_C, message_data( residual ) );
        atmos_c = atmos_c - residual;
        Ca.set( atmos_c.value( U_PGC ) * PGC_TO_PPMVCO2, U_PPMV_CO2 );
    } else {
        residual.set( 0.0, U_PGC );
    }
    
    // All good! t will be the start of the next timestep, so
    ODEstartdate = t;
}

// A series of small functions to calculate variables that will appear in the output stream

//------------------------------------------------------------------------------
/*! \brief      Compute annual net primary production
 *  \returns    current annual NPP
 */
unitval SimpleNbox::npp( std::string biome ) const
{
    unitval npp = npp_flux0.at( biome );    // 'at' throws exception if not found
    npp = npp * co2fert.at( biome );        // that's why used here instead of []
//    npp = npp * tempfert.at( biome );
    return npp;
}

//------------------------------------------------------------------------------
/*! \brief      Compute global net primary production
 *  \returns    Annual NPP summed across all biomes
 */
unitval SimpleNbox::sum_npp() const
{
    unitval total( 0.0, U_PGC_YR );
    for( unitval_stringmap::const_iterator it = veg_c.begin(); it != veg_c.end(); it++ ) {
        total = total + npp( it->first );
    }
    return total;
}

//------------------------------------------------------------------------------
/*! \brief      Compute detritus component of annual heterotrophic respiration
 *  \returns    current detritus component of annual heterotrophic respiration
 */
unitval SimpleNbox::rh_fda( std::string biome ) const
{
    unitval dflux( detritus_c.at( biome ).value( U_PGC ) * 0.25, U_PGC_YR );
    return dflux * tempfertd.at( biome );
}

//------------------------------------------------------------------------------
/*! \brief      Compute soil component of annual heterotrophic respiration
 *  \returns    current soil component of annual heterotrophic respiration
 */
unitval SimpleNbox::rh_fsa( std::string biome ) const
{
    unitval soilflux( soil_c.at( biome ).value( U_PGC ) * 0.02, U_PGC_YR );
    return soilflux * tempferts.at( biome );
}

//------------------------------------------------------------------------------
/*! \brief      Compute total annual heterotrophic respiration
 *  \returns    current annual heterotrophic respiration
 */
unitval SimpleNbox::rh( std::string biome ) const
{
    // Heterotrophic respiration is the sum of fluxes from detritus and soil
    return rh_fda( biome ) + rh_fsa( biome );
}

//------------------------------------------------------------------------------
/*! \brief      Compute global heterotrophic respiration
 *  \returns    Annual RH summed across all biomes
 */
unitval SimpleNbox::sum_rh() const
{
    unitval total( 0.0, U_PGC_YR );
    for( unitval_stringmap::const_iterator it = veg_c.begin(); it != veg_c.end(); it++ ) {
        total = total + rh( it->first );
    }
    return total;
}

//------------------------------------------------------------------------------
/*! \brief              Compute model fluxes for a time step
 *  \param[in]  t       time
 *  \param[in]  c       carbon pools (no units)
 *  \param[out] dcdt    carbon fluxes
 *  \returns            code indicating success or failure
 */
int SimpleNbox::calcderivs( double t, const double c[], double dcdt[] ) const
{
    // Solver is attempting to go from ODEstartdate to t
//    const double yearfraction = ( t - ODEstartdate );
//    H_ASSERT( yearfraction >= 0 && yearfraction <= 1, "yearfraction out of bounds" );

    // Atmosphere-ocean flux is calculated by ocean_component
    const int omodel_err = omodel->calcderivs( t, c, dcdt );
    unitval atmosocean_flux( dcdt[ SNBOX_OCEAN ], U_PGC_YR );
    
    // NPP is scaled by CO2 from preindustrial value
    unitval npp_current = sum_npp();
    unitval npp_fav = npp_current * f_nppv;
    unitval npp_fad = npp_current * f_nppd;
    unitval npp_fas = npp_current * ( 1 - f_nppv - f_nppd );
    
    // RH heterotrophic respiration
    unitval rh_fda_current( 0.0, U_PGC_YR );
    unitval rh_fsa_current( 0.0, U_PGC_YR );
    for( unitval_stringmap::const_iterator it = veg_c.begin(); it != veg_c.end(); it++ ) {
        rh_fda_current = rh_fda_current + rh_fda( it->first );
        rh_fsa_current = rh_fsa_current + rh_fsa( it->first );
    }
    unitval rh_current = rh_fda_current + rh_fsa_current;
    
    // Detritus flux comes from the vegetation pool
    // TODO: these values should use the c[] pools passed in by solver!
    unitval litter_flux( 0.0, U_PGC_YR );
    unitval litter_fvd( 0.0, U_PGC_YR );
    unitval litter_fvs( 0.0, U_PGC_YR );
    for( unitval_stringmap::const_iterator it = veg_c.begin(); it != veg_c.end(); it++ ) {
        unitval v = unitval( it->second.value( U_PGC ) * 0.035, U_PGC_YR );
        litter_flux = litter_flux + v;
        litter_fvd = litter_fvd + v * f_litterd;
        litter_fvs = litter_fvs + v * ( 1 - f_litterd );
    }
    
    // Some detritus goes to soil
    unitval detsoil_flux( 0.0, U_PGC_YR );
    for( unitval_stringmap::const_iterator it = detritus_c.begin(); it != detritus_c.end(); it++ ) {
        detsoil_flux = detsoil_flux + unitval( it->second.value( U_PGC ) * 0.6, U_PGC_YR );
    }
    
    // Annual anthropogenic industrial emissions
    unitval anthro_flux_current( 0.0, U_PGC_YR );
    if( !in_spinup ) {   // no perturbation allowed if in spinup
        anthro_flux_current = anthroEmissions.get( t );
    }
    
    // Annual land use change emissions
    unitval luc_current( 0.0, U_PGC_YR );
    if( !in_spinup ) {   // no perturbation allowed if in spinup
        luc_current = lucEmissions.get( t );
    }

    // Land-use change contribution can come from veg, detritus, and soil
    unitval luc_fva = luc_current * f_lucv;
    unitval luc_fda = luc_current * f_lucd;
    unitval luc_fsa = luc_current * ( 1 - f_lucv - f_lucd );
    
    // Oxidized methane of fossil fuel origin
    unitval ch4ox_current( 0.0, U_PGC_YR );     //TODO: implement this
    
    // Compute fluxes
    dcdt[ SNBOX_ATMOS ] = // change in atmosphere pool
        anthro_flux_current.value( U_PGC_YR )
        + luc_current.value( U_PGC_YR )
        + ch4ox_current.value( U_PGC_YR )
        - atmosocean_flux.value( U_PGC_YR )
        - npp_current.value( U_PGC_YR )
        + rh_current.value( U_PGC_YR );
    dcdt[ SNBOX_VEG ] = // change in vegetation pool
        npp_fav.value( U_PGC_YR )
        - litter_flux.value( U_PGC_YR )
        - luc_fva.value( U_PGC_YR );
    dcdt[ SNBOX_DET ] = // change in detritus pool
        npp_fad.value( U_PGC_YR )
        + litter_fvd.value( U_PGC_YR )
        - detsoil_flux.value( U_PGC_YR )
        - rh_fda_current.value( U_PGC_YR )
        - luc_fda.value( U_PGC_YR );
    dcdt[ SNBOX_SOIL ] = // change in soil pool
        npp_fas.value( U_PGC_YR )
        + litter_fvs.value( U_PGC_YR )
        + detsoil_flux.value( U_PGC_YR )
        - rh_fsa_current.value( U_PGC_YR )
        - luc_fsa.value( U_PGC_YR );
    dcdt[ SNBOX_OCEAN ] = // change in ocean pool
        atmosocean_flux.value( U_PGC_YR );
    dcdt[ SNBOX_EARTH ] = // change in earth pool
        - anthro_flux_current.value( U_PGC_YR );

/*    printf( "%6.3f%8.3f%8.2f%8.2f%8.2f%8.2f%8.2f\n", t, dcdt[ SNBOX_ATMOS ],
            dcdt[ SNBOX_VEG ], dcdt[ SNBOX_DET ], dcdt[ SNBOX_SOIL ], dcdt[ SNBOX_OCEAN ], dcdt[ SNBOX_EARTH ] );
*/
    return omodel_err;
}

//------------------------------------------------------------------------------
/*! \brief              Compute 'slowly varying' fluxes
 *  \param[in]  t       time
 *  \param[in]  c       carbon pools (no units)
 *
 *  Compute 'slowly varying' fertilization and anthropogenic fluxes.
 *  Treat the fertilization factors as slowly varying for illustrative purposes
 *  (in fact we could calculate it at each integration step if we wanted to).
 */
void SimpleNbox::slowparameval( double t, const double c[] )
{
    omodel->slowparameval( t, c );      // pass msg on to ocean model
    
	// CO2 fertilization
    Ca.set( c[ SNBOX_ATMOS ] * PGC_TO_PPMVCO2, U_PPMV_CO2 );

    // Compute CO2 fertilization factor globally (and for each biome specified)
    double_stringmap::const_iterator itd;
    for( itd = beta.begin(); itd != beta.end(); itd++ ) {
        if( in_spinup ) {
            co2fert[ itd->first ] = 1.0;  // no perturbation allowed if in spinup
        } else {
            co2fert[ itd->first ] = 1 + beta.at( itd->first ) * log( Ca/C0 );
        }
        H_LOG( logger,Logger::DEBUG ) << "co2fert[ " << itd->first << " ] at " << Ca << " = " << co2fert[ itd->first ] << std::endl;
    }

    // Compute temperature factor globally (and for each biome specified)
    // Heterotrophic respiration depends on the pool sizes (detritus and soil) and Q10 values
    // The soil pool uses a lagged Tgav, i.e. we assume it takes time for heat to diffuse into soil
    const double Tgav = core->sendMessage( M_GETDATA, D_GLOBAL_TEMP ).value( U_DEGC );
    
    // want to set tempferts (soil) and tempfertd (detritus) for each biome
    
    for( itd = tempfertd.begin(); itd != tempfertd.end(); itd++ ) {
        if( in_spinup ) {
            tempfertd[ itd->first ] = 1.0;  // no perturbation allowed in spinup
            tempferts[ itd->first ] = 1.0;  // no perturbation allowed in spinup
        } else {
            double wf = warmingfactor.at( SNBOX_DEFAULT_BIOME );
            if( warmingfactor.count( itd->first ) ) {
                wf = warmingfactor[ itd->first ];   // biome-specific warming
            }
            
            const double Tgav_biome = Tgav * wf;    // biome-specific temperature

            tempfertd[ itd->first ] = pow( q10_rh, ( Tgav_biome / 10.0 ) ); // detritus warms with air
            
        
            // Soil warm very slowly relative to the atmosphere
            // We use a mean temperature of a window (size Q10_TEMPN) of temperatures to scale Q10
            #define Q10_TEMPLAG 0 //125         // TODO: put lag in input files 150, 25
            #define Q10_TEMPN 200 //25
            double Tgav_rm = 0.0;       /* window mean of Tgav */
            if( t > core->getStartDate() + Q10_TEMPLAG ) {
                for( int i=t-Q10_TEMPLAG-Q10_TEMPN; i<t-Q10_TEMPLAG; i++ ) {
 //                   printf( "Fetching temp for %i = %f\n", i, Tgav_record.get( i ) );
                    Tgav_rm += Tgav_record.get( i ) * wf;
                }
                Tgav_rm /= Q10_TEMPN;
            }
            
            tempferts[ itd->first ] = pow( q10_rh, ( Tgav_rm / 10.0 ) );
            
            // The soil Q10 effect is 'sticky' and can only decline very slowly
            if(tempferts[ itd->first ] < m_last_tempferts * 1.0) {
                tempferts[ itd->first ] = m_last_tempferts * 1.0;
            }
            m_last_tempferts = tempferts[ itd->first ];
            
            H_LOG( logger,Logger::DEBUG ) << itd->first << " Tgav=" << Tgav << ", Tgav_biome=" << Tgav_biome << ", tempfertd=" << tempfertd[ itd->first ]
                << ", tempferts=" << tempferts[ itd->first ] << std::endl;
        }
    } // for itd
}

}
