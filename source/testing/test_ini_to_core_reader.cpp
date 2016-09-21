/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  test_ini_to_core_reader.cpp
 *  hector
 *
 *  Created by Pralit Patel on 11/2/10.
 *
 */

#include <fstream>
#include <iostream>
#include <gtest/gtest.h>

#include "h_exception.hpp"
#include "core/core.hpp"
#include "core/dummy_model_component.hpp"
#include "input/ini_to_core_reader.hpp"

/*! \brief Unit tests for the INIToCoreReader class.
 *
 *  Check to make sure the behavior of passing data parsed from an INI file to
 *  the model core behaves as expected.
 *
 *  \warning This test is currently tightly coupled with Core and DummyModelComponent
 *           which will likely break the tests in the near future.
 */
class TestINIToCore : public testing::Test {
public:
    TestINIToCore():reader( &core )
    {
        // Use the DummyModelComponent for testing
        core.addModelComponent( new DummyModelComponent );
        core.init();
    }
    // google test fixtures must be protected
protected:
    // fixture methods
    virtual void SetUp() {
        testFileName = "ini_test_file.ini";
        // avoid stomping over someone else's files.
        H_ASSERT( !fileExists( testFileName.c_str() ), "testfile exists" );
        
        testFile.open( testFileName.c_str(), std::ios::in | std::ios::out | std::ios::trunc );
    }
    
    // only define TearDown if it is needed
    virtual void TearDown() {
        // close the file explicitly so that the temp files can be deleted.
        testFile.close();
        
        // attempt to delete the temp file
        int retCode = remove( testFileName.c_str() );
        if( retCode != 0 )
            std::cout << "Warning could not remove temp file " << testFileName << std::endl;
    }
    
    // other helper methods
    bool fileExists( const char* fileName ) const {
        std::ifstream test( fileName );
        
        // this conversion to bool will let us know if the file successfully
        // opened
        return test;
    }
    
    Core core;
    
    INIToCoreReader reader;
    
    std::string testFileName;
    
    std::fstream testFile;
};

TEST_F(TestINIToCore, ReadNonExistentFile) {
    ASSERT_THROW(reader.parse("does_not_exist.ini"), h_exception);
}

TEST_F(TestINIToCore, InvalidLine) {
    testFile << "[made_up_component]" << std:: endl;
    testFile << "=6" << std::endl;
    ASSERT_THROW(reader.parse("does_not_exist.ini"), h_exception);
}

TEST_F(TestINIToCore, DoesNotFindComponent) {
    testFile << "[made_up_component]" << std:: endl;
    testFile << "slope=6" << std::endl;
    ASSERT_THROW(reader.parse(testFileName), h_exception);
}

TEST_F(TestINIToCore, DoesNotFindVar) {
    testFile << "[dummy-component]" << std:: endl;
    testFile << "x=6" << std::endl;
    ASSERT_THROW(reader.parse(testFileName), h_exception);
}

TEST_F(TestINIToCore, VarWrongType) {
    testFile << "[dummy-component]" << std:: endl;
    testFile << "slope=string" << std::endl;
    ASSERT_THROW(reader.parse(testFileName), h_exception);
}

TEST_F(TestINIToCore, DoesFindComponent) {
    testFile << "[dummy-component]" << std:: endl;
    testFile << "slope=6" << std::endl;
    ASSERT_NO_THROW(reader.parse(testFileName));
    // TODO: how to check to make sure the value got set?
}

TEST_F(TestINIToCore, UnclosedBracket) {
    testFile << "[dummy-component]" << std:: endl;
    testFile << "c[2.7=6" << std::endl;
    ASSERT_THROW(reader.parse(testFileName), h_exception);
}

TEST_F(TestINIToCore, UnopenedBracket) {
    testFile << "[dummy-component]" << std:: endl;
    testFile << "c2.7]=6" << std::endl;
    ASSERT_THROW(reader.parse(testFileName), h_exception);
}

TEST_F(TestINIToCore, ParseSingleIndex) {
    testFile << "[dummy-component]" << std:: endl;
    testFile << "c[2.7]=6" << std::endl;
    ASSERT_NO_THROW(reader.parse(testFileName));
    // TODO: how to check to make sure the value got set?
}

TEST_F(TestINIToCore, TableFilenameNoPrefix) {
    testFile << "[dummy-component]" << std:: endl;
    testFile << "c=non_existant.csv" << std::endl;
    // make sure this throws an exception for the right reason and not due to
    // a non existant file
    try {
        reader.parse(testFileName);
    } catch( h_exception e) {
        // we should get and exception from dummy model component when it
        // tries to set c to a single value
        ASSERT_EQ( e.file, "dummy_model_component.cpp" );
    }
}

TEST_F(TestINIToCore, TableFilenameWithCSVPrefix) {
    testFile << "[dummy-component]" << std:: endl;
    testFile << "c=csv:non_existant.csv" << std::endl;
    try {
        reader.parse(testFileName);
    } catch( h_exception e ) {
        // looking for an exception from CSVTableReader which indicates that it
        // understood to try to read a CSV file
        ASSERT_EQ( e.file, "csv_table_reader.cpp" );
    }
}
