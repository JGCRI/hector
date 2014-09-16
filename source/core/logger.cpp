/*
 *  logger.cpp
 *  hector
 *
 *  Created by Pralit Patel on 9/16/10.
 *
 */

#include <time.h>

#include "core/logger.hpp"

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
    consoleBuf = echoToScreen ? cout.rdbuf() : 0;
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
Logger::Logger()
:isInitialized( false ),
minLogLevel( WARNING ),
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
 * \exception h_exception Exception thrown if the logger has already been
 *                        initialized.
 */
void Logger::open( const string& logName, const bool echoToScreen,
                  LogLevel minLogLevel ) throw ( h_exception )
{
    H_ASSERT( !isInitialized, "This log has already been initialized." );
    
	const string fqName = LOG_DIRECTORY + logName + LOG_EXTENSION;	// fully-qualified name
	
    this->minLogLevel = minLogLevel;
    
    // TODO: does the logger or the ostream manage this memory?
    // FIXME:  currently nobody does.  The memory is leaked.
    LoggerStreamBuf* buff = new LoggerStreamBuf( echoToScreen );
    buff->open( fqName.c_str(), ios::out );
    
    loggerStream.rdbuf( buff );
    isInitialized = true;
}

//------------------------------------------------------------------------------
/*! \brief Indicate whether a message at the given priority will be logged.
 *  \param writeLevel The priority level to check.
 *  \return True if this level would be logged, false otherwise.
 */
bool Logger::shouldWrite( const LogLevel writeLevel ) const {
    return writeLevel >= minLogLevel;
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
                       const string& functionInfo ) throw ( h_exception )
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
        LoggerStreamBuf *lsbuf = static_cast<LoggerStreamBuf*>( loggerStream.rdbuf() );
        lsbuf->close();
        /*! \note Setting isInitialized back to false will allow this logger to
         *        be reopened.
         */
        isInitialized = false;
        delete lsbuf;
    }
}

//------------------------------------------------------------------------------
/*! \brief Get the instance of the global logger.
 *
 *  There will only be a single instance of the global logger which can be
 *  accessed from anywhere in the code without having to pass it around.
 *
 * \return A reference to the global logger.
 */
Logger& Logger::getGlobalLogger() {
    static Logger globalLogger;
    
    return globalLogger;
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
    // TODO: I think visual studio will complain about the use of these
    time( &rawtime );
    timeinfo = localtime( &rawtime );
    // TODO: come up with our own formatted time output
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
