/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  test_csv_table_reader.cpp
 *  hector
 *
 *  Created by Pralit Patel on 1/12/11.
 *
 */

#include <fstream>
#include <errno.h>
#include <iostream>
#include <gtest/gtest.h>

#include "h_exception.hpp"
#include "core.hpp"
#include "csv_table_reader.hpp"
#include "avisitor.hpp"
#include "dummy_model_component.hpp"
#include "tseries.hpp"

using namespace Hector;

/*! \brief Create a testing environment so that we can create our temporary file
 *         before we construct our test reader.
 */
class TestCSVTableReaderEnv : public testing::Environment {
public:
    TestCSVTableReaderEnv() {}
    virtual ~TestCSVTableReaderEnv() {}
    
    virtual void SetUp() {
        // avoid stomping over someone else's files.
        H_ASSERT( !fileExists( testFileName.c_str() ), "testfile exists" );
        std::fstream temp( testFileName.c_str(), std::fstream::out );
    }
    
    virtual void TearDown() {
        // attempt to delete the temp file
        int retCode = remove( testFileName.c_str() );
        if( retCode != 0 )
            std::cout << "Warning could not remove temp file " << testFileName << std::endl;
    }
    
    // other helper methods
    bool fileExists( const char* fileName ) const {
        std::ifstream test( fileName );
        return test.good();
    }
    
    static std::string testFileName;
};

std::string TestCSVTableReaderEnv::testFileName = "csv_test_file.csv";

::testing::Environment* const testingGlobalEnv = ::testing::AddGlobalTestEnvironment( new TestCSVTableReaderEnv );

/*! \brief Unit tests for the CSVTableReader class.
 *
 *  Check to make sure this class enforces the CSV format expected and properly
 *  reads and sends data to the Core.
 *
 *  \warning This test is currently tightly coupled with Core and DummyModelComponent
 *           which will likely break the tests in the near future.
 */
class TestCSVTableReader : public testing::Test {
public:
    TestCSVTableReader()
    :reader( TestCSVTableReaderEnv::testFileName ),
     testComponentName("dummy-component"),
     testVarName("c")
    {
        // Use the DummyModelComponent for testing
        Core core(Logger::SEVERE, false, false);
        core.addModelComponent( new DummyModelComponent );
        core.init();
    }
    // google test fixtures must be protected
protected:
    // fixture methods
    virtual void SetUp() {
        // open and truncate the temporary file each time so we can alter it for
        // each test
        testFile.open( TestCSVTableReaderEnv::testFileName.c_str(),
                       std::ios::in | std::ios::out | std::ios::trunc );
    }
    
    // only define TearDown if it is needed
    /*virtual void TearDown() {
    }*/
        
    const std::string testComponentName;
    const std::string testVarName;
    
    CSVTableReader reader;
    
    std::fstream testFile;
    
    class CheckDummyVisitor: public AVisitor {
    public:
        CheckDummyVisitor(double indexCheck)
        :indexCheck( indexCheck ), valueResult( -1 )
        {
        }
        
        virtual bool shouldVisit( const bool in_spinup, const double date ) {
            return true;
        }
        
        virtual void visit( DummyModelComponent* dummyModel ) {
            tseries<double>& tempC = const_cast<tseries<double>&>(dummyModel->getC());
            // TODO: size should be const
            if(tempC.size() > 0) {
                valueResult = tempC.get( indexCheck );
            }
        }
        
        double indexCheck;
        double valueResult;
    };
};

TEST_F(TestCSVTableReader, ReadNonExistentFile) {
    ASSERT_THROW(CSVTableReader reader2("does_not_exist.csv"), h_exception);
}

TEST_F(TestCSVTableReader, InvalidHeaderLine) {
    testFile << "Date,Wrong" << std::endl;
    testFile << "2,6" << std::endl;
    testFile.close();
    Core core(Logger::SEVERE, false, false);
    ASSERT_THROW(reader.process(&core, testComponentName, testVarName), h_exception);
}

TEST_F(TestCSVTableReader, WrongSeparatorInHeader) {
    testFile << "Date;" << testVarName << std::endl;
    testFile << "2,6" << std::endl;
    testFile.close();
    Core core(Logger::SEVERE, false, false);
    ASSERT_THROW(reader.process(&core, testComponentName, testVarName), h_exception);
}

TEST_F(TestCSVTableReader, WrongSeparatorInRow) {
    testFile << "Date," << testVarName << std::endl;
    testFile << "2;6" << std::endl;
    testFile.close();
    Core core(Logger::SEVERE, false, false);
//    ASSERT_THROW(reader.process(&core, testComponentName, testVarName), h_exception);
}

TEST_F(TestCSVTableReader, OkWithJustHeaderNoData) {
    testFile << "Date," << testVarName << std::endl;
    testFile.close();
    Core core(Logger::SEVERE, false, false);
    ASSERT_NO_THROW(reader.process(&core, testComponentName, testVarName));
    CheckDummyVisitor check( -1 );
    core.accept( &check );
    ASSERT_EQ( check.valueResult, -1 );
}

TEST_F(TestCSVTableReader, IgnoreWhitespaceHeader) {
    Core core(Logger::SEVERE, false, false);
    core.addModelComponent( new DummyModelComponent );
    testFile << "Date,  " << testVarName << "  , Other" << std::endl;
    testFile << "2,6,1" << std::endl;
    testFile.close();
    ASSERT_NO_THROW(reader.process(&core, testComponentName, testVarName));
    CheckDummyVisitor check( 2 );
    core.accept( &check );
    ASSERT_EQ( check.valueResult, 6 );
}

TEST_F(TestCSVTableReader, IgnoreWhitespaceRow) {
    Core core(Logger::SEVERE, false, false);
    core.addModelComponent( new DummyModelComponent );
    testFile << "Date," << testVarName << ",Other" << std::endl;
    testFile << "2,  6,   1" << std::endl;
    testFile.close();
    ASSERT_NO_THROW(reader.process(&core, testComponentName, testVarName));
    CheckDummyVisitor check( 2 );
    core.accept( &check );
    ASSERT_EQ( check.valueResult, 6 );
}

TEST_F(TestCSVTableReader, InvalidDateIndex) {
    testFile << "Date," << testVarName << std::endl;
    testFile << "Oops,6" << std::endl;
    testFile.close();
    Core core(Logger::SEVERE, false, false);
    ASSERT_THROW(reader.process(&core, testComponentName, testVarName), h_exception);
}

TEST_F(TestCSVTableReader, NeedIndexColumn) {
    testFile << testVarName << ",other" << std::endl;
    testFile << "2,6" << std::endl;
    testFile.close();
    Core core(Logger::SEVERE, false, false);
    ASSERT_THROW(reader.process(&core, testComponentName, testVarName), h_exception);
}

TEST_F(TestCSVTableReader, NumColumnsDoNotChange) {
    testFile << "Date," << testVarName << ",Other" << std::endl;
    testFile << "2,6" << std::endl;
    testFile.close();
    Core core(Logger::SEVERE, false, false);
    ASSERT_THROW(reader.process(&core, testComponentName, testVarName), h_exception);
}

TEST_F(TestCSVTableReader, CanReRead) {
    Core core(Logger::SEVERE, false, false);
    core.addModelComponent( new DummyModelComponent );
    testFile << "Date," << testVarName << ",Other" << std::endl;
    testFile << "2,6,1" << std::endl;
    testFile.close();
    ASSERT_NO_THROW(reader.process(&core, testComponentName, testVarName));
    CheckDummyVisitor check( 2 );
    core.accept( &check );
    ASSERT_EQ( check.valueResult, 6 );
    ASSERT_NO_THROW(reader.process(&core, testComponentName, testVarName));
    check.valueResult = -1;
    core.accept( &check );
    ASSERT_EQ( check.valueResult, 6 );
}

TEST_F(TestCSVTableReader, CanMixNewlineHeader) {
    Core core(Logger::SEVERE, false, false);
    core.addModelComponent( new DummyModelComponent );
    testFile << "Date," << testVarName << "\r\n";
    testFile << "2,6\n";
    testFile.close();
    ASSERT_NO_THROW(reader.process(&core, testComponentName, testVarName));
    CheckDummyVisitor check( 2 );
    core.accept( &check );
    ASSERT_EQ( check.valueResult, 6 );
}

TEST_F(TestCSVTableReader, CanMixNewlineData) {
    Core core(Logger::SEVERE, false, false);
    core.addModelComponent( new DummyModelComponent );
    testFile << "Date," << testVarName << "\n";
    testFile << "2,6\r\n";
    testFile.close();
    ASSERT_NO_THROW(reader.process(&core, testComponentName, testVarName));
    CheckDummyVisitor check( 2 );
    core.accept( &check );
    ASSERT_EQ( check.valueResult, 6 );
}

TEST_F(TestCSVTableReader, CanIgnoreExtraline) {
    Core core(Logger::SEVERE, false, false);
    core.addModelComponent( new DummyModelComponent );
    testFile << "Date," << testVarName << ",Other" << std::endl;
    testFile << "2,6,1" << std::endl;
    testFile << "\n";
    testFile.close();
    ASSERT_NO_THROW(reader.process(&core, testComponentName, testVarName));
    CheckDummyVisitor check( 2 );
    core.accept( &check );
    ASSERT_EQ( check.valueResult, 6 );
}

TEST_F(TestCSVTableReader, CanIgnoreExtralineCR) {
    Core core(Logger::SEVERE, false, false);
    core.addModelComponent( new DummyModelComponent );
    testFile << "Date," << testVarName << ",Other" << std::endl;
    testFile << "2,6,1" << std::endl;
    testFile << "\r\n";
    testFile.close();
    ASSERT_NO_THROW(reader.process(&core, testComponentName, testVarName));
    CheckDummyVisitor check( 2 );
    core.accept( &check );
    ASSERT_EQ( check.valueResult, 6 );
}
