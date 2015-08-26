/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
		cerr << "* Program exception: " << e.msg << "\n* Function " << e.func << ", file " 
        << e.file << ", line " << e.linenum << endl;
	}
	catch( std::exception &e ) {
		cerr << "Standard exception: " << e.what() << endl;
	}
    catch( ... ) {
        cerr << "Other exception! " << endl;
    }

}
