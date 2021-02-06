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

#include "dependency_finder.hpp"
#include "simpleNbox.hpp"
#include "avisitor.hpp"

#include <algorithm>

namespace Hector {

using namespace boost;

//------------------------------------------------------------------------------
/*! \brief constructor
 */
SimpleNbox::SimpleNbox() : CarbonCycleModel( 6 ), masstot(0.0) {
    ffiEmissions.allowInterp( true );
    ffiEmissions.name = "ffiEmissions";
    lucEmissions.allowInterp( true );
    lucEmissions.name = "lucEmissions";
    Ftalbedo.allowInterp( true );
    Ftalbedo.name = "albedo";
    CO2_constrain.name = "CO2_constrain";

    // earth_c keeps track of how much fossil C is pulled out
    // so that we can do a mass-balance check throughout the run
    // 2020-02-05 With the introduction of non-negative 'fluxpool' class
    // we can't start earth_c at zero. Value of 4000 is from
    // http://globecarboncycle.unh.edu/CarbonCycleBackground.pdf
    earth_c.set( 4000, U_PGC );
    
    // fluxpool test code - move to unit test later BBL-TODO
    fluxpool x1(1.0, U_PGC);
    fluxpool x2(2.0, U_PGC);
    unitval y1(1.0, U_PGC);
    unitval y2(2.0, U_PGC);

    // Things that should NOT throw an exception
    fluxpool test = x2 - x1;
    test = x1 + x2;
    test = x1 * 2.0;
    test = x1 / 2.0;
    test = x1 - y1;
    test = x1 + y2;
    
    // Things that SHOULD throw an exception
    int thrown = 0;
    try { fluxpool(-1.0, U_PGC ); } catch ( h_exception e ) { thrown++; }
    try { test.set(-1.0, U_PGC ); } catch ( h_exception e ) { thrown++; }
    try { test = x1 - x2; } catch ( h_exception e ) { thrown++; }
    try { test = x1 - y2; } catch ( h_exception e ) { thrown++; }
    try { test = x1 * -1.0; } catch ( h_exception e ) { thrown++; }
    try { test = -1.0 * x1; } catch ( h_exception e ) { thrown++; }
    try { test = x1 / -1.0; } catch ( h_exception e ) { thrown++; }

    H_ASSERT(thrown == 7, "Something didn't throw!")
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
    tempfertd[ SNBOX_DEFAULT_BIOME ] = 1.0;
    tempferts[ SNBOX_DEFAULT_BIOME ] = 1.0;

    // Initialize the `biome_list` with just "global"
    biome_list.push_back( SNBOX_DEFAULT_BIOME );

    Tgav_record.allowInterp( true );

    // Register the data we can provide
    core->registerCapability( D_ATMOSPHERIC_CO2, getComponentName() );
    core->registerCapability( D_ATMOSPHERIC_C, getComponentName() );
    core->registerCapability( D_PREINDUSTRIAL_CO2, getComponentName() );
    core->registerCapability( D_RF_T_ALBEDO, getComponentName() );
    core->registerCapability( D_LAND_CFLUX, getComponentName() );
    core->registerCapability( D_VEGC, getComponentName() );
    core->registerCapability( D_DETRITUSC, getComponentName() );
    core->registerCapability( D_SOILC, getComponentName() );
    core->registerCapability( D_NPP_FLUX0, getComponentName() );
    core->registerCapability( D_NPP, getComponentName() );

    // Register our dependencies
    core->registerDependency( D_OCEAN_CFLUX, getComponentName() );

    // Register the inputs we can receive from outside
    core->registerInput(D_FFI_EMISSIONS, getComponentName());
    core->registerInput(D_LUC_EMISSIONS, getComponentName());
    core->registerInput(D_PREINDUSTRIAL_CO2, getComponentName());
    core->registerInput(D_VEGC, getComponentName());
    core->registerInput(D_DETRITUSC, getComponentName());
    core->registerInput(D_SOILC, getComponentName());
    core->registerInput(D_NPP_FLUX0, getComponentName());
    core->registerInput(D_WARMINGFACTOR, getComponentName());
    core->registerInput(D_BETA, getComponentName());
    core->registerInput(D_Q10_RH, getComponentName());
    core->registerInput(D_F_NPPV, getComponentName());
    core->registerInput(D_F_NPPD, getComponentName());
    core->registerInput(D_F_LITTERD, getComponentName());
    core->registerInput(D_F_LUCV, getComponentName());
    core->registerInput(D_F_LUCD, getComponentName());
    core->registerInput(D_CO2_CONSTRAIN, getComponentName());
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
    auto it_global = std::find(biome_list.begin(), biome_list.end(), SNBOX_DEFAULT_BIOME);

    if( splitvec.size() == 2 ) {    // i.e., in form <biome>.<varname>
        biome = splitvec[ 0 ];
        varNameParsed = splitvec[ 1 ];
        if ( has_biome( SNBOX_DEFAULT_BIOME ) ) {
            H_LOG( logger, Logger::DEBUG ) << "Removing biome '" << SNBOX_DEFAULT_BIOME <<
                "' because you cannot have both 'global' and biome data. " << std::endl;
            // We don't use the `deleteBiome` function here because
            // when `setData` is used to initialize the core from the
            // INI file, most of the time series variables that
            // `deleteBiome` modifies have not been initialized yet.
            // This should be relatively safe because (1) we check
            // consistency of biome-specific variable sizes before
            // running, and (2) the R interface will not let you use
            // `setData` to modify the biome list.
            biome_list.erase( it_global );
        }
    }

    H_ASSERT( !(it_global != biome_list.end() && biome != SNBOX_DEFAULT_BIOME),
              "If one of the biomes is 'global', you cannot add other biomes." );

    // If the biome is not currently in the `biome_list`, and it's not
    // the "global" biome, add it to `biome_list`
    if ( biome != SNBOX_DEFAULT_BIOME && !has_biome( biome ) ) {
        H_LOG( logger, Logger::DEBUG ) << "Adding biome '" << biome << "' to `biome_list`." << std::endl;
        // We don't use `createBiome` here for the same reasons as above.
        biome_list.push_back( biome );
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
            // Hector input files specify initial atmospheric CO2 in terms of
            // the carbon pool, rather than the CO2 concentration.  Since we
            // don't have a place to store the initial carbon pool, we convert
            // it to initial concentration and store that.  It will be converted
            // back to carbon content when the state variables are set up in
            // prepareToRun.
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            H_ASSERT( biome == SNBOX_DEFAULT_BIOME, "atmospheric C must be global" );
            set_c0(data.getUnitval(U_PGC).value(U_PGC) * PGC_TO_PPMVCO2);
        }
        else if( varNameParsed == D_PREINDUSTRIAL_CO2 ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            H_ASSERT( biome == SNBOX_DEFAULT_BIOME, "preindustrial C must be global" );
            set_c0(data.getUnitval(U_PPMV_CO2).value(U_PPMV_CO2));
        }
        else if( varNameParsed == D_VEGC ) {
            // For `veg_c`, `detritus_c`, and `soil_c`, if date is not
            // provided, set only the "current" model pool, without
            // touching the time series variable. This is to
            // accommodate the way the INI file is parsed. For
            // interactive use, you will usually want to pass the date
            // -- otherwise, the current value will be overridden by a
            // `reset` (which includes code like `veg_c = veg_c_tv.get(t)`).
            
            // Data are coming in as unitvals, but change to fluxpools
            veg_c[ biome ] = fluxpool(data.getUnitval( U_PGC ).value(U_PGC), U_PGC);
            if (data.date != Core::undefinedIndex()) {
                veg_c_tv.set(data.date, veg_c);
            }
        }
        else if( varNameParsed == D_DETRITUSC ) {
            detritus_c[ biome ] = fluxpool(data.getUnitval( U_PGC ).value(U_PGC), U_PGC);
            if (data.date != Core::undefinedIndex()) {
                detritus_c_tv.set(data.date, detritus_c);
            }
        }
        else if( varNameParsed == D_SOILC ) {
            soil_c[ biome ] = fluxpool(data.getUnitval( U_PGC ).value(U_PGC), U_PGC);
            if (data.date != Core::undefinedIndex()) {
                soil_c_tv.set(data.date, soil_c);
            }
        }

        // Albedo effect
        else if( varNameParsed == D_RF_T_ALBEDO ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            Ftalbedo.set( data.date, data.getUnitval( U_W_M2 ) );
        }

        // Partitioning
        else if( varNameParsed == D_F_NPPV ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            f_nppv[ biome ] = data.getUnitval(U_UNITLESS);
        }
        else if( varNameParsed == D_F_NPPD ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            f_nppd[ biome ] = data.getUnitval(U_UNITLESS);
        }
        else if( varNameParsed == D_F_LITTERD ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            f_litterd[ biome ] = data.getUnitval(U_UNITLESS);
        }
        else if( varNameParsed == D_F_LUCV ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            f_lucv = data.getUnitval(U_UNITLESS);
        }
        else if( varNameParsed == D_F_LUCD ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            f_lucd = data.getUnitval(U_UNITLESS);
        }

        // Initial fluxes
        else if( varNameParsed == D_NPP_FLUX0 ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            npp_flux0[ biome ] = fluxpool(data.getUnitval( U_PGC_YR ).value( U_PGC_YR ), U_PGC_YR);
        }

        // Fossil fuels and industry contributions time series
        else if( varNameParsed == D_FFI_EMISSIONS ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            H_ASSERT( biome == SNBOX_DEFAULT_BIOME, "fossil fuels and industry emissions must be global" );
            ffiEmissions.set( data.date, data.getUnitval( U_PGC_YR ) );
        }
        else if( varNameParsed == D_LUC_EMISSIONS ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            lucEmissions.set( data.date, data.getUnitval( U_PGC_YR ) );
        }
        // Atmospheric CO2 record to constrain model to (optional)
        else if( varNameParsed == D_CO2_CONSTRAIN ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            H_ASSERT( biome == SNBOX_DEFAULT_BIOME, "atmospheric constraint must be global" );
            unitval co2c = data.getUnitval( U_PPMV_CO2 );
            CO2_constrain.set( data.date, fluxpool( co2c.value( U_PPMV_CO2 ), U_PPMV_CO2 ) );
        }

        // Fertilization
        else if( varNameParsed == D_BETA ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            beta[ biome ] = data.getUnitval(U_UNITLESS);
        }
        else if( varNameParsed == D_WARMINGFACTOR ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            warmingfactor[ biome ] = data.getUnitval(U_UNITLESS);
        }
        else if( varNameParsed == D_Q10_RH ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            q10_rh[ biome ] = data.getUnitval(U_UNITLESS);
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
/*! \brief      Sum a string->unitval map
 *  \param      pool to sum over
 *  \returns    Sum of the unitvals in the map
 *  \exception  If the map is empty
 */
fluxpool SimpleNbox::sum_map( fluxpool_stringmap pool ) const
{
    H_ASSERT( pool.size(), "can't sum an empty map" );
    fluxpool sum( 0.0, pool.begin()->second.units() );
    for( fluxpool_stringmap::const_iterator it = pool.begin(); it != pool.end(); it++ )
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
// documentation is inherited
unitval SimpleNbox::getData(const std::string& varName,
                            const double date) throw ( h_exception )
{
    unitval returnval;

    std::string biome = SNBOX_DEFAULT_BIOME;
    std::string varNameParsed = varName;
    std::string biome_error = "Biome '" + biome + "' missing from biome list. " +
        "Hit this error while trying to retrieve variable: '" + varName + "'.";

    // Does the varName contain our parse character? If so, split it
    std::vector<std::string> splitvec;
    boost::split( splitvec, varName, boost::is_any_of( SNBOX_PARSECHAR ) );
    H_ASSERT( splitvec.size() < 3, "max of one separator allowed in variable names" );

    if( splitvec.size() == 2 ) {    // i.e., in form <biome>.<varname>
        biome = splitvec[ 0 ];
        varNameParsed = splitvec[ 1 ];
        // Have to re-set this here because `biome` has changed
        biome_error = "Biome '" + biome + "' missing from biome list. " +
            "Hit this error while trying to retrieve variable: '" + varName + "'.";
    }

    if( varNameParsed == D_ATMOSPHERIC_C ) {
        if(date == Core::undefinedIndex())
            returnval = atmos_c;
        else
            returnval = atmos_c_ts.get(date);
    } else if( varNameParsed == D_ATMOSPHERIC_CO2 ) {
        if(date == Core::undefinedIndex())
            returnval = Ca;
        else
            returnval = Ca_ts.get(date);
    } else if( varNameParsed == D_ATMOSPHERIC_C_RESIDUAL ) {
        if(date == Core::undefinedIndex())
            returnval = residual;
        else
            returnval = residual_ts.get(date);
    } else if( varNameParsed == D_PREINDUSTRIAL_CO2 ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for preindustrial CO2" );
        returnval = C0;
    } else if(varNameParsed == D_WARMINGFACTOR) {
        H_ASSERT(date == Core::undefinedIndex(), "Date not allowed for biome warming factor");
        H_ASSERT(has_biome( biome ), biome_error);
        returnval = unitval(warmingfactor.at(biome), U_UNITLESS);
    } else if(varNameParsed == D_BETA) {
        H_ASSERT(date == Core::undefinedIndex(), "Date not allowed for CO2 fertilization (beta)");
        H_ASSERT(has_biome( biome ), biome_error);
        returnval = unitval(beta.at(biome), U_UNITLESS);
    } else if(varNameParsed == D_Q10_RH) {
        H_ASSERT(date == Core::undefinedIndex(), "Date not allowed for Q10");
        returnval = unitval(q10_rh.at( biome ), U_UNITLESS);
    } else if( varNameParsed == D_LAND_CFLUX ) {
        if(date == Core::undefinedIndex())
            returnval = atmosland_flux;
        else
            returnval = atmosland_flux_ts.get( date );
    } else if( varNameParsed == D_RF_T_ALBEDO ) {
        H_ASSERT( date != Core::undefinedIndex(), "Date required for albedo forcing" );
        returnval = Ftalbedo.get( date );

        // Partitioning parameters.
    } else if(varNameParsed == D_F_NPPV) {
        H_ASSERT(date == Core::undefinedIndex(), "Date not allowed for vegetation NPP fraction");
        returnval = unitval(f_nppv.at( biome ), U_UNITLESS);
    } else if(varNameParsed == D_F_NPPD) {
        H_ASSERT(date == Core::undefinedIndex(), "Date not allowed for detritus NPP fraction");
        returnval = unitval(f_nppd.at( biome ), U_UNITLESS);
    } else if(varNameParsed == D_F_LITTERD) {
        H_ASSERT(date == Core::undefinedIndex(), "Date not allowed for litter-detritus fraction");
        returnval = unitval(f_litterd.at( biome ), U_UNITLESS);
    } else if(varNameParsed == D_F_LUCV) {
        H_ASSERT(date == Core::undefinedIndex(), "Date not allowed for LUC vegetation fraction");
        returnval = unitval(f_lucv, U_UNITLESS);
    } else if(varNameParsed == D_F_LUCD) {
        H_ASSERT(date == Core::undefinedIndex(), "Date not allowed for LUC detritus fraction");
        returnval = unitval(f_lucd, U_UNITLESS);

    } else if( varNameParsed == D_EARTHC ) {
        if(date == Core::undefinedIndex())
            returnval = earth_c;
        else
            returnval = earth_c_ts.get(date);
    } else if( varNameParsed == D_VEGC ) {
        if(biome == SNBOX_DEFAULT_BIOME) {
            if(date == Core::undefinedIndex())
                returnval = sum_map( veg_c );
            else
                returnval = sum_map(veg_c_tv.get(date));
        } else {
            H_ASSERT(has_biome( biome ), biome_error);
            if(date == Core::undefinedIndex())
                returnval = veg_c.at(biome) ;
            else
                returnval = veg_c_tv.get(date).at(biome);
        }
    } else if( varNameParsed == D_DETRITUSC ) {
        if(biome == SNBOX_DEFAULT_BIOME) {
            if(date == Core::undefinedIndex())
                returnval = sum_map( detritus_c );
            else
                returnval = sum_map(detritus_c_tv.get(date));
        } else {
            H_ASSERT(has_biome( biome ), biome_error);
            if(date == Core::undefinedIndex())
                returnval = detritus_c.at(biome) ;
            else
                returnval = detritus_c_tv.get(date).at(biome);
        }
    } else if( varNameParsed == D_SOILC ) {
        if(biome == SNBOX_DEFAULT_BIOME) {
            if(date == Core::undefinedIndex())
                returnval = sum_map( soil_c );
            else
                returnval = sum_map(soil_c_tv.get(date));
        } else {
            H_ASSERT(has_biome( biome ), biome_error);
            if(date == Core::undefinedIndex())
                returnval = soil_c.at(biome);
            else
                returnval = soil_c_tv.get(date).at(biome);
        }
    } else if( varNameParsed == D_NPP_FLUX0 ) {
      H_ASSERT(date == Core::undefinedIndex(), "Date not allowed for npp_flux0" );
      H_ASSERT(has_biome( biome ), biome_error);
      returnval = npp_flux0.at(biome);
    } else if( varNameParsed == D_FFI_EMISSIONS ) {
        H_ASSERT( date != Core::undefinedIndex(), "Date required for ffi emissions" );
        returnval = ffiEmissions.get( date );
    } else if( varNameParsed == D_LUC_EMISSIONS ) {
        H_ASSERT( date != Core::undefinedIndex(), "Date required for luc emissions" );
        returnval = lucEmissions.get( date );
    } else if( varNameParsed == D_CO2_CONSTRAIN ) {
        H_ASSERT( date != Core::undefinedIndex(), "Date required for atmospheric CO2 constraint" );
        if (CO2_constrain.exists(date)) {
            returnval = CO2_constrain.get( date );
        } else {
            H_LOG( logger, Logger::DEBUG ) << "No CO2 constraint for requested date " << date <<
                ". Returning missing value." << std::endl;
            returnval = unitval( MISSING_FLOAT, U_PPMV_CO2 );
        }
    } else if( varNameParsed == D_NPP ) {
        // `sum_npp` works whether or not `date` is defined (if undefined, it
        // evaluates for the current date).
        returnval = sum_npp(date);
    } else if( varNameParsed == D_RH ) {
        H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for rh" );
        returnval = sum_rh();
    }else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }

    // returnval might be a unitval or a fluxpool; need to return only the former
    return static_cast<unitval>(returnval);
}

void SimpleNbox::reset(double time) throw(h_exception)
{
    // Reset all state variables to their values at the reset time
    earth_c.set( earth_c_ts.get(time).value( U_PGC ), U_PGC );
    atmos_c.set( atmos_c_ts.get(time).value( U_PGC ), U_PGC );
    Ca.set( Ca_ts.get(time).value( U_PPMV_CO2 ), U_PPMV_CO2 );

    veg_c = veg_c_tv.get(time);
    detritus_c = detritus_c_tv.get(time);
    soil_c = soil_c_tv.get(time);

    residual = residual_ts.get(time);

    tempferts = tempferts_tv.get(time);
    tempfertd = tempfertd_tv.get(time);

    // Calculate derived quantities
    for( auto it = biome_list.begin(); it != biome_list.end(); it++ ) {
        std::string biome = *it;
        if(in_spinup) {
            co2fert[ biome ] = 1.0; // co2fert fixed if in spinup.  Placeholder in case we decide to allow resetting into spinup
        }
        else {
            co2fert[ biome ] = calc_co2fert(biome);
        }
    }
    Tgav_record.truncate(time);
    // No need to reset masstot; it's not supposed to change anyhow.

    // Truncate all of the state variable time series
    earth_c_ts.truncate(time);
    atmos_c_ts.truncate(time);
    Ca_ts.truncate(time);

    veg_c_tv.truncate(time);
    detritus_c_tv.truncate(time);
    soil_c_tv.truncate(time);

    residual_ts.truncate(time);

    tempferts_tv.truncate(time);
    tempfertd_tv.truncate(time);

    tcurrent = time;

    H_LOG(logger, Logger::NOTICE)
        << getComponentName() << " reset to time= " << time << "\n";
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

void SimpleNbox::record_state(double t)
{
    tcurrent = t;
    earth_c_ts.set(t, earth_c);
    atmos_c_ts.set(t, atmos_c);
    Ca_ts.set(t, Ca);

    veg_c_tv.set(t, veg_c);
    detritus_c_tv.set(t, detritus_c);
    soil_c_tv.set(t, soil_c);

    residual_ts.set(t, residual);

    tempfertd_tv.set(t, tempfertd);
    tempferts_tv.set(t, tempferts);
    H_LOG(logger, Logger::DEBUG) << "record_state: recorded tempferts = " << tempferts[SNBOX_DEFAULT_BIOME]
                                 << " at time= " << t << std::endl;

    // ocean model appears to be controlled by the N-box model.  Seems
    // like it makes swapping out for another model a nightmare, but
    // that's where we're at.
    omodel->record_state(t);

}

// Set the preindustrial carbon value and adjust total mass to reflect the new
// value (unless it hasn't yet been set).  Note that after doing this,
// attempting to run without first doing a reset will cause an exception due to
// failure to conserve mass.
void SimpleNbox::set_c0(double newc0)
{
    if(masstot > 0.0) {
        double massdiff = (newc0 - C0) * PPMVCO2_TO_PGC;
        masstot += massdiff;
        H_LOG(logger, Logger::DEBUG) << "massdiff= " << massdiff << "  new masstot= " << masstot
                                     << "\n";
    }
    C0.set(newc0, U_PPMV_CO2);

}

// Check if `biome` is present in biome_list
bool SimpleNbox::has_biome(const std::string& biome) {
    return std::find(biome_list.begin(), biome_list.end(), biome) != biome_list.end();
}

// Create a new biome, and initialize it with zero C pools and fluxes
// and the same parameters as the most recently created biome.
void SimpleNbox::createBiome(const std::string& biome)
{

    H_LOG(logger, Logger::DEBUG) << "Creating new biome '" << biome << "'." << std::endl;

    // Throw an error if the biome already exists
    std::string errmsg = "Biome '" + biome + "' is already in `biome_list`.";
    H_ASSERT(!has_biome( biome ), errmsg);

    // Initialize new pools
    veg_c[ biome ] = fluxpool(0, U_PGC);
    add_biome_to_ts(veg_c_tv, biome, veg_c.at( biome ));
    detritus_c[ biome ] = fluxpool(0, U_PGC);
    add_biome_to_ts(detritus_c_tv, biome, detritus_c.at( biome ));
    soil_c[ biome ] = fluxpool(0, U_PGC);
    add_biome_to_ts(soil_c_tv, biome, soil_c.at( biome ));

    npp_flux0[ biome ] = fluxpool(0, U_PGC_YR);

    // Other defaults (these will be re-calculated later)
    co2fert[ biome ] = 1.0;
    tempfertd[ biome ] = 1.0;
    add_biome_to_ts(tempfertd_tv, biome, 1.0);
    tempferts[ biome ] = 1.0;
    add_biome_to_ts(tempferts_tv, biome, 1.0);

    std::string last_biome = biome_list.back();

    // Set parameters to same as most recent biome
    beta[ biome ] = beta[ last_biome ];
    q10_rh[ biome ] = q10_rh[ last_biome ];
    warmingfactor[ biome ] = warmingfactor[ last_biome ];
    f_nppv[ biome ] = f_nppv[ last_biome ];
    f_nppd[ biome ] = f_nppd[ last_biome ];
    f_litterd[ biome ] = f_litterd[ last_biome ];

    // Add to end of biome list
    biome_list.push_back(biome);

    H_LOG(logger, Logger::DEBUG) << "Finished creating biome '" << biome << "'." << std::endl;}

// Delete a biome: Remove it from the `biome_list` and `erase` all of
// the associated parameters.
void SimpleNbox::deleteBiome(const std::string& biome) // Throw an error if the biome already exists
{

    H_LOG(logger, Logger::DEBUG) << "Deleting biome '" << biome << "'." << std::endl;

    std::string errmsg = "Biome '" + biome + "' not found in `biome_list`.";
    std::vector<std::string>::const_iterator i_biome = std::find(biome_list.begin(), biome_list.end(), biome);
    H_ASSERT(has_biome( biome ), errmsg);

    // Erase all values associated with the biome:
    // Parameters
    beta.erase( biome );
    q10_rh.erase(biome);
    warmingfactor.erase( biome );
    f_nppv.erase(biome);
    f_nppd.erase(biome);
    f_litterd.erase(biome);

    // C pools
    veg_c.erase( biome );
    remove_biome_from_ts(veg_c_tv, biome);
    detritus_c.erase( biome );
    remove_biome_from_ts(detritus_c_tv, biome);
    soil_c.erase( biome );
    remove_biome_from_ts(soil_c_tv, biome);

    // Others
    npp_flux0.erase( biome );
    tempfertd.erase( biome );
    remove_biome_from_ts( tempfertd_tv, biome );
    tempferts.erase( biome );
    remove_biome_from_ts( tempferts_tv, biome );
    co2fert.erase( biome );

    // Remove from `biome_list`
    biome_list.erase( i_biome );

    H_LOG(logger, Logger::DEBUG) << "Finished deleting biome '" << biome << ",." << std::endl;

}

// Create a new biome called `newname`, transfer all of the parameters
// and pools from `oldname`, and delete `oldname`. Note that the new
// biome will be at the end of the `biome_list`.
void SimpleNbox::renameBiome(const std::string& oldname, const std::string& newname)
{
    H_LOG(logger, Logger::DEBUG) << "Renaming biome '" << oldname <<
        "' to '" << newname << "'." << std::endl;

    std::string errmsg = "Biome '" + oldname + "' not found in `biome_list`.";
    H_ASSERT(has_biome( oldname ), errmsg);
    errmsg = "Biome '" + newname + "' already exists in `biome_list`.";
    H_ASSERT(!has_biome( newname ), errmsg);

    beta[ newname ] = beta.at( oldname );
    beta.erase(oldname);
    q10_rh[ newname ] = q10_rh.at( oldname );
    q10_rh.erase(oldname);
    warmingfactor[ newname ] = warmingfactor.at( oldname );
    warmingfactor.erase( oldname );
    f_nppv[ newname ] = f_nppv.at( oldname );
    f_nppv.erase(oldname);
    f_nppd[ newname ] = f_nppd.at( oldname );
    f_nppd.erase(oldname);
    f_litterd[ newname ] = f_litterd.at( oldname );
    f_litterd.erase(oldname);

    H_LOG(logger, Logger::DEBUG) << "Transferring C from biome '" << oldname <<
        "' to '" << newname << "'." << std::endl;

    // Transfer all C from `oldname` to `newname`
    veg_c[ newname ] = veg_c.at( oldname );
    veg_c.erase(oldname);
    rename_biome_in_ts(veg_c_tv, oldname, newname);
    detritus_c[ newname ] = detritus_c.at( oldname );
    detritus_c.erase(oldname);
    rename_biome_in_ts(detritus_c_tv, oldname, newname);
    soil_c[ newname ] = soil_c.at( oldname );
    soil_c.erase(oldname);
    rename_biome_in_ts(soil_c_tv, oldname, newname);

    npp_flux0[ newname ] = npp_flux0.at( oldname );
    npp_flux0.erase(oldname);

    co2fert[ newname ] = co2fert[ oldname ];
    co2fert.erase( oldname );

    tempfertd[ newname ] = tempfertd[ oldname ];
    tempfertd.erase(oldname);
    tempferts[ newname ] = tempferts[ oldname ];
    tempferts.erase(oldname);

    biome_list.push_back(newname);
    biome_list.erase(std::find(biome_list.begin(), biome_list.end(), oldname));

    H_LOG(logger, Logger::DEBUG) << "Done renaming biome '" << oldname <<
        "' to '" << newname << "'." << std::endl;

}

}
