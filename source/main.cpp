/*
 *  main.cpp - wrapper entry point
 *  hector
 *
 *  Created by Ben on 9/14/10.
 *
 */

#include <stdlib.h>
#include <iostream>
#include "boost/algorithm/string.hpp"

#include "core/core.hpp"
#include "core/logger.hpp"
#include "h_exception.hpp"
#include "h_util.hpp"
#include "input/h_reader.hpp"
#include "input/ini_to_core_reader.hpp"
#include "visitors/ini_restart_visitor.hpp"
#include "visitors/csv_output_visitor.hpp"
#include "visitors/csv_outputstream_visitor.hpp"

#include "data/unitval.hpp"
#include "data/message_data.hpp"
#include "components/component_names.hpp"
#include "components/imodel_component.hpp"

using namespace std;
using namespace Hector; 

void read_and_set_co2(double tlast,double t, Core &core, istream &sim_gcam_emiss);
void init_emiss_strm(istream &sim_gcam_emiss);

//-----------------------------------------------------------------------
/*! \brief Entry point for HECTOR wrapper.
 *
 *  Starting point for wrapper, not the core.
 */
int main (int argc, char * const argv[]) {
  
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
        INIRestartVisitor restartVisitor( string( OUTPUT_DIRECTORY ) + "restart.ini", core.getEndDate() );
        core.addVisitor( &restartVisitor );
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

        // variables for reading CO2 emissions to pass into the core as it's running.
        double tlast = core.getStartDate();
        ifstream sim_gcam_emiss("input/emissions/RCP45_emissions.csv");
        init_emiss_strm(sim_gcam_emiss);
        
        for(double t=core.getStartDate()+5.0; t<=core.getEndDate(); t+=5.0) {
            read_and_set_co2(tlast, t, core, sim_gcam_emiss);
            core.run(t);
            // The following provide an example of how to request data
            // from components.  Note you don't need to get the name
            // of the component; you just need to say what kind of
            // data you want, and the core takes care of the rest.
            unitval temp = core.sendMessage(M_GETDATA, D_GLOBAL_TEMP);
            unitval ca   = core.sendMessage(M_GETDATA, D_ATMOSPHERIC_CO2);
            unitval forc = core.sendMessage(M_GETDATA, D_RF_TOTAL);
            H_LOG(glog, Logger::NOTICE)
                << "t= " << t << "\t"
                << "temp= " << temp << "\t"
                << "atmos. C= " << ca << "\t"
                << "RF= " << forc << endl;
            // logging doesn't seem to be working properly right now, so repeat
            // the info to stdout
            cout << "t= " << t << "\t"
                 << "temp= " << temp << "\t"
                 << "atmos. C= " << ca << "\t"
                 << "RF= " << forc << endl;

            tlast = t;
        }

        H_LOG(glog, Logger::NOTICE) << "Shutting down all components.\n";
        core.shutDown();
        
        H_LOG( glog, Logger::NOTICE ) << "Hector wrapper end" << endl;
        glog.close();
	}
	catch( h_exception e ) {
		cerr << "* Program exception: " << e.msg << "\n* Function " << e.func << ", file "
        << e.file << ", line " << e.linenum << endl;
	}
	catch( std::exception &e ) {
		cerr << "Standard exception: " << e.what() << endl;
	}
    catch( ... ) {
        cerr << "Other exception! " << endl;
    }

    return 0;
}
