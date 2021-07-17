/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  testing_main.cpp
 *  hector
 *
 *  Created by d3x290-local on 9/23/10.
 *  Copyright 2010 DOE Pacific Northwest Lab. All rights reserved.
 *
 */

#include <iostream>
#include <gtest/gtest.h>
#include "h_exception.hpp"

using namespace std;

// Google Test main. All tests are automatically detected and run.


GTEST_API_ int main(int argc, char **argv) {

 testing::InitGoogleTest(&argc, argv);

    try {
        
        return RUN_ALL_TESTS();
        
	}
	catch( const h_exception &e ) {
        cerr << "* Program exception:\n" << e << endl;
        return 1;
	}
	catch( std::exception &e ) {
        cerr << "Standard exception: " << e.what() << endl;
        return 2;
	}
    catch( ... ) {
        cerr << "Other exception! " << endl;
        return 3;
    }

}
