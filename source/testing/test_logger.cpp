/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  test_logger.hpp
 *  hector
 *
 *  Created by Pralit Patel on 9/23/10.
 *
 */

#include <iostream>
#include <sstream>
#include <gtest/gtest.h>

#include "h_exception.hpp"
#include "logger.hpp"

class TestLogger : public testing::Test {
// google test fixtures must be protected
protected:
    // fixture methods
    virtual void SetUp() {
        testFile1 = "logger_test_file1.txt";
        testFile2 = "logger_test_file2.txt";
        // avoid stomping over someone else's files.
        H_ASSERT( !fileExists( testFile1.c_str() ), "testfile1 exists" );
        H_ASSERT( !fileExists( testFile2.c_str() ), "testfile2 exists" );

        loggerNoEcho.open( testFile1, false, Logger::WARNING );
        std::streambuf* tmpBuff = std::cout.rdbuf( &consoleTestBuff );
        loggerEcho.open( testFile2, true, Logger::WARNING );
        
        // now that the logger has attached to our test buffer put the
        // original back into cout
        std::cout.rdbuf( tmpBuff );
    }

    // only define TearDown if it is needed
    virtual void TearDown() {
        // close the loggers explicitly so that the temp files can be deleted.
        loggerNoEcho.close();
        loggerEcho.close();
        
        // attempt to delete the temp files
        int retCode = remove( testFile1.c_str() );
        if( retCode != 0 )
            std::cout << "Warning could not remove temp file " << testFile1 << std::endl;
        retCode = remove( testFile2.c_str() );
        if( retCode != 0 )
            std::cout << "Warning could not remove temp file " << testFile2 << std::endl;
    }
    
    // other helper methods
    bool fileExists( const char* fileName ) const {
        std::ifstream test( fileName );
        
        // this conversion to bool will let us know if the file successfully
        // opened
        return test;
    }
    
    std::string testFile1;
    std::string testFile2;
    
    Logger loggerNoEcho;
    Logger loggerEcho;
    
    std::stringbuf consoleTestBuff;
};

TEST_F(TestLogger, UninitializedLog) {
    Logger unitialized;
    ASSERT_THROW(H_LOG(unitialized, Logger::SEVERE), h_exception);
}

TEST_F(TestLogger, PriorityTooLow) {
    H_LOG(loggerEcho, Logger::DEBUG) << "This message should not show up" << std::endl;
    
    ASSERT_EQ(consoleTestBuff.str(), "");
}

TEST_F(TestLogger, PriorityHighEnough) {
    H_LOG(loggerEcho, Logger::SEVERE) << "This message should show up" << std::endl;
    
    // TODO: what to do about the non-deterministic logging message
    const std::string bufferValue = consoleTestBuff.str();
    EXPECT_NE(bufferValue, "");
    
    // TODO: create a more strict regular expression for what we are expecting.
    testing::internal::RE regex( ".*This message should show up" );
    ASSERT_TRUE(testing::internal::RE::PartialMatch( bufferValue, regex ) ) << "The actual value was " << bufferValue;
    
}

TEST_F(TestLogger, PrioritySameAsMin) {
    H_LOG(loggerEcho, Logger::WARNING) << "This message should show up" << std::endl;
    
    // TODO: what to do about the non-deterministic logging message
    ASSERT_NE(consoleTestBuff.str(), "");
}

TEST_F(TestLogger, NoEchoDoesNotEcho) {
    H_LOG(loggerNoEcho, Logger::SEVERE) << "This message should not show up" << std::endl;
    
    ASSERT_EQ(consoleTestBuff.str(), "");
}

TEST_F(TestLogger, LogToClosedLogger) {
    loggerEcho.close();
    EXPECT_THROW(
                 H_LOG(loggerEcho, Logger::SEVERE) 
                    << "This message should not show up" << std::endl,
                 h_exception );
    ASSERT_EQ(consoleTestBuff.str(), "");
}
