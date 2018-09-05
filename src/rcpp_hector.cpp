#include <Rcpp.h>
#include <fstream>
#include <sstream>
#include "hector.hpp"
#include "logger.hpp"

using namespace Rcpp;

// [[Rcpp::plugins("cpp11")]]

//' Create and initialize a new hector core
//'
//' The object returned is a handle to the newly created core.  It will be required as an
//' argument for all functions that operate on the core.  Creating multiple cores
//' simultaneously is supported.
//'
//' @param infile (String) name of the hector input file.
//' @return handle for the Hector core.
//' @export
// [[Rcpp::export]]
List newcore(String inifile)
{
    try {
        // Check that the configuration file exists. The easiest way to do
        // this is to try to open it.
        std::ifstream ifs(inifile);      // we'll use this to test if the file exists
        if(ifs) {
            ifs.close();            // don't actually want to read from it
        }
        else {
            std::string fn = inifile;
            Rcpp::stop(std::string("Input file ") + fn + std::string(" does not exist."));
        }

        // Initialize the global logger
        Hector::Logger &glog = Hector::Logger::getGlobalLogger();
        glog.open("hector.log", true, true, Hector::Logger::DEBUG);
        H_LOG(glog, Hector::Logger::DEBUG) << "Hector logger initialized" << std::endl;
        

        // Create and initialize the core.
        int coreidx = Hector::Core::mkcore();
        Hector::Core *hcore = Hector::Core::getcore(coreidx);
        hcore->init();

        Rcout << "Core initialized\n";

        try {
            Hector::INIToCoreReader coreParser(hcore);
            coreParser.parse(inifile);
            Rcout << "Core parser run\n";
        }
        catch(h_exception e) {
            std::stringstream msg;
            msg << "While parsing hector input file: " << e;
            Rcpp::stop(msg.str());
        }


        // The Following three lines of code are occasionally useful for
        // generating debugging output; however, they leak memory.
        // Therefore, they should only be used for short tests where
        // you need the CSV output to compare to a benchmark run.
        // TODO:  Remove these before release.
        // std::ofstream *output = new std::ofstream("rcpp-test-output.csv");
        // Hector::CSVOutputStreamVisitor *csvosv = new Hector::CSVOutputStreamVisitor(*output);
        // hcore->addVisitor(csvosv);

        // Run the last bit of setup
        hcore->prepareToRun();

        // Construct the object we are going to return to R
        double strtdate = hcore->getStartDate();
        double enddate = hcore->getEndDate();

        List rv= List::create(coreidx, strtdate, enddate, true);
        rv.attr("class") = "hcore";
        return rv;
    }
    catch(h_exception e) {
        std::stringstream msg;
        msg << "During hector core setup: " << e;
        Rcpp::stop(msg.str());
    }
}

//' Shutdown a hector core
//'
//' Shutting down a core will free the core itself and all of the objects it created. Any attempted
//' operation on the core after that will raise an error.
//'
//' @param core Handle to the core that is to be shut down.
//' @return The core handle, modified to show that it is no longer active.
//' @export
// [[Rcpp::export]]
List shutdown(List core)
{
    // TODO: check that the list supplied is an hcore object

    int idx = core[0];
    Hector::Core::delcore(idx);

    core[3] = false;

    return core;
}


//' Run the Hector climate model
//'
//' Run Hector up through the specified time.  This function does not return the results
//' of the run.  To get results, run \code{fetch}.
//'
//' @param core Handle to the hector core that is to be run.
//' @param runtodate Date to run to.  The default is to run to the end date configured
//' in the input file used to initialize the core.
//' @export
// [[Rcpp::export]]
void run(List core, double runtodate=-1.0)
{
    int idx = core[0];
    Hector::Core *hcore = Hector::Core::getcore(idx);
    if(!hcore) {
        Rcpp::stop("hector::run:  invalid index");
    }
    try {
        hcore->run(runtodate);
    }
    catch(h_exception e) {
        std::stringstream msg;
        msg << "Error while running hector:  " << e;
        Rcpp::stop(msg.str());
    }
}
