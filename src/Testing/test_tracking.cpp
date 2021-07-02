/* Hector -- A Simple Climate Model
 Copyright (C) 2014-2015  Battelle Memorial Institute
 
 Please see the accompanying file LICENSE.md for additional licensing
 information.
 */
/*
 *  test_tracking.cpp
 *  hector
 *
 *  Created by Ben on 2021-07-02.
 *
 */

#include <iostream>
#include <gtest/gtest.h>
#include <fstream>

#include "unitval.hpp"
#include "fluxpool.hpp"
#include "h_exception.hpp"

using namespace Hector;

// test helper function
void identity_tests(bool trk) {
    string trks = trk ? "true" : "false";
    fluxpool a(1.0, U_UNITLESS, trk);
    EXPECT_EQ(a, a); // "a not equal to itself"
    EXPECT_EQ(a / a, 1.0); // "a divided by itself not 1"
    EXPECT_EQ(a - a, fluxpool(0, U_UNITLESS, trk)); // "a minus itself not 0"
}

TEST( TestTracking, Construction ) {
    
    // Default constructor
    fluxpool f0;
    std::unordered_map<std::string, double> f0_map = f0.get_tracking_map();
    EXPECT_EQ(f0.tracking, false); // "default constructor tracking isn't false"
    EXPECT_EQ(f0.value(U_UNDEFINED), 0.0); // "default constructor value and units don't set"
    EXPECT_TRUE(f0_map.empty()); // "default constructor doesn't create empty map"
    EXPECT_EQ(f0.name, "?"); //  "default constructor doesn't set name correctly"
    
    // 4 argument constructor with default values
    fluxpool f1(1.0, U_PGC);
    EXPECT_FALSE(f1.tracking); // "basic constructor tracking isn't default to false"
    EXPECT_EQ(f1.value(U_PGC), 1.0); // "basic constructor value and units don't set"
    EXPECT_EQ(f1.name, "?"); //  "basic constructor doesn't set name correctly"
    
    std::unordered_map<std::string, double> f1_map = f1.get_tracking_map();
    EXPECT_EQ(f1_map.size(), 1); // "basic constructor-wrong size map"
    fluxpool f1_track = f1; // for ease of testing later
    f1_track.tracking = true;
    EXPECT_EQ(f1_track.get_fraction("?"), 1); // "basic constructor fraction doesn't set correctly"
    
    // 4 argument constructor
    fluxpool f2(2.0, U_PGC, false, "f2");
    fluxpool f2_track = f2;
    f2_track.tracking = true;
    EXPECT_TRUE(f2_track.tracking); // "basic constructor tracking doesn't set correctly"
    EXPECT_EQ(f2_track.value(U_PGC), 2.0); // "basic constructor value and units don't set"
    EXPECT_EQ(f2_track.name, "f2"); // "basic constructor doesn't set name correctly"
    std::unordered_map<std::string, double> f2_track_map = f2_track.get_tracking_map();
    EXPECT_EQ(f2_track_map.size(), 1); // "basic constructor-wrong size map"
    EXPECT_EQ(f2_track.get_fraction("f2"), 1); // "basic constructor fraction doesn't set correctly"
}

TEST( TestTracking, Setting ) {
    // Basic set test
    fluxpool set1;
    set1.set(1, U_PGC);
    EXPECT_FALSE(set1.tracking); // "set doesn't set tracking correctly"
    EXPECT_EQ(set1.value(U_PGC), 1.0); // "set value and units don't set"
    EXPECT_EQ(set1.name, "?"); //  "set doesn't set name correctly"
    set1.tracking = true;
    std::unordered_map<std::string, double> set1_map = set1.get_tracking_map();
    EXPECT_EQ(set1_map.size(), 1); // "set-wrong size map"
    EXPECT_EQ(set1.get_fraction("?"), 1); // "set fraction doesn't set correctly"
    
    // Set test with tracking and name specified
    fluxpool set2;
    set2.set(2.0, U_PGC, true, "set2");
    EXPECT_TRUE(set2.tracking); // "set doesn't set tracking correct"
    EXPECT_EQ(set2.name, "set2"); // "set doesn't set name"
    EXPECT_EQ(set2.get_fraction("set2"), 1); // "set fraction doesn't set correctly with custom name"
}

TEST( TestTracking, Getting ) {
    // Sources and fractions from an empty map
    fluxpool empty_pool;
    empty_pool.tracking = true;
    vector<string> no_sources = empty_pool.get_sources();
    EXPECT_TRUE(no_sources.empty()); // "error on fetching empty list of sources"
    EXPECT_EQ(empty_pool.get_fraction(""), 0); // "doesn't return 0 for strings not in map"
    
    // Sources and fractions from a map with one key
    fluxpool f1(1.0, U_PGC);
    fluxpool f1_track = f1; // for ease of testing later
    f1_track.tracking = true;
    
    vector<string> f1_sources = f1_track.get_sources();
    EXPECT_EQ(f1_sources.size(), 1); // "f1 has wrong number of sources"
    vector<string> sources_actual;
    sources_actual.push_back("?"); //f1_track has name of ? from default testing
    EXPECT_EQ(sources_actual, f1_sources); // "set1 sources doesn't match actual sources"
    EXPECT_EQ(f1_track.get_fraction("?"), 1); // "one element get fraction doesn't work"
    
    // Sources and fractions from a map with multiple keys
    fluxpool f2(2.0, U_PGC, false, "f2");
    fluxpool f2_track = f2;
    f2_track.tracking = true;
    
    fluxpool f3 = f2 + f1;
    fluxpool f3_track = f2_track + f1_track;
    
    unitval u0(0.0, U_PGC);
    unitval u1(1.0, U_PGC);
    unitval u2(2.0, U_PGC);
    unitval u3 = u1 + u2;
    
    vector<string> f3_sources = f3_track.get_sources();
    sort(f3_sources.begin(), f3_sources.end());
    sources_actual.push_back("f2");
    sort(sources_actual.begin(), sources_actual.end());
    EXPECT_EQ(sources_actual, f3_sources); // "f3 sources doesn't match actual sources"
    EXPECT_EQ(f3_track.get_fraction("?"), u1/u3); // "two element get fraction doesn't work"
    EXPECT_EQ(f3_track.get_fraction("f2"), u2/u3); // "two element get fraction doesn't work"
}

TEST( TestTracking, FluxFrom ) {
    // Flux_from_unitval and flux_from_fluxpool tests
    
    unitval u1(1.0, U_PGC);
    unitval u2(2.0, U_PGC);
    unitval u3 = u1 + u2;
    
    fluxpool f1(1.0, U_PGC);
    fluxpool f1_track = f1; // for ease of testing later
    f1_track.tracking = true;
    
    fluxpool f2(2.0, U_PGC, false, "f2");
    fluxpool f2_track = f2;
    f2_track.tracking = true;
    
    fluxpool f3 = f2 + f1;
    fluxpool f3_track = f2_track + f1_track;
    
    // Basic flux from unitval test
    fluxpool flux1 = f1.flux_from_unitval(u1);
    EXPECT_EQ(flux1, u1); // "flux_from_untival doesn't set value right"
    EXPECT_EQ(flux1.name, "?"); // "flux_from_untival default name"
    EXPECT_EQ(flux1.tracking, f1.tracking); // "flux_from_untival tracking when tracking is false"
    EXPECT_EQ(flux1.get_tracking_map(), f1.get_tracking_map()); // "flux_from_untival map not set correctly"
    
    // Flux from unitval test named flux test
    fluxpool flux1_named = f1.flux_from_unitval(u1, "flux1");
    EXPECT_EQ(flux1_named.name, "flux1"); // "flux_from_untival set name"
    
    // Flux from unitval test when tracking - one key
    fluxpool flux1_track = f1_track.flux_from_unitval(u1);
    EXPECT_EQ(flux1_track.tracking, f1_track.tracking); // "flux_from_untival tracking when tracking is true"
    EXPECT_EQ(flux1_track.get_tracking_map(), f1_track.get_tracking_map()); // "flux_from_untival map not set correctly"
    
    // Flux from unitval test when tracking - muliple keys
    fluxpool flux3_track = f3_track.flux_from_unitval(u3);
    EXPECT_EQ(flux3_track.get_tracking_map(), f3_track.get_tracking_map()); // "flux_from_untival map not set correctly with multiple keys"
    
    // Flux from fluxpool test - untracked and unnamed
    fluxpool flux4_track = flux3_track + flux1_track;
    fluxpool flux4 = flux1.flux_from_fluxpool(flux4_track);
    EXPECT_EQ(flux4.value(flux4_track.units()), flux4_track.value(flux4.units())); // "flux_from_fluxpool sets value correctly"
    EXPECT_EQ(flux4.name, "?"); // "flux_from_fluxpool doesn't set default name"
    EXPECT_EQ(flux4.get_tracking_map(), flux1.get_tracking_map()); // "flux_from_fluxpool map not set correctly"
    EXPECT_EQ(flux4.tracking, flux1.tracking); // "flux_from_fluxpool tracking not set correctly"
    
    // Flux from fluxpool test - tracked and named
    fluxpool flux4_named = flux4_track.flux_from_fluxpool(flux4, "flux4");
    EXPECT_EQ(flux4_named.name, "flux4"); // "flux_from_fluxpool doesn't set name"
    EXPECT_EQ(flux4_named.tracking, flux4_track.tracking); // "flux_from_fluxpool doesn't set tracking when true"
}

TEST( TestTracking, AdjustPool ) {
    // Adjust Pool To Val tests
    
    unitval u1(1.0, U_PGC);
    unitval u2(2.0, U_PGC);
    
    fluxpool f1(1.0, U_PGC);
    fluxpool f1_track = f1; // for ease of testing later
    f1_track.tracking = true;
    
    fluxpool f2(2.0, U_PGC, false, "f2");
    fluxpool f2_track = f2;
    f2_track.tracking = true;
    
    // Adjusting when not tracking - no "untracked"
    fluxpool adjust2_no_track = f2.flux_from_unitval(u1);
    adjust2_no_track.adjust_pool_to_val(u2.value(u2.units()));
    EXPECT_EQ(adjust2_no_track, u2); // "value is not adjusted correctly"
    adjust2_no_track.tracking = true;
    EXPECT_EQ(adjust2_no_track.get_fraction("untracked"), 0); // "when not tracking, untracked is not added to the map"
    
    // Adjusting to lower value - no "untracked"
    fluxpool adjust1_track = f2_track.flux_from_unitval(u2);
    adjust1_track.adjust_pool_to_val(u1.value(u1.units()));
    EXPECT_EQ(adjust1_track, u1); // "value is not adjusted correctly when tracking"
    EXPECT_EQ(adjust1_track.get_fraction("untracked"), 0); // "when tracking with negative diff, untracked is not added to the map"
    
    // Adjusting when no "untracked" is set
    fluxpool adjust2_no_untracked = f2_track.flux_from_unitval(u1);
    adjust2_no_untracked.adjust_pool_to_val(u2.value(u2.units()), false);
    EXPECT_EQ(adjust2_no_untracked, u2); // "value is not adjusted correctly"
    EXPECT_EQ(adjust2_no_untracked.get_fraction("untracked"), 0); // "when tracking with negative diff, untracked is not added to the map"
    
    // Adjusting to higher value when tracking - "untracked" is used
    fluxpool adjust2_track = f1_track.flux_from_unitval(u1);
    adjust2_track.adjust_pool_to_val(u2);
    EXPECT_EQ(adjust2_track, u2); // "value not adjusted correctly when tracking"
    EXPECT_EQ(adjust2_track.get_fraction("untracked"), u1/u2); // "untracked value not added to map"
}

TEST( TestTracking, Math ) {
    
    unitval u0(0.0, U_PGC);
    unitval u1(1.0, U_PGC);
    unitval u2(2.0, U_PGC);
    unitval u3 = u1 + u2;
    
    fluxpool f1(1.0, U_PGC);
    fluxpool f1_track = f1; // for ease of testing later
    f1_track.tracking = true;
    
    fluxpool f2(2.0, U_PGC, false, "f2");
    fluxpool f2_track = f2;
    f2_track.tracking = true;
    
    fluxpool f3 = f2 + f1;
    fluxpool f3_track = f2_track + f1_track;
    
    // Multiplication Tests
    fluxpool mult0r = f1*0.0;
    EXPECT_EQ(mult0r, u0); // "multiplication by 0 does not work"
    EXPECT_FALSE(f1 == u0); // "multiplied fluxpool is not affected"
    fluxpool mult0l = 0.0*f1;
    EXPECT_EQ(mult0l, u0); // "multiplication by 0 does not work"
    
    fluxpool mult4 = f1_track * 2.0;
    EXPECT_EQ(mult4, u2); // "multiplication by 0 does not work"
    EXPECT_EQ(mult4.get_tracking_map(), f1.get_tracking_map()); // "product map same as multipiled map"
    EXPECT_EQ(mult4.name, f1_track.name); // "names aren't preserved during multiplication"
    EXPECT_EQ(mult4.tracking, f1_track.tracking); // "tracking is not preserved during multiplicaiton"
    mult4 = mult4 + f2_track;
    EXPECT_FALSE(mult4.get_tracking_map() == f1_track.get_tracking_map()); // "multiplication does not produce a deep copy"
    fluxpool mult3 = mult4 * 0.5;
    // test for map with more than one element
    EXPECT_EQ(mult3.get_tracking_map(), mult4.get_tracking_map()); //"product map same as multipiled map with multiple elements"
    
    // Division Tests
    fluxpool div1 = f2_track / 2.0;
    EXPECT_EQ(div1, u1); // "division does not work"
    EXPECT_EQ(div1.get_tracking_map(), f2_track.get_tracking_map()); // "quotient map same as multipiled map"
    EXPECT_EQ(div1.name, f2_track.name); // "names aren't preserved during division"
    EXPECT_EQ(div1.tracking, f2_track.tracking); // "tracking is not preserved during multiplicaiton"
    div1 = div1 + f1_track;
    EXPECT_FALSE(div1.get_tracking_map() == f2_track.get_tracking_map()); // "division does not produce a deep copy"
    fluxpool div2 = div1 / 0.5;
    // test for map with more than one element
    EXPECT_EQ(div2.get_tracking_map(), div2.get_tracking_map()); // "quotient map same as divided map with multiple elements"
    
    // Subtraction Tests with Unitvals
    fluxpool sub_zero = f2 - u0;
    EXPECT_EQ(sub_zero, f2); // "subtraction by zero works"
    
    fluxpool sub1 = f2 - u1;
    EXPECT_EQ(sub1, u1); // "subtraction value doesn't work when not tracked"
    EXPECT_EQ(sub1.get_tracking_map(), f2.get_tracking_map()); // "untracked subtraction changes map"
    EXPECT_EQ(sub1.name, f2.name); // "subtraction preserves name"
    EXPECT_EQ(sub1.tracking, f2.tracking); // "subtract preserved tracking"
    
    fluxpool sub1_track = f2_track - u1;
    EXPECT_EQ(sub1_track, u1); // "subtraction value doesn't work when tracked"
    EXPECT_EQ(sub1_track.get_tracking_map(), f2_track.get_tracking_map()); // "tracked subtraction changes map"
    EXPECT_EQ(sub1_track.name, f2_track.name); // "subtraction preserves name"
    EXPECT_EQ(sub1_track.tracking, f2_track.tracking); // "subtract preserved tracking"
    
    fluxpool sub2_track = f3_track - u1;
    EXPECT_EQ(sub2_track.get_tracking_map(), f3_track.get_tracking_map()); // "tracked subtraction changes map with multiple keys"
    
    // Subtraction Tests with Fluxpools
    sub1_track = f2_track - f1_track;
    EXPECT_EQ(sub1_track, u1); // "subtraction value doesn't work when tracked"
    EXPECT_EQ(sub1_track.get_tracking_map(), f2_track.get_tracking_map()); // "tracked subtraction changes map"
    EXPECT_EQ(sub1_track.name, f2_track.name); // "subtraction preserves name"
    EXPECT_EQ(sub1_track.tracking, f2_track.tracking); // "subtract preserved tracking"
    
    sub2_track = f3_track - f1_track;
    EXPECT_EQ(sub2_track.get_tracking_map(), f3_track.get_tracking_map()); // "tracked subtraction changes map with multiple keys"
    
    // Addition Tests with unitvals
    fluxpool add2 = f1 + u1;
    EXPECT_EQ(add2.name, f1.name); // "name not preserved in unitval addition"
    EXPECT_EQ(add2.tracking, f1.tracking); // "tracking not preserved in unitval addition"
    EXPECT_EQ(add2, u2); // "value not preserved in unitval addition"
    fluxpool add4 = f3 + u1;
    
    // Addition Tests with Fluxpools
    fluxpool add3 = f1 + f2;
    EXPECT_EQ(add3.name, f1.name); // "addition of fluxpools doesn't preserve name when not tracking"
    EXPECT_EQ(add3.tracking, f1.tracking); // "addition of fluxpools doesn't preserve tracking when not tracking"
    EXPECT_EQ(add3, u3); // "addition of fluxpools doesn't preserve value when not tracking"
    
    fluxpool add3_track = f1_track + f2_track;
    EXPECT_EQ(add3_track.name, f1_track.name); // "addition of fluxpools doesn't preserve name when tracking"
    EXPECT_EQ(add3_track.tracking, f1_track.tracking); // "addition of fluxpools doesn't preserve tracking when tracking"
    EXPECT_EQ(add3_track, u3); // "addition of fluxpools doesn't preserve value when tracking"
    EXPECT_EQ(add3_track.get_fraction(f1_track.name), u1 / u3); // "addition of fluxpools does not modify map correctly when tracking"
    EXPECT_EQ(add3_track.get_fraction(f2_track.name), u2 / u3); // "addition of fluxpools does not modify map correctly when tracking"
    
    // Things that should NOT throw an exception
    fluxpool test = f2 - f1;
    test = f1 + f2;
    test = f1 * 2.0;
    test = f1 / 2.0;
    test = f1 - u1;
    test = f1 + u2;
    
    identity_tests(false);
    
    // Things that SHOULD throw an exception
    EXPECT_THROW(fluxpool(-1.0, U_PGC), h_exception);
    EXPECT_THROW(test.set(-1.0, U_PGC ), h_exception);
    EXPECT_THROW(f1 - f2, h_exception);
    EXPECT_THROW(f1 - u2, h_exception);
    EXPECT_THROW(f1 * -1.0, h_exception);
    EXPECT_THROW(-1.0 * f1, h_exception);
    EXPECT_THROW(f1 / -1.0, h_exception);
    EXPECT_THROW(if(f1 == fluxpool(1.0, U_UNITLESS)) {}, h_exception);
    EXPECT_THROW(f2 - unitval(0.0, U_UNITLESS), h_exception);
    EXPECT_THROW(f2 - fluxpool(0.0, U_UNITLESS, true), h_exception);
    EXPECT_THROW(f2_track - f1, h_exception);
    EXPECT_THROW(f2_track + u1, h_exception);
    EXPECT_THROW(f2_track + unitval(0.0, U_UNITLESS), h_exception);
    EXPECT_THROW(f2_track + f1, h_exception);
    EXPECT_THROW(f2_track + fluxpool(1.0, U_UNITLESS), h_exception);
}

TEST( TestTracking, Tracking ) {
    // Tracking tests
    identity_tests(true);
    
    // Non-tracked objects shouldn't give back tracking info
    fluxpool x(1.0, U_PGC);
    EXPECT_FALSE(x.tracking); // "default fluxpool shouldn't track"
    EXPECT_THROW(x.get_sources(), h_exception);
    EXPECT_THROW(x.get_fraction(""), h_exception);
    
    fluxpool y(1.0, U_PGC, true, "y");
    EXPECT_EQ(y.get_fraction("blah blah"), 0.0); // "get_fraction wasn't 0"
    EXPECT_EQ(y.get_fraction("y"), 1.0); // "get_fraction wasn't 1"
    vector<string> source = y.get_sources();
    EXPECT_EQ(source.size(), 1); // "source wasn't size 1"
    EXPECT_EQ(source.at(0), "y"); // "source wasn't y"
    
    // An actual tracking test
    // Test 1: one source, one destination
    fluxpool src1(10, U_PGC, true, "src1"),
    dest(0.0, U_PGC, true, "dest");
    
    fluxpool flux = src1 * 0.4;
    dest = dest + flux;
    src1 = src1 - flux;
    
    EXPECT_EQ(dest.get_fraction("src1"), 1.0); // "dest fraction src1 wasn't 1"
    source = dest.get_sources();
    EXPECT_EQ(source.size(), 2); // "dest source wasn't size 2"
    EXPECT_FALSE(find(source.begin(), source.end(), "src1") == source.end()); // "src wasn't in dest sources"
    
    // Test 2: two sources, one destination
    
    fluxpool src2(10, U_PGC, true, "src2");
    flux = src2 * 0.6;
    dest = dest + flux;
    src2 = src2 - flux;
    
    EXPECT_EQ(dest.get_fraction("src1"), 0.4); // "get_fraction wasn't 0.4 for src1"
    EXPECT_EQ(dest.get_fraction("src2"), 0.6); // "get_fraction wasn't 0.6 for src2"
    source = dest.get_sources();
    EXPECT_EQ(source.size(), 3); // "dest source wasn't size 3"
    
    // Test 3: start with a unitval flux and construct fluxpool (neeeded e.g. for LUC)
    
    unitval uflux(1.0, U_PGC);
    flux = src1.flux_from_unitval(uflux);
    EXPECT_EQ(uflux.value(U_PGC), flux.value(U_PGC)); // "flux and uflux values and/or units don't match"
    vector<string> s1 = src1.get_sources(), s2 = flux.get_sources();
    EXPECT_EQ(s1.size(), s2.size()); // "flux and flux source maps not same size"
    EXPECT_TRUE(std::equal(s1.begin(), s1.end(), s2.begin())); // "flux and uflux values source maps don't match"
    
    // Test 4: adjusting a total
    
    dest.adjust_pool_to_val(dest.value(U_PGC) * 1.1);
    EXPECT_TRUE(dest.get_fraction("untracked") - 1.0/11.0 < 1e-6); // "untracked not correct"
}
