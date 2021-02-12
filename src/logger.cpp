/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  logger.cpp
 *  hector
 *
 *  Created by Pralit Patel on 9/16/10.
 *
 */

#include <time.h>
#include "logger.hpp"
#include "h_util.hpp"
#include <algorithm>

// If using the R package, use Rcpp to call R's file processing
// functions. Otherwise (e.g. if building standalone Hector), fall
// back to boost::filesystem (which needs to be installed).
#ifdef USE_RCPP
#include <Rcpp.h>
#else
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#endif

#ifdef USE_RCPP
// This should be defined only if compiling as an R package
#include <Rcpp.h>
#define STDOUT_STREAM Rcpp::Rcout
#else
#define STDOUT_STREAM std::cout
#endif

namespace Hector {

using namespace std;

//------------------------------------------------------------------------------
// Methods for LoggerStreamBuf

//------------------------------------------------------------------------------
/*! \brief Constructor which will allow a user the option to echo output to a
 *         console buffer.
 *
 * \param echoToScreen Boolean to indicate whether output will be echoed.
 */
Logger::LoggerStreamBuf::LoggerStreamBuf( const bool echoToScreen )
:filebuf()
{
    consoleBuf = echoToScreen ? STDOUT_STREAM.rdbuf() : 0;
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
Logger::LoggerStreamBuf::~LoggerStreamBuf() {
}

//------------------------------------------------------------------------------
/*! \brief Synchronize stream buffer.
 *  \return 0 indicates success, any other value is an error code.
 */
int Logger::LoggerStreamBuf::sync() {
    // sync will force an overflow to write EOF so wait until that occurs
    // before forward a flush to the console
    int ret = filebuf::sync();
    if(consoleBuf) {
        ret = consoleBuf->pubsync();
    }

    return ret;
}

//------------------------------------------------------------------------------
/*! \brief Write a character in the case of a buffer overflow.
 *  \param c Character to be written.
 *  \return Any value other than EOF indicates success.
 */
int Logger::LoggerStreamBuf::overflow( int c ) {
    if(consoleBuf && c == EOF ) {
        // if this overflow was due to a sync then copy the current buffer
        // to the console before it gets flushed
        consoleBuf->sputn( pbase(), pptr() - pbase() );
    }

    return filebuf::overflow( c );
}

//------------------------------------------------------------------------------
/*! \brief Write a sequence of characters.
 *  \param c Character sequence to be written.
 *  \param n Number of characters to be written.
 *  \return The actual number of characters written.
 */
streamsize Logger::LoggerStreamBuf::xsputn( const char* s, streamsize n ) {
    if( consoleBuf && ( n + pptr() > epptr() ) && ( epptr() - pptr() > 0 ) ) {
        // if the size of the incoming string is greater than than the buffer
        // then it will flush the buffer and flush the given string without
        // giving an overflow so we will need to update the console buffer now
        consoleBuf->sputn( pbase(), pptr() - pbase() );
        consoleBuf->sputn( s, n );
    }

    return filebuf::xsputn( s, n );
}

//------------------------------------------------------------------------------
// Methods for Logger

//------------------------------------------------------------------------------
/*! \brief Create an uninitialized logger.
 */
Logger::Logger() :
minLogLevel( WARNING ),
isInitialized( false ),
loggerStream( 0 )
{
}

//------------------------------------------------------------------------------
/*! \brief Destructor.
 *
 *  The logger will be automatically closed on destruction.
 */
Logger::~Logger() {
    close();
}

//------------------------------------------------------------------------------
/*! \brief Open and initialize the logger.
 *
 *  Initialization can only occur once, any subsequent attempts will result in
 *  an exception.  Once open the logger will be unable to change setting on
 *  which file to log to, whether to echo to screen, nor the minimum priority
 *  level which will be logged.
 *
 * \param logName The file name the log will be written to.
 * \param echoToScreen A flag to indicate if messages should be echoed to the
 *                     console.
 * \param minLogLevel The minimum priority which will be processed.
 * \param echoToFile A flag to indicate if messages should be written to a log
 *                   file. If neither echoToScreen nor echoToFile is true, the
 *                   logger is disabled.
 *                   (default: true)
 * \exception h_exception Exception thrown if the logger has already been
 *                        initialized.
 *
 */
void Logger::open( const string& logName, bool echoToScreen,
                   bool echoToFile, LogLevel minLogLevel ) {
    H_ASSERT( !isInitialized, "This log has already been initialized." );

    this->minLogLevel = minLogLevel;
    this->echoToFile = echoToFile;

    if (echoToFile) {
        chk_logdir(LOG_DIRECTORY);

        const string fqName = LOG_DIRECTORY + logName + LOG_EXTENSION;	// fully-qualified name

        LoggerStreamBuf* buff = new LoggerStreamBuf( echoToScreen );
        if( !buff->open( fqName.c_str(), ios::out ) )
            H_THROW("Unable to open log file " + fqName);

        loggerStream.rdbuf( buff );
    } else {
        loggerStream.rdbuf( STDOUT_STREAM.rdbuf() );
    }
    enabled = echoToScreen || echoToFile;

    isInitialized = true;

    // ensure that the log header is always printed.
    printLogHeader( max( minLogLevel, NOTICE ) );
}

//------------------------------------------------------------------------------
/*! \brief Indicate whether a message at the given priority will be logged.
 *  \param writeLevel The priority level to check.
 *  \return True if this level would be logged, false otherwise.
 */
bool Logger::shouldWrite( const LogLevel writeLevel ) const {
    return enabled && writeLevel >= minLogLevel;
}

//------------------------------------------------------------------------------
/*! \brief Write a formatted log message header and return the output stream to
 *         allow the outputting the actual message.
 *  \param writeLevel The priority level at which this message is logged at.
 *  \param functionInfo A string which indicates which function this message
 *                      came from.
 *  \return The output stream to place the log message into.
 *  \warning This method should NEVER be used directly, instead use the macro
 *           H_LOG(log, level).
 *  \exception h_exception If this logger has not been initialized.
 */
ostream& Logger::write( const LogLevel writeLevel,
                       const string& functionInfo )
{
    H_ASSERT( isInitialized, "can't write to logger until initialized" );

    // note that we not double checking the writeLevel
    return loggerStream << getDateTimeStamp() << ':' <<logLevelToStr( writeLevel )
    << ':' << functionInfo << ": ";
}

//------------------------------------------------------------------------------
/*! \brief Close the logger.
 */
void Logger::close() {
    if( isInitialized ) {
        if (echoToFile) {
            LoggerStreamBuf *lsbuf = static_cast<LoggerStreamBuf*>( loggerStream.rdbuf() );
            lsbuf->close();
            delete lsbuf;
        }
        /*! \note Setting isInitialized back to false will allow this logger to
         *        be reopened.
         */
        isInitialized = false;
    }
}


//------------------------------------------------------------------------------
/*! \brief Convert the enum to a string so that it can be logged.
 *  \param logLevel The enum to convert.
 *  \return The string representing the given LogLevel.
 */
const string& Logger::logLevelToStr( const LogLevel logLevel ) {
    const static string logLevelStrings[] = {
        "DEBUG",
        "NOTICE",
        "WARNING",
        "SEVERE"
    };

    return logLevelStrings[ logLevel ];
}

//------------------------------------------------------------------------------
/*! \brief Get the current data and time stamp.
 *  \return A string representing the current date and time.
 */
const char* Logger::getDateTimeStamp() {
    time_t rawtime;
    struct tm* timeinfo;
    time( &rawtime );
    timeinfo = localtime( &rawtime );
    char* ret = asctime( timeinfo );
    // remove the newline at the end of the string
    int i = 0;
    while( ret[ i ] != 0 ) {
        i++;
    }
    if( i > 0 ) {
        ret[ i - 1 ] = 0;
    }

    return ret;
}

/*!
 * \brief Checks if the given directory exists. If not it attempts to create
 *        it and if it was unable to do that it will raise an exception.
 * \param dir The directory to check/create.
 */
void Logger::chk_logdir(std::string dir)
{
#ifdef USE_RCPP
    // Load R functions for path management
    Rcpp::Environment base("package:base");
    Rcpp::Environment utils("package:utils");
    Rcpp::Function dirCreate = base["dir.create"];
    Rcpp::Function file_test = utils["file_test"];
    // use file_test -d to see if dir exists and is a directory
    if(!Rcpp::as<bool>(file_test("-d", dir))) {
        // either does not exist or is a file
        // we can try to create it and if it still fails it must
        // have been a file or a permissions error
        if(!Rcpp::as<bool>(dirCreate(dir))) {
            // no luck, throw exception
            H_THROW("Directory "+dir+" does not exist and could not create it.");
        }
    }
#else
    fs::path fs_dir(dir);
    // first check to see if dir exists and is a directory
    if(!fs::is_directory(fs_dir)) {
        // either does not exist or is a file
        // we can try to create it and if it still fails it must
        // have been a file or a permissions error
        boost::system::error_code status;
        if(!fs::create_directory(fs_dir, status)) {
            H_THROW("Directory "+dir+" does not exist and could not create it.");
        }
    }
#endif
}

//------------------------------------------------------------------------------
/*! \brief Print a standard header at the top of all logs that indicates the model version.
 *  \param writeLevel The priority level to print the header.
 */
void Logger::printLogHeader( const LogLevel writeLevel ) {
    H_ASSERT( isInitialized, "Logger must be initialized" );

    H_LOG( (*this), writeLevel ) << MODEL_NAME << " version " << MODEL_VERSION << endl;
}

}
