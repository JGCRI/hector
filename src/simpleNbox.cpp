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

// test helper function
void identity_tests(bool trk) {
    string trks = trk ? "true" : "false";
    fluxpool a(1.0, U_UNITLESS, trk);
    H_ASSERT(a == a, "a not equal to itself for " + trks);
    H_ASSERT(a / a == 1.0, "a divided by itself not 1 for " + trks);
    H_ASSERT(a - a == fluxpool(0, U_UNITLESS, trk), "a minus itself not 0 for " + trks);
}

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

    // TODO: Have this read in by csv and set by setData
    trackingYear = 1850;
    // The actual atmos_c value will be filled in later by setData
    atmos_c.set(0.0, U_PGC, false, "atmos_c");
    
    // earth_c keeps track of how much fossil C is pulled out
    // so that we can do a mass-balance check throughout the run
    // 2020-02-05 With the introduction of non-negative 'fluxpool' class
    // we can't start earth_c at zero. Value of 5500 is set to avoid
    // overdrawing in RCP 8.5
    earth_c.set( 5500, U_PGC, false, "earth_c" );

    // TODO: This needs to be read in from the CSV
    ocean_model_c.set( 38000, U_PGC, false, "ocean_c" );
    
    // -----------------------------------------
    // fluxpool test code - move to unit test later BBL-TODO
    // -----------------------------------------
    
    // Construction Tests
    unitval u0(0.0, U_PGC);
    unitval u1(1.0, U_PGC);
    unitval u2(2.0, U_PGC);
    unitval u3 = u1 + u2;

    fluxpool f0;
    std::unordered_map<std::string, double> f0_map = f0.get_tracking_map();
    H_ASSERT(f0.tracking == false, "default constructor tracking isn't false");
    H_ASSERT(f0.value(U_UNDEFINED) == 0.0, "default constructor value and units don't set");
    H_ASSERT(f0_map.empty() == true, "default constructor doesn't create empty map");
    H_ASSERT(f0.name == "?",  "default constructor doesn't set name correctly");

    fluxpool f1(1.0, U_PGC);
    H_ASSERT(f1.tracking == false, "basic constructor tracking isn't default to false");
    H_ASSERT(f1.value(U_PGC) == 1.0, "basic constructor value and units don't set");
    H_ASSERT(f1.name == "?",  "basic constructor doesn't set name correctly");
    std::unordered_map<std::string, double> f1_map = f1.get_tracking_map();
    H_ASSERT(f1_map.size() == 1, "basic constructor-wrong size map");
    fluxpool f1_track = f1; // for ease of testing later
    f1_track.tracking = true;
    H_ASSERT(f1_track.get_fraction("?") == 1, "basic constructor fraction doesn't set correctly");

    fluxpool f2(2.0, U_PGC, false, "f2");
    fluxpool f2_track = f2;
    f2_track.tracking = true;
    H_ASSERT(f2_track.tracking == true, "basic constructor tracking doesn't set correctly");
    H_ASSERT(f2_track.value(U_PGC) == 2.0, "basic constructor value and units don't set");
    H_ASSERT(f2_track.name == "f2",  "basic constructor doesn't set name correctly");
    std::unordered_map<std::string, double> f2_track_map = f2_track.get_tracking_map();
    H_ASSERT(f2_track_map.size() == 1, "basic constructor-wrong size map");
    H_ASSERT(f2_track.get_fraction("f2") == 1, "basic constructor fraction doesn't set correctly");

    // Set Tests
    fluxpool set1;
    set1.set(1, U_PGC);
    H_ASSERT(set1.tracking == false, "set doesn't set tracking correctly");
    H_ASSERT(set1.value(U_PGC) == 1.0, "set value and units don't set");
    H_ASSERT(set1.name == "?",  "set doesn't set name correctly");
    set1.tracking = true;
    std::unordered_map<std::string, double> set1_map = set1.get_tracking_map();
    H_ASSERT(set1_map.size() == 1, "set-wrong size map");
    H_ASSERT(set1.get_fraction("?") == 1, "set fraction doesn't set correctly");

    fluxpool set2;
    set2.set(2.0, U_PGC, true, "set2");
    H_ASSERT(set2.tracking == true, "set doesn't set tracking correct");
    H_ASSERT(set2.name == "set2", "set doesn't set name");
    H_ASSERT(set2.get_fraction("set2") == 1, "set fraction doesn't set correctly with custom name");

    //get_sources and get_fraction tests
    fluxpool empty_pool;
    empty_pool.tracking = true;
    vector<string> no_sources = empty_pool.get_sources();
    H_ASSERT(no_sources.empty() == true, "error on fetching empty list of sources");
    H_ASSERT(empty_pool.get_fraction("") == 0, "doesn't return 0 for strings not in map");

    vector<string> f1_sources = f1_track.get_sources();
    H_ASSERT(f1_sources.size() == 1, "f1 has wrong number of sources");
    vector<string> sources_actual;
    sources_actual.push_back("?"); //f1_track has name of ? from default testing
    H_ASSERT(sources_actual == f1_sources, "set1 sources doesn't match actual sources");
    H_ASSERT(f1_track.get_fraction("?") == 1, "one element get fraction doesn't work");

    fluxpool f3 = f2 + f1;
    fluxpool f3_track = f2_track + f1_track;
    vector<string> f3_sources = f3_track.get_sources();
    sort(f3_sources.begin(), f3_sources.end());
    sources_actual.push_back("f2");
    sort(sources_actual.begin(), sources_actual.end());
    H_ASSERT(sources_actual == f3_sources, "f3 sources doesn't match actual sources");
    H_ASSERT(f3_track.get_fraction("?") == u1/u3, "two element get fraction doesn't work"); 
    H_ASSERT(f3_track.get_fraction("f2") == u2/u3, "two element get fraction doesn't work"); 

    // flux_from_unitval and flux_from_fluxpool tests
    fluxpool flux1 = f1.flux_from_unitval(u1);
    H_ASSERT(flux1 == u1, "flux_from_untival doesn't set value right");
    H_ASSERT(flux1.name == "?", "flux_from_untival default name");
    H_ASSERT(flux1.tracking == f1.tracking, "flux_from_untival tracking when tracking is false");
    H_ASSERT(flux1.get_tracking_map() == f1.get_tracking_map(), "flux_from_untival map not set correctly");
    
    fluxpool flux1_named = f1.flux_from_unitval(u1, "flux1");
    H_ASSERT(flux1_named.name == "flux1", "flux_from_untival set name");

    fluxpool flux1_track = f1_track.flux_from_unitval(u1); 
    H_ASSERT(flux1_track.tracking == f1_track.tracking, "flux_from_untival tracking when tracking is true");
    H_ASSERT(flux1_track.get_tracking_map() == f1_track.get_tracking_map(), "flux_from_untival map not set correctly");

    fluxpool flux3_track = f3_track.flux_from_unitval(u3);
    H_ASSERT(flux3_track.get_tracking_map() == f3_track.get_tracking_map(), "flux_from_untival map not set correctly with multiple keys");

    // Still need to make sure no negative unitvals can make fluxes
    fluxpool flux4_track = flux3_track + flux1_track;
    fluxpool flux4 = flux1.flux_from_fluxpool(flux4_track);
    H_ASSERT(flux4.value(flux4_track.units()) == flux4_track.value(flux4.units()), "flux_from_fluxpool sets value correctly");
    H_ASSERT(flux4.name == "?", "flux_from_fluxpool doesn't set default name");
    H_ASSERT(flux4.get_tracking_map() == flux1.get_tracking_map(), "flux_from_fluxpool map not set correctly");
    H_ASSERT(flux4.tracking == flux1.tracking, "flux_from_fluxpool tracking not set correctly"); 
  
    fluxpool flux4_named = flux4_track.flux_from_fluxpool(flux4, "flux4");
    H_ASSERT(flux4_named.name == "flux4", "flux_from_fluxpool doesn't set name");
    H_ASSERT(flux4_named.tracking == flux4_track.tracking, "flux_from_fluxpool doesn't set tracking when true");
    
    // Adjust Pool To Val tests
    fluxpool adjust2_no_track = f2.flux_from_unitval(u1);
    adjust2_no_track.adjust_pool_to_val(u2.value(u2.units()));
    H_ASSERT(adjust2_no_track == u2, "value is not adjusted correctly");
    adjust2_no_track.tracking = true;
    H_ASSERT(adjust2_no_track.get_fraction("untracked") == 0, "when not tracking, untracked is not added to the map");
    
    fluxpool adjust1_track = f2_track.flux_from_unitval(u2);
    adjust1_track.adjust_pool_to_val(u1.value(u1.units()));
    H_ASSERT(adjust1_track == u1, "value is not adjusted correctly when tracking");
    H_ASSERT(adjust1_track.get_fraction("untracked") == 0, "when tracking with negative diff, untracked is not added to the map");

    fluxpool adjust2_no_untracked = f2_track.flux_from_unitval(u1);
    adjust2_no_untracked.adjust_pool_to_val(u2.value(u2.units()), false);
    H_ASSERT(adjust2_no_untracked == u2, "value is not adjusted correctly");
    H_ASSERT(adjust2_no_untracked.get_fraction("untracked") == 0, "when tracking with negative diff, untracked is not added to the map");

    fluxpool adjust2_track = f1_track.flux_from_unitval(u1);
    adjust2_track.adjust_pool_to_val(u2);
    H_ASSERT(adjust2_track == u2, "value not adjusted correctly when tracking");
    H_ASSERT(adjust2_track.get_fraction("untracked") == (u1/u2), "untracked value not added to map");
    H_EXCEPTION_H

    // Multiplication Tests
    fluxpool mult0r = f1*0.0;
    H_ASSERT(mult0r == u0, "multiplication by 0 does not work");
    H_ASSERT(f1 != u0, "multipiled fluxpool is not affected");
    fluxpool mult0l = 0.0*f1;
    H_ASSERT(mult0l == u0, "multiplication by 0 does not work");

    fluxpool mult4 = f1_track*2.0;
    H_ASSERT(mult4 == u2, "multiplication by 0 does not work");
    H_ASSERT(mult4.get_tracking_map() == f1.get_tracking_map(), "product map same as multipiled map");
    H_ASSERT(mult4.name == f1_track.name, "names aren't preserved during multiplicaiton");
    H_ASSERT(mult4.tracking == f1_track.tracking, "tracking is not preserved during multiplicaiton");
    mult4 = mult4 + f2_track;
    H_ASSERT(mult4.get_tracking_map() != f1_track.get_tracking_map(), "multiplication does not produce a deep copy");
    fluxpool mult3 = mult4 * 0.5;
    // test for map with more than one element
    H_ASSERT(mult3.get_tracking_map() == mult4.get_tracking_map(), "product map same as multipiled map with multiple elements");


    // Division Tests
    fluxpool div1 = f2_track/2.0;
    H_ASSERT(div1 == u1, "division does not work");
    H_ASSERT(div1.get_tracking_map() == f2_track.get_tracking_map(), "quotient map same as multipiled map");
    H_ASSERT(div1.name == f2_track.name, "names aren't preserved during division");
    H_ASSERT(div1.tracking == f2_track.tracking, "tracking is not preserved during multiplicaiton");
    div1 = div1 + f1_track;
    H_ASSERT(div1.get_tracking_map() != f2_track.get_tracking_map(), "division does not produce a deep copy");
    fluxpool div2 = div1/0.5;
    // test for map with more than one element
    H_ASSERT(div2.get_tracking_map() == div2.get_tracking_map(), "quotient map same as divided map with multiple elements");

    // Subtraction Tests with Unitvals
    fluxpool sub_zero = f2 - u0;
    H_ASSERT(sub_zero == f2, "subtraction by zero works");

    fluxpool sub1 = f2 - u1;
    H_ASSERT(sub1 == u1, "subtraction value doesn't work when not tracked");
    H_ASSERT(sub1.get_tracking_map() == f2.get_tracking_map(), "untracked subtraction changes map");
    H_ASSERT(sub1.name == f2.name, "subtraction preserves name");
    H_ASSERT(sub1.tracking == f2.tracking, "subtract preserved tracking");

    fluxpool sub1_track = f2_track - u1;
    H_ASSERT(sub1_track == u1, "subtraction value doesn't work when tracked");
    H_ASSERT(sub1_track.get_tracking_map() == f2_track.get_tracking_map(), "tracked subtraction changes map");
    H_ASSERT(sub1_track.name == f2_track.name, "subtraction preserves name");
    H_ASSERT(sub1_track.tracking == f2_track.tracking, "subtract preserved tracking");

    fluxpool sub2_track = f3_track - u1;
    H_ASSERT(sub2_track.get_tracking_map() == f3_track.get_tracking_map(), "tracked subtraction changes map with multiple keys");

    // Subtraction Tests with Fluxpools
    sub1_track = f2_track - f1_track;
    H_ASSERT(sub1_track == u1, "subtraction value doesn't work when tracked");
    H_ASSERT(sub1_track.get_tracking_map() == f2_track.get_tracking_map(), "tracked subtraction changes map");
    H_ASSERT(sub1_track.name == f2_track.name, "subtraction preserves name");
    H_ASSERT(sub1_track.tracking == f2_track.tracking, "subtract preserved tracking");

    sub2_track = f3_track - f1_track;
    H_ASSERT(sub2_track.get_tracking_map() == f3_track.get_tracking_map(), "tracked subtraction changes map with multiple keys");

    // Addition Tests with unitvals
    fluxpool add2 = f1 + u1;
    H_ASSERT(add2.name == f1.name, "name not preserved in unitval addition");
    H_ASSERT(add2.tracking == f1.tracking, "tracking not preserved in unitval addition");
    H_ASSERT(add2 == u2, "value not preserved in unitval addition");
    fluxpool add4 = f3 + u1;

    // Addition Tests with Fluxpools
    fluxpool add3 = f1 + f2;
    H_ASSERT(add3.name == f1.name, "addition of fluxpools doesn't preserve name when not tracking");
    H_ASSERT(add3.tracking == f1.tracking, "addition of fluxpools doesn't preserve tracking when not tracking");
    H_ASSERT(add3 == u3, "addition of fluxpools doesn't preserve value when not tracking");

    fluxpool add3_track = f1_track + f2_track;
    H_ASSERT(add3_track.name == f1_track.name, "addition of fluxpools doesn't preserve name when tracking");
    H_ASSERT(add3_track.tracking == f1_track.tracking, "addition of fluxpools doesn't preserve tracking when tracking");
    H_ASSERT(add3_track == u3, "addition of fluxpools doesn't preserve value when tracking");
    H_ASSERT(add3_track.get_fraction(f1_track.name) == u1/u3, "addition of fluxpools does not modify map correctly when tracking");
    H_ASSERT(add3_track.get_fraction(f2_track.name) == u2/u3, "addition of fluxpools does not modify map correctly when tracking");


    // Things that should NOT throw an exception
    fluxpool test = f2 - f1;
    test = f1 + f2;
    test = f1 * 2.0;
    test = f1 / 2.0;
    test = f1 - u1;
    test = f1 + u2;

    identity_tests(false);
    
    // Things that SHOULD throw an exception
    int thrown = 0;
    try { fluxpool(-1.0, U_PGC ); } catch ( h_exception e ) { thrown++; }
    try { test.set(-1.0, U_PGC ); } catch ( h_exception e ) { thrown++; }
    try { test = f1 - f2; } catch ( h_exception e ) { thrown++; }
    try { test = f1 - u2; } catch ( h_exception e ) { thrown++; }
    try { test = f1 * -1.0; } catch ( h_exception e ) { thrown++; }
    try { test = -1.0 * f1; } catch ( h_exception e ) { thrown++; }
    try { test = f1 / -1.0; } catch ( h_exception e ) { thrown++; }
    try { if(f1 == fluxpool(1.0, U_UNITLESS)) {}; } catch ( h_exception e ) { thrown++; }
    try { test = f2 - unitval(0.0, U_UNITLESS); } catch ( h_exception e ) { thrown++; }
    try { test = f2 - fluxpool(0.0, U_UNITLESS, true); } catch ( h_exception e ) { thrown++; }
    try { test = f2_track - f1; } catch ( h_exception e ) { thrown++; }
    try { test = f2_track + u1; } catch ( h_exception e ) { thrown++; }
    try { test = f2_track + unitval(0.0, U_UNITLESS); } catch ( h_exception e ) { thrown++; }
    try { test = f2_track + f1; } catch ( h_exception e ) { thrown++; }
    try { test = f2_track + fluxpool(1.0, U_UNITLESS); } catch ( h_exception e ) { thrown++; }
    H_ASSERT(thrown == 15, "1-something didn't throw!")
    
    // Tracking test code
    
    identity_tests(true);

    // Non-tracked objects shouldn't give back tracking info
    fluxpool x(1.0, U_PGC);
    H_ASSERT(x.tracking == false, "default fluxpool shouldn't track");
    thrown = 0;
    try { x.get_sources(); } catch ( h_exception e ) { thrown++; }
    try { x.get_fraction(""); } catch ( h_exception e ) { thrown++; }
    H_ASSERT(thrown == 2, "2-something didn't throw");
    
    fluxpool y(1.0, U_PGC, true, "y");
    H_ASSERT(y.get_fraction("blah blah") == 0.0, "get_fraction wasn't 0");
    H_ASSERT(y.get_fraction("y") == 1.0, "get_fraction wasn't 1");
    vector<string> source = y.get_sources();
    H_ASSERT(source.size() == 1, "source wasn't size 1");
    H_ASSERT(source.at(0) == "y", "source wasn't y");

    // An actual tracking test
    // Test 1: one source, one destination
    fluxpool    src1(10, U_PGC, true, "src1"),
                dest(0.0, U_PGC, true, "dest");
    
    fluxpool flux = src1 * 0.4;
    dest = dest + flux;
    src1 = src1 - flux;

    
    H_ASSERT(dest.get_fraction("src1") == 1.0, "dest fraction src1 wasn't 1");
    source = dest.get_sources();
    H_ASSERT(source.size() == 2, "dest source wasn't size 2");
    H_ASSERT(find(source.begin(), source.end(), "src1") != source.end(), "src wasn't in dest sources");

    // Test 2: two sources, one destination
    
    fluxpool src2(10, U_PGC, true, "src2");
    flux = src2 * 0.6;
    dest = dest + flux;
    src2 = src2 - flux;
    
    H_ASSERT(dest.get_fraction("src1") == 0.4, "get_fraction wasn't 0.4 for src1");
    H_ASSERT(dest.get_fraction("src2") == 0.6, "get_fraction wasn't 0.6 for src2");
    source = dest.get_sources();
    H_ASSERT(source.size() == 3, "dest source wasn't size 3");

    // Test 3: start with a unitval flux and construct fluxpool (neeeded e.g. for LUC)
    
    unitval uflux(1.0, U_PGC);
    flux = src1.flux_from_unitval(uflux);
    H_ASSERT(uflux.value(U_PGC) == flux.value(U_PGC), "flux and uflux values and/or units don't match");
    vector<string> s1 = src1.get_sources(), s2 = flux.get_sources();
    H_ASSERT(s1.size() == s2.size(), "flux and flux source maps not same size");
    H_ASSERT(std::equal(s1.begin(), s1.end(), s2.begin()), "flux and uflux values source maps don't match");
 
    // Test 4: adjusting a total

    dest.adjust_pool_to_val(dest.value(U_PGC) * 1.1);
    H_ASSERT(dest.get_fraction("untracked") - 1.0/11.0 < 1e-6, "untracked not correct");
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
                                const message_data info )
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
                          const message_data& data )
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
            veg_c[ biome ] = fluxpool(data.getUnitval( U_PGC ).value(U_PGC), U_PGC, false, "veg_c_" + biome);
            if (data.date != Core::undefinedIndex()) {
                veg_c_tv.set(data.date, veg_c);
            }
        }
        else if( varNameParsed == D_DETRITUSC ) {
            detritus_c[ biome ] = fluxpool(data.getUnitval( U_PGC ).value(U_PGC), U_PGC, false, "detritus_c_" + biome);
            if (data.date != Core::undefinedIndex()) {
                detritus_c_tv.set(data.date, detritus_c);
            }
        }
        else if( varNameParsed == D_SOILC ) {
            soil_c[ biome ] = fluxpool(data.getUnitval( U_PGC ).value(U_PGC), U_PGC, false, "soil_c_" + biome);
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
    fluxpool sum( 0.0, pool.begin()->second.units(), pool.begin()->second.tracking);
    for( fluxpool_stringmap::const_iterator it = pool.begin(); it != pool.end(); it++ ) {
        H_ASSERT(sum.tracking == (it->second).tracking, "tracking mismatch in sum_map function");
        sum = sum + it->second;
    }
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
                            const double date)
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

void SimpleNbox::reset(double time)
{
    // Reset all state variables to their values at the reset time
    earth_c = earth_c_ts.get(time);
    atmos_c = atmos_c_ts.get(time);
    Ca = Ca_ts.get(time);

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
    C0.set(newc0, U_PPMV_CO2, C0.tracking, C0.name);
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
    veg_c[ biome ] = fluxpool(0, U_PGC, false, "veg_c_"+biome);
    add_biome_to_ts(veg_c_tv, biome, veg_c.at( biome ));
    detritus_c[ biome ] = fluxpool(0, U_PGC, false, "detritus_c_" + biome);
    add_biome_to_ts(detritus_c_tv, biome, detritus_c.at( biome ));
    soil_c[ biome ] = fluxpool(0, U_PGC, false, "soil_c_" + biome);
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
