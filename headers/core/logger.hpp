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
#ifndef LOGGER_H
#define LOGGER_H
/*
 *  logger.h
 *  hector
 *
 *  Created by Pralit Patel on 9/16/10.
 *
 */

#include <iostream>
#include <fstream>

#include "h_exception.hpp"

#define LOG_DIRECTORY "logs/"
#define LOG_EXTENSION ".log"

namespace Hector {
  
//------------------------------------------------------------------------------
/*! \brief Logger class
 *
 *  A basic logger class which can write logs to a file and optionally echo to
 *  the console as well.  Messages are logged with a priority and only messages
 *  with a high enough priority will actually be processed.
 */
class Logger {
public:
    /*! \brief Enumeration to describe available logging priority levels.
     */
    enum LogLevel {
        DEBUG,
        NOTICE,
        WARNING,
        SEVERE
    };
    
private:
    // Make the copy constructs private and undefined to disallow multiple
    // instances of the same log file.
    Logger( const Logger& );
    Logger& operator=( const Logger& );
    
    //! The minimum log level which will be processed.
    LogLevel minLogLevel;
    
    //! Flag to indicate that this logger has been successfully opened.
    bool isInitialized;
    
    //! The actual output stream which will handle the logging.
    std::ostream loggerStream;
    
    static const std::string& logLevelToStr( const LogLevel logLevel );
    
    static const char* getDateTimeStamp();

    static int chk_logdir(std::string dir);

    void printLogHeader( const LogLevel logLevel );
    
    /*! \brief A customized file stream buffer to enable echoing to a console.
     *
     *  This subclass will override the virtual protected methods necessary for
     *  output only.
     */
    class LoggerStreamBuf : public std::filebuf {
    private:
        //! A pointer to the streambuf of the console output stream, or null
        //! if echoToScreen was set to false during construction.
        std::streambuf* consoleBuf;
    public:
        LoggerStreamBuf( const bool echoToScreen );
        virtual ~LoggerStreamBuf();
        
    protected:
        // std::streambuf methods
        virtual int sync();
        
        virtual int overflow( int c = EOF );
        
        virtual std::streamsize xsputn( const char* s, std::streamsize n );
    };
    
public:
    Logger();
    ~Logger();
    
    void open( const std::string& logName, const bool echoToScreen,
              LogLevel minLogLevel) throw ( h_exception );
    
    bool shouldWrite( const LogLevel writeLevel ) const;
    
    std::ostream& write( const LogLevel writeLevel,
                        const std::string& functionInfo ) throw ( h_exception );
    
    void close();
    
    static Logger& getGlobalLogger();
};

}

/*!
 * \brief Some systems define the function name macro differently, so double
 *		  check and redefine as necessary.
 */
#ifndef __func__
#define __func__ __FUNCTION__
#endif

//------------------------------------------------------------------------------
/*! \brief Macro to perform logging.
 *
 *  This macro will check if the logging level qualifies to be logged.  If not
 *  the no more processing will be done.  Otherwise it will fill in the name of
 *  the function and return a reference to the output stream so the rest of the
 *  message may be logged.
 *
 * \param log An instance of the logger to log to.
 * \param level The logging priority to log at.
 */
#define H_LOG(log, level)  \
if( log.shouldWrite( level ) ) log.write( level, __func__ )

#endif
