/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  test_fluxpool.cpp
 *  hector
 *
 *  Created by Skylar Gering on 03/26/21.
 *
 */

#include <iostream>
#include <gtest/gtest.h>

#include "fluxpool.hpp"

using namespace std;

/*! \brief Unit tests for the fluxpool class.
 */
class TestFluxPools : public testing::Test {
public:
    TestFluxPools()
    {
    }
    // google test fixtures must be protected
protected:
    // fixture methods
    virtual void SetUp() {
    }   
};