/* Hector -- A Simple Climate Model
   Copyright (C) 2022  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  main.cpp - wrapper entry point
 *  hector
 *
 *  Created by Ben on 9/14/10.
 *
 */

#include <fstream>
#include <iostream>

#include "core.hpp"
#include "csv_outputstream_visitor.hpp"
#include "csv_tracking_visitor.hpp"
#include "h_exception.hpp"
#include "h_reader.hpp"
#include "h_util.hpp"
#include "ini_to_core_reader.hpp"
#include "logger.hpp"

#include "unitval.hpp"

using namespace std;

//-----------------------------------------------------------------------
/*! \brief Entry point for HECTOR wrapper.
 *
 *  Starting point for wrapper, not the core.
 */
int main(int argc, char *const argv[]) {
  using namespace Hector;

  try {
    // Create the Hector core
    Core core(Hector::Logger::DEBUG, true, true);
    Logger &glog = core.getGlobalLogger();
    H_LOG(glog, Logger::NOTICE) << MODEL_NAME << " wrapper start" << endl;

    // Parse the main configuration file
    if (argc > 1) {
      if (ifstream(argv[1])) {
        H_LOG(glog, Logger::NOTICE) << "Reading input file " << argv[1] << endl;
        h_reader reader(argv[1], INI_style);
      } else {
        H_LOG(glog, Logger::SEVERE)
            << "Couldn't find input file " << argv[1] << endl;
        H_THROW("Couldn't find input file")
      }
    } else {
      H_LOG(glog, Logger::SEVERE) << "No configuration filename!" << endl;
      H_THROW("Usage: <program> <config file name>")
    }

    // Initialize the core and send input data to it
    H_LOG(glog, Logger::NOTICE)
        << "Creating and initializing the core." << endl;
    core.init();

    H_LOG(glog, Logger::NOTICE) << "Setting data in the core." << endl;
    INIToCoreReader coreParser(&core);
    coreParser.parse(argv[1]);

    // Create visitors
    H_LOG(glog, Logger::NOTICE) << "Adding visitors to the core." << endl;
    filebuf csvoutputStreamFile;
    filebuf csvFluxPoolTrackingFile;

    // Open the stream output file, which has an optional run name (specified in
    // the INI file) in it First ensure that OUTPUT_DIRECTORY exists so that we
    // can place our outputs in there Note: this will attempt to create
    // OUTPUT_DIRECTORY if it indeed did not exist and it will throw an
    // exception if it couldn't create it
    ensure_dir_exists(OUTPUT_DIRECTORY);

    string rn = core.getRun_name();
    if (rn == "")
      csvoutputStreamFile.open(
          string(string(OUTPUT_DIRECTORY) + "outputstream.csv").c_str(),
          ios::out);
    else
      csvoutputStreamFile.open(
          string(string(OUTPUT_DIRECTORY) + "outputstream_" + rn + ".csv")
              .c_str(),
          ios::out);
    // Open the simpleNbox tracking output file, similarly named
    if (rn == "")
      csvFluxPoolTrackingFile.open(
          string(string(OUTPUT_DIRECTORY) + "tracking.csv").c_str(), ios::out);
    else
      csvFluxPoolTrackingFile.open(
          string(string(OUTPUT_DIRECTORY) + "tracking_" + rn + ".csv").c_str(),
          ios::out);

    ostream outputStream(&csvoutputStreamFile);
    CSVOutputStreamVisitor csvOutputStreamVisitor(outputStream);
    core.addVisitor(&csvOutputStreamVisitor);

    ostream trackingStream(&csvFluxPoolTrackingFile);
    CSVFluxPoolVisitor csvFluxPoolVisitor(trackingStream);
    core.addVisitor(&csvFluxPoolVisitor);

    H_LOG(glog, Logger::NOTICE) << "Calling prepareToRun()\n";
    core.prepareToRun();

    H_LOG(glog, Logger::NOTICE) << "Running the core." << endl;
    core.run();

    H_LOG(glog, Logger::NOTICE) << "Hector wrapper end" << endl;
    glog.close();
  } catch (h_exception e) {
    cerr << "* Program exception:\n" << e << endl;
    return 1;
  } catch (std::exception &e) {
    cerr << "Standard exception: " << e.what() << endl;
    return 2;
  } catch (...) {
    cerr << "Other exception! " << endl;
    return 3;
  }

  return 0;
}
