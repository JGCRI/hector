/* Hector -- A Simple Climate Model
   Copyright (C) 2022  Battelle Memorial Institute

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

#include "logger.hpp"
#include "h_util.hpp"
#include <algorithm>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/null.hpp>
#include <boost/iostreams/tee.hpp>
#include <time.h>

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
// Methods for Logger

//------------------------------------------------------------------------------
/*! \brief Create an uninitialized logger.
 */
Logger::Logger() : minLogLevel(WARNING), isInitialized(false), loggerStream() {}

//------------------------------------------------------------------------------
/*! \brief Destructor.
 *
 *  The logger will be automatically closed on destruction.
 */
Logger::~Logger() { close(); }

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
void Logger::open(const string &logName, bool echoToScreen, bool echoToFile,
                  LogLevel minLogLevel) {
  H_ASSERT(!isInitialized, "This log has already been initialized.");

  this->minLogLevel = minLogLevel;
  this->echoToFile = echoToFile;

  using namespace boost::iostreams;

  if (echoToScreen) {
    // if the user wants to echo to screen we can use a "tee" to
    // duplicate the output to go to the screen as well as carry
    // on to potentially a file
    tee_filter<ostream> screenEcho(STDOUT_STREAM);
    loggerStream.push(screenEcho);
  }

  if (echoToFile) {
    ensure_dir_exists(LOG_DIRECTORY);

    const string fqName =
        LOG_DIRECTORY + logName + LOG_EXTENSION; // fully-qualified name

    file_sink fileBuff(fqName);
    if (!fileBuff.is_open())
      H_THROW("Unable to open log file " + fqName);

    // add the file sink to the output stream which will serve as an end point
    // for the output data
    loggerStream.push(fileBuff);
  } else {
    // if we are not going to echo to file explicitly set the end point to
    // a null device
    // doing it this way covers the cases:
    //   1) that we are going to echo to screen but not file
    //   2) If a user by passes shouldWrite / H_LOG this will ensure the output
    //      still goes nowhere
    loggerStream.push(null_sink());
  }
  enabled = echoToScreen || echoToFile;

  isInitialized = true;

  // ensure that the log header is always printed.
  printLogHeader(max(minLogLevel, NOTICE));
}

//------------------------------------------------------------------------------
/*! \brief Indicate whether a message at the given priority will be logged.
 *  \param writeLevel The priority level to check.
 *  \return True if this level would be logged, false otherwise.
 */
bool Logger::shouldWrite(const LogLevel writeLevel) const {
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
ostream &Logger::write(const LogLevel writeLevel, const string &functionInfo) {
  H_ASSERT(isInitialized, "can't write to logger until initialized");

  // note that we are not double checking the writeLevel
  return loggerStream << getDateTimeStamp() << ':' << logLevelToStr(writeLevel)
                      << ':' << functionInfo << ": ";
}

//------------------------------------------------------------------------------
/*! \brief Close the logger.
 */
void Logger::close() {
  if (isInitialized) {
    /*! \note Setting isInitialized back to false will allow this logger to
     *        be reopened.
     */
    isInitialized = false;
    loggerStream.reset();
  }
}

//------------------------------------------------------------------------------
/*! \brief Convert the enum to a string so that it can be logged.
 *  \param logLevel The enum to convert.
 *  \return The string representing the given LogLevel.
 */
const string &Logger::logLevelToStr(const LogLevel logLevel) {
  const static string logLevelStrings[] = {"DEBUG", "NOTICE", "WARNING",
                                           "SEVERE"};

  return logLevelStrings[logLevel];
}

//------------------------------------------------------------------------------
/*! \brief Get the current data and time stamp.
 *  \return A string representing the current date and time.
 */
const char *Logger::getDateTimeStamp() {
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  char *ret = asctime(timeinfo);
  // remove the newline at the end of the string
  int i = 0;
  while (ret[i] != 0) {
    i++;
  }
  if (i > 0) {
    ret[i - 1] = 0;
  }

  return ret;
}

//------------------------------------------------------------------------------
/*! \brief Print a standard header at the top of all logs that indicates the
 * model version. \param writeLevel The priority level to print the header.
 */
void Logger::printLogHeader(const LogLevel writeLevel) {
  H_ASSERT(isInitialized, "Logger must be initialized");

  H_LOG((*this), writeLevel)
      << MODEL_NAME << " version " << MODEL_VERSION << endl;
}

} // namespace Hector
