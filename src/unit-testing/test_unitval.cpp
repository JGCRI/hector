/* Hector -- A Simple Climate Model
 Copyright (C) 2021  Battelle Memorial Institute
 
 Please see the accompanying file LICENSE.md for additional licensing
 information.
 */
/*
 *  test_unitval.cpp
 *  hector
 *
 *  Created by Ben Bond-Lamberty on 2021-08-27
 *
 */

#include <iostream>
#include <gtest/gtest.h>

#include "unitval.hpp"

using namespace std;

/*! \brief Unit tests for the unitval class.
 */
class UnitvalTest : public testing::Test {
public:
    UnitvalTest()
    {
    }
    // google test fixtures must be protected
protected:
    // fixture methods
    virtual void SetUp() {
    }
};

using namespace Hector;

TEST_F( UnitvalTest, Construction ) {
    
    // Default constructors
    unitval x;
    EXPECT_EQ(x.value(U_UNDEFINED), 0.0);
    
    const double val = 1.0;
    x = unitval(val, U_UNITLESS);
    EXPECT_EQ(x.value(U_UNITLESS), val);
    
    // set works, but throws an error if changing units (other than if was U_UNDEFINED)
    // set, no unit change
    x = unitval(val, U_G);
    x.set(val * 2, U_G);
    EXPECT_EQ(x.value(U_G), val * 2);
    EXPECT_EQ(x.units(), U_G);
    
    // set, unit change but was previously undefined
    x = unitval(val, U_UNDEFINED);
    x.set(val * 2, U_G);
    EXPECT_EQ(x.value(U_G), val * 2);

    // set, illegal unit change
    EXPECT_THROW(x.set(val, U_K), h_exception);
}

TEST_F( UnitvalTest, Units ) {
    // unitsName
    // not going to test ALL the possibilities here
    unitval x(1.0, U_UNDEFINED);
    EXPECT_EQ(x.units(), U_UNDEFINED);
    EXPECT_EQ(x.unitsName(), "(undefined)");

    // unitsName also has a version where you pass it a unit independent of 'x'
    EXPECT_EQ(x.unitsName(U_K), "K");
    EXPECT_EQ(x.unitsName(U_M3_S), "m3/s");
    
    // expecting_unit
    unitval y(1.0, U_M3_S);
    y.expecting_unit(U_M3_S);
    EXPECT_THROW(y.expecting_unit(U_UNDEFINED), h_exception);
    
    // if current units are undefined, they are set
    y = unitval(1.0, U_UNDEFINED);
    y.expecting_unit(U_M3_S);
    EXPECT_EQ(y.units(), U_M3_S);
}

TEST_F( UnitvalTest, Parsing ) {
    unitval x(1.0, U_M3_S);
    string input = "1.0," + x.unitsName(U_M3_S);
    // parses string correctly
    EXPECT_EQ(x.parse_unitval(input, U_M3_S), x);
    // errors on different unit than expected
    EXPECT_THROW(x.parse_unitval(input, U_K), h_exception);
    
    // handles whitespace
    input = " 1.0," + x.unitsName(U_M3_S);
    EXPECT_EQ(x.parse_unitval(input, U_M3_S), x);
    input = "1.0 , " + x.unitsName(U_M3_S);
    EXPECT_EQ(x.parse_unitval(input, U_M3_S), x);
    input = "1.0," + x.unitsName(U_M3_S) + "   ";
    EXPECT_EQ(x.parse_unitval(input, U_M3_S), x);
    
    // no units given is allowed
    EXPECT_EQ(x.parse_unitval("1.0", U_M3_S), x);
    EXPECT_EQ(x.parse_unitval("1.0,", U_M3_S), x);

    // bad input
    input = "abc," + x.unitsName(U_M3_S);
    EXPECT_THROW(x.parse_unitval(input, U_M3_S), h_exception);
    EXPECT_THROW(x.parse_unitval("1.0,abc", U_M3_S), h_exception);
}

TEST_F( UnitvalTest, Math ) {
    unitval x(1.0, U_M3_S);
    unitval y(2.0, U_M3_S);
    unitval minusy(-2.0, U_M3_S);
    unitval ybad(2.0, U_G);
    unitval z(3.0, U_M3_S);
    
    // addition and subtraction
    EXPECT_EQ(x + y, z);
    EXPECT_EQ(-y, minusy);  // unary minus
    EXPECT_EQ(x - -y, z);   // unary minus
    EXPECT_EQ(z - y, x);
    EXPECT_EQ(z - x, y);
    EXPECT_THROW(x + ybad, h_exception);
    EXPECT_THROW(x - ybad, h_exception);
    
    // multiplication and division
    EXPECT_EQ(x * 1.0, x);
    EXPECT_EQ(1.0 * x, x);
    EXPECT_EQ(x / 1.0, x);
    EXPECT_EQ(x * 2.0, y);
    EXPECT_EQ(z / 3.0, x);
    EXPECT_EQ(y / x, 2.0);
    EXPECT_EQ(z / z, 1.0);
    EXPECT_THROW(ybad / x, h_exception);
}

TEST_F( UnitvalTest, unitsName ) {
    // Make sure unitsName() has a string all the enumerated units
    for( int i = 0; i <= U_UNDEFINED; ++i ) {
        const unit_types u = static_cast<unit_types>( i );
        EXPECT_NO_THROW( Hector::unitval::unitsName( u ) );
    }
}

TEST_F( UnitvalTest, Output ) {
    stringstream out;
    unitval x(1.0, U_M3_S);
    
    // not sure what to test here, other than it does print as we expect
    out << x;
    EXPECT_EQ(out.str(), "1 " + x.unitsName());
}
