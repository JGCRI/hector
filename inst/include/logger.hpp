/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <boost/iostreams/filtering_stream.hpp>
#pragma clang diagnostic pop

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
  enum LogLevel { DEBUG, NOTICE, WARNING, SEVERE };

private:
  // Make the copy constructs private and undefined to disallow multiple
  // instances of the same log file.
  Logger(const Logger &);
  Logger &operator=(const Logger &);

  //! The minimum log level which will be processed.
  LogLevel minLogLevel;

  //! Flag to indicate that this logger has been successfully opened.
  bool isInitialized;

  //! Flag to indicate that this logger writes into a log file.
  bool echoToFile;

  //! Flag to indicate that this logger is enabled.
  //! If false this logger does not log regardless of log level provided.
  bool enabled;

  //! The actual output stream which will handle the logging.
  boost::iostreams::filtering_ostream loggerStream;

  static const std::string &logLevelToStr(const LogLevel logLevel);

  static const char *getDateTimeStamp();

  void printLogHeader(const LogLevel logLevel);

public:
  Logger();
  ~Logger();

  void open(const std::string &logName, bool echoToScreen, bool echoToFile,
            LogLevel minLogLevel);

  bool shouldWrite(const LogLevel writeLevel) const;

  std::ostream &write(const LogLevel writeLevel,
                      const std::string &functionInfo);

  void close();

  LogLevel getMinLogLevel() const { return minLogLevel; }

  bool getEchoToFile() const { return echoToFile; }

  bool isEnabled() const { return enabled; }
};

} // namespace Hector

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
#define H_LOG(log, level)                                                      \
  if (log.shouldWrite(level))                                                  \
  log.write(level, __func__)

#endif
