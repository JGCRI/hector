/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  test_units.cpp
 *  hector
 *
 *  Created by Pralit Patel on 10/12/10.
 *
 */

#include <iostream>
#include <gtest/gtest.h>

#include "h_units.hpp"


using namespace boost::units;
using namespace boost::units::si;
using namespace boost::units::us;

class TestUnits : public testing::Test {
    // google test fixtures must be protected
protected:
    // fixture methods
    virtual void SetUp() {
    }
    
    // only define TearDown if it is needed
    /*virtual void TearDown() {
    }*/
};

TEST_F(TestUnits, MultiplyToCorrectUnit) {
    quantity<force> f( 2.0 * newton );
    quantity<length> l( 3.2 * meter );
    quantity<energy> e;
    quantity<energy> eCompare( 6.4 * joule );
    e = f * l;
    ASSERT_EQ( e, eCompare );
}

TEST_F(TestUnits, ConvertSystems) {
    quantity<length> lMeter( 1.0 * meter );
    
    typedef boost::units::make_system<foot_base_unit>::type foot_system;
    unit<length_dimension, foot_system> foot, feet;
    
    quantity<length> lFeet( 3.2808399 * feet );
    EXPECT_NEAR( lMeter.value(), lFeet.value(), .001 );
    //set_format( std::cout, typename_fmt );
//    std::cout << lMeter << std::endl;
//    std::cout << lFeet << std::endl;
}

TEST_F(TestUnits, molarMassTest) {
    quantity<grams_C> gas1( MOL_MASS_C * grams_C() );
//    std::cout << "gas1: " << gas1 << std::endl;
//    std::cout << "gas1: " << quantity<mole_mass>( gas1 ) << std::endl;
    EXPECT_EQ( quantity<mole_mass>( gas1 ).value(), 1.0 );
    
    quantity<grams_CO2> gas2( MOL_MASS_CO2 * grams_CO2() );
//    std::cout << "gas2: " << gas2 << std::endl;
//    std::cout << "gas2: " << quantity<mole_mass>( gas2 ) << std::endl;
//    std::cout << "gas2: " << quantity<grams_C>( gas2 ) << std::endl;
    EXPECT_EQ( quantity<grams_C>( gas2 ).value(), MOL_MASS_C );

    quantity<mole_mass> gas3( 2.0 * mole_mass() );
//    std::cout << "gas3: " << gas3 << std::endl;
//    std::cout << "gas3: " << quantity<grams_C>( gas3 ) << std::endl;
//    std::cout << "gas3: " << quantity<grams_CO2>( gas3 ) << std::endl;
//    std::cout << "gas3: " << quantity<grams_CH4>( gas3 ) << std::endl;
    EXPECT_EQ( quantity<grams_C>( gas3 ).value(), 2 * MOL_MASS_C );

    quantity<mole_mass> gas4 = quantity<mole_mass>( gas1 ) 
        + quantity<mole_mass>( gas2 ) + quantity<mole_mass>( gas3 );
//    std::cout << "gas4: " << gas4 << std::endl;
    EXPECT_EQ( gas4.value(), 4.0 );

    quantity<grams_CH4> gas5( 2 * MOL_MASS_CH4 * grams_CH4() );
    EXPECT_EQ( gas5.value(), 2 * MOL_MASS_CH4 );
//    std::cout << "gas5: " << gas5 << std::endl;
//    std::cout << "gas5: " << quantity<mole_mass>( gas5 ) << std::endl;
//    std::cout << "gas5: " << quantity<grams_C>( gas5 ) << std::endl;
}

TEST_F(TestUnits, scaledUnitsTest) {
    quantity<petagrams_C> gas1( 1.0 * petagrams_C() );
//    std::cout << "gas1: " << gas1 << std::endl;
//    std::cout << "gas1: " << quantity<grams_C>( gas1 ) << std::endl;
    EXPECT_EQ( (quantity<teragrams_C>( gas1 )).value(), 1e3 );
    EXPECT_EQ( (quantity<gigagrams_C>( gas1 )).value(), 1e6 );
    EXPECT_EQ( (quantity<megagrams_C>( gas1 )).value(), 1e9 );
    EXPECT_EQ( (quantity<kilograms_C>( gas1 )).value(), 1e12 );
    EXPECT_EQ( (quantity<grams_C>( gas1 )).value(), 1e15 );

    quantity<petagrams_CO2> gas2( 1.0 * petagrams_CO2() );
//    std::cout << "gas2: " << gas2 << std::endl;
//    std::cout << "gas2: " << quantity<grams_CO2>( gas2 ) << std::endl;
    EXPECT_EQ( (quantity<teragrams_CO2>( gas2 )).value(), 1e3 );
    EXPECT_EQ( (quantity<gigagrams_CO2>( gas2 )).value(), 1e6 );
    EXPECT_EQ( (quantity<megagrams_CO2>( gas2 )).value(), 1e9 );
    EXPECT_EQ( (quantity<kilograms_CO2>( gas2 )).value(), 1e12 );
    EXPECT_EQ( (quantity<grams_CO2>( gas2 )).value(), 1e15 );
//    std::cout << "gas2: " << quantity<grams_C>( gas2 ) << std::endl;
              
    quantity<petagrams_CH4> gas3( 1.0 * petagrams_CH4() );
//    std::cout << "gas3: " << gas3 << std::endl;
//    std::cout << "gas3: " << quantity<grams_CH4>( gas3 ) << std::endl;
    EXPECT_EQ( (quantity<teragrams_CH4>( gas3 )).value(), 1e3 );
    EXPECT_EQ( (quantity<gigagrams_CH4>( gas3 )).value(), 1e6 );
    EXPECT_EQ( (quantity<megagrams_CH4>( gas3 )).value(), 1e9 );
    EXPECT_EQ( (quantity<kilograms_CH4>( gas3 )).value(), 1e12 );
    EXPECT_EQ( (quantity<grams_CH4>( gas3 )).value(), 1e15 );
//    std::cout << "gas3: " << quantity<grams_C>( gas3 ) << std::endl;
}

TEST_F(TestUnits, ppmvTest) {
    // Value below based on 1 ppm by volume of atmosphere CO2 = 2.13 Gt C
    // (see h_units.hpp) #define GRAMSC_TO_PPMVCO2   1/2.13/1e15
    quantity<petagrams_C> gas1( 1  * petagrams_C() );
    EXPECT_EQ( quantity<ppmv_CO2>( gas1 ).value(), GRAMSC_TO_PPMVCO2 * 1e15 );    
}

TEST_F(TestUnits, GWPTest) {
    quantity<co2e> gas1( 21 * co2() );
    quantity<co2e> gas2( 1 * ch4() );
//    std::cout << "gas1: " << gas1 << std::endl;
//    std::cout << "gas2: " << gas2 << std::endl;
    quantity<co2e> gas3 = gas1 + gas2;
//    std::cout << "gas3: " << gas3 << std::endl;
//    std::cout << base_unit_info<co2_tag>::name() << std::endl;
    ASSERT_EQ( gas1, gas2 );
}

TEST_F(TestUnits, GWPMass) {
    typedef make_system<teragram_base_unit>::type teragram_system;
    unit<mass_dimension, teragram_system> teragram;
    quantity<mass_co2e> gas1( 21 * teragram * co2() );
    quantity<mass_co2e> gas2( 1 * teragram * ch4() );
//    std::cout << "gas1: " << gas1 << std::endl;
//    std::cout << "gas2: " << gas2 << std::endl;
    typedef scaled_base_unit<metric::ton_base_unit, scale<10, static_rational<6> > > megaton_base_unit;
    typedef make_system<megaton_base_unit>::type megaton_system;
    unit<mass_dimension, megaton_system> megaton;
    quantity<mass_co2e> gas3( 1 * megaton * co2() );
//    std::cout << "gas3: " << gas3 << std::endl;
    
    typedef scaled_base_unit<cgs::gram_base_unit, scale<10, static_rational<9> > > gigagram_base_unit;
    typedef make_system<gigagram_base_unit, co2e_tag>::type giga_mass_co2e_system;
    typedef unit<mass_co2e_dimension, giga_mass_co2e_system> giga_mass_co2e;

//    std::cout << "gas3 in Gg: " << quantity<giga_mass_co2e>( gas3 ) << std::endl;
    EXPECT_EQ( gas1, gas2 );
    EXPECT_EQ( gas3, quantity<mass_co2e>( 1 * teragram * co2e() ) );
}

TEST_F(TestUnits, MassFluxTest) {
    quantity<mole_flux> flux1( 1.0 * mole_flux() );
//    std::cout << "flux1: " << flux1 << std::endl;
}
