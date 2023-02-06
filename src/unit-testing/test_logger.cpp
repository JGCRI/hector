/* Hector -- A Simple Climate Model
 Copyright (C) 2022  Battelle Memorial Institute
 
 Please see the accompanying file LICENSE.md for additional licensing
 information.
 */
/*
 *  test_logger.hpp
 *  hector
 *
 *  Created by Pralit Patel on 2010-09-23
 *
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <gtest/gtest.h>

#include "h_exception.hpp"
#include "logger.hpp"

using namespace Hector;

class LoggerTest : public testing::Test {
    // google test fixtures must be protected
protected:
    // fixture methods
    virtual void SetUp() {
        testFile1 = "logger_test_file1.txt";
        testFile2 = "logger_test_file2.txt";
        // avoid stomping over someone else's files.
        H_ASSERT( !fileExists( testFile1.c_str() ), "testfile1 exists" );
        H_ASSERT( !fileExists( testFile2.c_str() ), "testfile2 exists" );

        // swap out the cout buffer with a string stream so we can keep
        // track of if messages get echoed when they should / should not
        // be sure to keep a copy of the original so that we can restore
        // it in TearDown
        origCoutBuf = std::cout.rdbuf( &consoleTestBuff );
        
        loggerNoEcho.open( testFile1, false, true, Logger::WARNING );
        loggerEcho.open( testFile2, true, true, Logger::WARNING );
    }
    
    virtual void TearDown() {
        // restore the original cout buffer
        std::cout.rdbuf( origCoutBuf );

        // close the loggers explicitly so that the temp files can be deleted.
        loggerNoEcho.close();
        loggerEcho.close();
        
        // attempt to delete the temp files
        int retCode;
        if(fileExists( testFile1.c_str() )) {
            retCode = remove( testFile1.c_str() );
            if( retCode != 0 )
                std::cout << "Warning could not remove " << testFile1 << std::endl;
        }
        
        if(fileExists( testFile2.c_str() )) {
            retCode = remove( testFile2.c_str() );
            if( retCode != 0 )
                std::cout << "Warning could not remove " << testFile2 << std::endl;
        }
    }
    
    // other helper methods
    bool fileExists( const char* fileName ) const {
        std::ifstream test( fileName );
        return test.good();
    }
    
    std::string testFile1;
    std::string testFile2;
    
    Logger loggerNoEcho;
    Logger loggerEcho;
    
    std::stringbuf consoleTestBuff;
    std::streambuf* origCoutBuf;
};

TEST_F(LoggerTest, UninitializedLog) {
    Logger uninitialized;
    
    // 2021-07-03: H_LOG no longer throws an error because the log isn't
    // "enabled" and so Logger::write is never called. I'm not sure it's
    // possible to have an enabled but uninitialized log.  -BBL
    //ASSERT_THROW(H_LOG(unitialized, Logger::SEVERE), h_exception);
    // Instead test write() directly
    EXPECT_THROW(uninitialized.write(Logger::SEVERE, ""), h_exception);
}

TEST_F(LoggerTest, PriorityTooLow) {
    const std::string oldlog = consoleTestBuff.str();
    H_LOG(loggerEcho, Logger::DEBUG) << "This message should not show up" << std::endl;
    EXPECT_EQ(consoleTestBuff.str(), oldlog);  // i.e. no change
}

TEST_F(LoggerTest, PriorityHighEnough) {
    const std::string oldlog = consoleTestBuff.str();
    H_LOG(loggerEcho, Logger::SEVERE) << "This message should show up" << std::endl;
    const std::string bufferValue = consoleTestBuff.str();
    EXPECT_NE(bufferValue, oldlog);  // i.e. a change

    // TODO: create a more strict regular expression for what we are expecting.
    testing::internal::RE regex( ".*This message should show up" );
    EXPECT_TRUE(testing::internal::RE::PartialMatch( bufferValue, regex ) ) << "The actual value was " << bufferValue;
    
}

TEST_F(LoggerTest, PrioritySameAsMin) {
    H_LOG(loggerEcho, Logger::WARNING) << "This message should show up" << std::endl;
    
    // TODO: what to do about the non-deterministic logging message
    EXPECT_NE(consoleTestBuff.str(), "");
}

TEST_F(LoggerTest, NoEchoDoesNotEcho) {
    const std::string oldlog = consoleTestBuff.str();
    H_LOG(loggerNoEcho, Logger::SEVERE) << "This message should not show up" << std::endl;
    
    EXPECT_EQ(consoleTestBuff.str(), oldlog);  // i.e. no change
}

TEST_F(LoggerTest, LogToClosedLogger) {
    const std::string oldlog = consoleTestBuff.str();
loggerEcho.close();
    EXPECT_THROW(
                 H_LOG(loggerEcho, Logger::SEVERE)
                 << "This message should not show up" << std::endl,
                 h_exception );
    EXPECT_EQ(consoleTestBuff.str(), oldlog);  // i.e. no change
}
