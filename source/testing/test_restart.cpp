/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  test_restart.cpp
 *  hector
 *
 *  Created by Pralit Patel on 3/28/11.
 *
 */

#include <iostream>
#include <gtest/gtest.h>
#include <string>
#include <sstream>

#include "h_exception.hpp"
#include "core/core.hpp"
#include "input/ini_to_core_reader.hpp"
#include "visitors/csv_outputstream_visitor.hpp"
#include "visitors/ini_restart_visitor.hpp"

using namespace std;

/*! \brief Unit tests for the restart ability of hector.
 *
 *  This will run the full model to capture the correct output.  Then run the model
 *  part way.  The restart file generated will then be used to run the model the
 *  rest of the way.  The output captured from the two segments will be combined and
 *  compared to the full output to determine correctness.  The CSVOutputStreamVisitor
 *  is used to compare results.
 */
class TestRestart : public testing::Test {
public:
    // google test fixtures must be protected
protected:
    // fixture methods
    virtual void SetUp() {
        tempRestartFileName = "test_restart_file.ini";
        // avoid stomping over someone else's files.
        H_ASSERT( !fileExists( tempRestartFileName.c_str() ), tempRestartFileName.c_str() );
    }
    
    // only define TearDown if it is needed
    virtual void TearDown() {
        // attempt to delete the temp file
        int retCode = remove( tempRestartFileName.c_str() );
        if( retCode != 0 )
            std::cout << "Warning could not remove temp file " << tempRestartFileName << std::endl;
    }
    
    // other helper methods
    bool fileExists( const char* fileName ) const {
        std::ifstream test( fileName );
        
        // this conversion to bool will let us know if the file successfully
        // opened
        return test;
    }
    
    std::string tempRestartFileName;
};

TEST_F(TestRestart, All) {
    try {
        Core full, to2000, from2000;
        INIToCoreReader fullReader( &full ), to2000Reader( &to2000 ), from2000Reader( &from2000 );
        stringstream fullOutput, to2000Output, from2000Output;
        // The full csv output stream is used to compare results, note the header
        // line will be suppressed to simply the comparisons.
        CSVOutputStreamVisitor fullVisitor( fullOutput, false ), to2000Visitor( to2000Output, false ),
            from2000Visitor( from2000Output, false );
        
        // WARNING: hard coding input file
        const string mainInputFile = "input/hector.ini";
        const string breakDate = "2000";
        
        // do the full run
        full.init();
        fullReader.parse( mainInputFile );
        full.addVisitor( &fullVisitor );
        full.run();
        
        // do the run up to year 2000
        to2000.init();
        to2000Reader.parse( mainInputFile );
        // reset the end date
        to2000.setData( to2000.getComponentName(), "endDate", breakDate );
        // create the restart file
        INIRestartVisitor restartVisitor( tempRestartFileName, to2000.getEndDate() );
        to2000.addVisitor( &restartVisitor );
        to2000.addVisitor( &to2000Visitor );
        to2000.run();
        
        // do the run from 2000
        from2000.init();
        from2000Reader.parse( mainInputFile );
        from2000Reader.parse( tempRestartFileName );
        from2000.addVisitor( &from2000Visitor );
        from2000.run();
        
        // combine latter two runs to compare to the full
        to2000Output << from2000Output.rdbuf();
        EXPECT_EQ( fullOutput.str(), to2000Output.str() );
        
    }
    catch( h_exception e ) {
		FAIL() << "* Program exception: " << e.msg << "\n* Function " << e.func << ", file " 
            << e.file << ", line " << e.linenum << endl;
	}
	catch( std::exception &e ) {
		FAIL() << "Standard exception: " << e.what() << endl;
	}
    catch( ... ) {
        FAIL() << "Other exception! " << endl;
    }
}
