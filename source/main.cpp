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
            h_reader reader( argv[1], INI_style ); }
        else {
            H_LOG( glog, Logger::SEVERE ) << "No configuration filename!" << endl;
            H_THROW( "Usage: <program> <config file name>" )
        }
        
        H_LOG( glog, Logger::NOTICE ) << "Creating and initializing the core." << endl;
        Core core;
        core.init();
        
        H_LOG( glog, Logger::NOTICE ) << "Setting data in the core." << endl;
        INIToCoreReader coreParser( &core );
        coreParser.parse( argv[1] );
        
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
            unitval n2o  = core.sendMessage(M_GETDATA, D_ATMOSPHERIC_N2O, t);
            H_LOG(glog, Logger::NOTICE)
                << "t= " << t << "\t"
                << "temp= " << temp << "\t"
                << "atmos. C= " << ca << "\t"
                << "RF= " << forc << "\t"
                << "Na= " << n2o 
                << endl;
            // logging doesn't seem to be working properly right now, so repeat
            // the info to stdout
            cout 
                << "t= " << t << "\t"
                << "temp= " << temp << "\t"
                << "atmos. C= " << ca << "\t"
                << "RF= " << forc << "\t"
                << "Na= " << n2o 
                << endl;

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
        cerr << "\nRun completed. Press [ENTER] to finish." << endl;
	cin.get();
    return 0;
}


void read_and_set_co2(double tstrt, double tend, Core &core, istream &sim_gcam_emiss)
{
  
    double t;
    std::string line;
    std::vector<std::string> splitvec;
    do {
        getline(sim_gcam_emiss, line);
        boost::split(splitvec, line, boost::algorithm::is_any_of(","));
        t = atof(splitvec[0].c_str());
        if(t>=tstrt && t>2010.0) {
            double anthro = atof(splitvec[1].c_str());
            double luc    = atof(splitvec[2].c_str());
            double so2    = atof(splitvec[8].c_str());
            double bc     = atof(splitvec[14].c_str());
            double oc     = atof(splitvec[16].c_str());
            double cf4    = atof(splitvec[18].c_str());
            double hcf22  = atof(splitvec[37].c_str());
            double nmvoc  = atof(splitvec[10].c_str());

            // This is how you set annual emissions into the model
            core.sendMessage(M_SETDATA, D_ANTHRO_EMISSIONS,
                             message_data(t, unitval(anthro, U_PGC_YR)));
            core.sendMessage(M_SETDATA, D_LUC_EMISSIONS,
                             message_data(t, unitval(luc, U_PGC_YR)));
            core.sendMessage(M_SETDATA, D_EMISSIONS_SO2,
                             message_data(t, unitval(so2, U_GG)));
            core.sendMessage(M_SETDATA, D_EMISSIONS_BC,
                             message_data(t, unitval(bc, U_KG)));
            core.sendMessage(M_SETDATA, D_EMISSIONS_OC,
                             message_data(t, unitval(oc, U_KG)));
            core.sendMessage(M_SETDATA, D_EMISSIONS_CF4,
                             message_data(t, unitval(cf4, U_GG)));
            core.sendMessage(M_SETDATA, D_EMISSIONS_HCF22,
                             message_data(t, unitval(hcf22, U_GG)));
            core.sendMessage(M_SETDATA, D_EMISSIONS_NMVOC,
                              message_data(t, unitval(nmvoc, U_TG_NMVOC)));
            std::cout << "t= " << t << "\n"
                      << "\t\tanthro= " << anthro << "\n"
                      << "\t\tluc= " << luc << "\n"
                      << "\t\tSO2= " << so2 << "\n"
                      << "\t\tBC= "  << bc << "\n"
                      << "\t\tOC= "  << oc << "\n"
                      << "\t\tCF4= " << cf4 << "\n"
                      << "\t\tHCF22= " << hcf22 << "\n"
                      << "\t\tNMVOC= " << nmvoc << "\n"
              ;
        }
    } while(t < tend);
    // when t >=  tend, we exit, leaving tend+1 in the stream to read next time. 
}

void init_emiss_strm(istream &sim_gcam_emiss)
{
    // skip the 4 lines of header information at the beginning of the file
    std::string junk;
    for(int i=0; i<4; ++i) {
        getline(sim_gcam_emiss, junk); 
    }
}

