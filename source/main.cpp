/*
 *  main.cpp - wrapper entry point
 *  hector
 *
 *  Created by Ben on 9/14/10.
 *
 */

#include <iostream>

#include "core/core.hpp"
#include "core/logger.hpp"
#include "h_exception.hpp"
#include "h_util.hpp"
#include "input/h_reader.hpp"
#include "input/ini_to_core_reader.hpp"
#include "visitors/csv_output_visitor.hpp"
#include "visitors/csv_outputstream_visitor.hpp"

#include "data/unitval.hpp"

using namespace std;

//-----------------------------------------------------------------------
/*! \brief Entry point for HECTOR wrapper.
 *
 *  Starting point for wrapper, not the core.
 */
int main (int argc, char * const argv[]) {
    using namespace Hector;
    
	try {
        // Create the global log
        Logger& glog = Logger::getGlobalLogger();
        glog.open( string( MODEL_NAME ), true, Logger::DEBUG );
        H_LOG( glog, Logger::NOTICE ) << MODEL_NAME << " wrapper start" << endl;
        
        // Parse the main configuration file
        if( argc > 1 ) {
            if( ifstream( argv[1] ) ) {
                h_reader reader( argv[1], INI_style );
            } else {
                H_LOG( glog, Logger::SEVERE ) << "Couldn't find input file " << argv[ 1 ] << endl;
                H_THROW( "Couldn't find input file" )
            }
        } else {
            H_LOG( glog, Logger::SEVERE ) << "No configuration filename!" << endl;
            H_THROW( "Usage: <program> <config file name>" )
        }
        
        // Initialize the core and send input data to it
        H_LOG( glog, Logger::NOTICE ) << "Creating and initializing the core." << endl;
        Core core;
        core.init();
        
        H_LOG( glog, Logger::NOTICE ) << "Setting data in the core." << endl;
        INIToCoreReader coreParser( &core );
        coreParser.parse( argv[1] );
        
        // Create visitors
        H_LOG( glog, Logger::NOTICE ) << "Adding visitors to the core." << endl;
        CSVOutputVisitor csvOutputVisitor( string( OUTPUT_DIRECTORY ) + "output.csv"  );
        core.addVisitor( &csvOutputVisitor );
        filebuf csvoutputStreamFile;
        
        // Open the stream output file, which has an optional run name (specified in the INI file) in it
        string rn = core.getRun_name();
        if( rn == "" )
            csvoutputStreamFile.open( string( string( OUTPUT_DIRECTORY ) + "outputstream.csv" ).c_str(), ios::out );
        else
            csvoutputStreamFile.open( string( string( OUTPUT_DIRECTORY ) + "outputstream_" + rn + ".csv" ).c_str(), ios::out );
        
        
        ostream outputStream( &csvoutputStreamFile );
        CSVOutputStreamVisitor csvOutputStreamVisitor( outputStream );
        core.addVisitor( &csvOutputStreamVisitor );

        H_LOG(glog, Logger::NOTICE) << "Calling prepareToRun()\n";
        core.prepareToRun();

        H_LOG( glog, Logger::NOTICE ) << "Running the core." << endl;
        core.run();
        
        H_LOG( glog, Logger::NOTICE ) << "Hector wrapper end" << endl;
        glog.close();
    }
    // catch( h_exception e ) {
    //     cerr << "* Program exception: " << e.msg << "\n* Function " << e.func << ", file "
    //     << e.file << ", line " << e.linenum << endl;
    // }
    catch( std::exception &e ) {
        cerr << "Standard exception: " << e.what() << endl;
    }
    // catch( ... ) {
    //     cerr << "Other exception! " << endl;
    // }

    return 0;
}
