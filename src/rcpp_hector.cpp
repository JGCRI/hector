#include <Rcpp.h>
#include <fstream>
#include <sstream>

#include "hector.hpp"
#include "logger.hpp"
#include "message_data.hpp"

using namespace Rcpp;

// [[Rcpp::plugins("cpp11")]]


/* Non exported helper functions
 * These are intended for use by the C++ wrappers and are not callable from R directly.
 */

// Get a pointer to a core from the R handle. 
Hector::Core *gethcore(Environment core)
{
    int idx = core["coreidx"];
    Hector::Core *hcore = Hector::Core::getcore(idx);
    if(!hcore) {
        Rcpp::stop("Invalid or inactive hcore object");
    }
    return hcore;
}

// This is the C++ implementation of the core constructor.  It should only ever
// be called from the `newcore` wrapper function.
// [[Rcpp::export]]
Environment newcore_impl(String inifile, int loglevel, bool suppresslogging, String name)
{
    try {
        // Check that the configuration file exists. The easiest way to do
        // this is to try to open it.
	std::ifstream ifs(inifile.get_cstring());      // we'll use this to test if the file exists
        if(ifs) {
            ifs.close();            // don't actually want to read from it
        }
        else {
            std::string fn = inifile;
            Rcpp::stop(std::string("Input file ") + fn + std::string(" does not exist."));
        }

        // Create and initialize the core.
        int coreidx = Hector::Core::mkcore(!suppresslogging,
                                           (Hector::Logger::LogLevel)loglevel,
                                           false);
                                           
        Hector::Core *hcore = Hector::Core::getcore(coreidx);
        hcore->init();

        try {
            Hector::INIToCoreReader coreParser(hcore);
            coreParser.parse(inifile);
        }
        catch(h_exception e) {
            std::stringstream msg;
            msg << "While parsing hector input file: " << e;
            Rcpp::stop(msg.str());
        }

        // Run the last bit of setup
        hcore->prepareToRun();

        // Construct the object we are going to return to R
        double strtdate = hcore->getStartDate();
        double enddate = hcore->getEndDate();

        Environment rv(new_env());
        rv["coreidx"] = coreidx;
        rv["strtdate"] = strtdate;
        rv["enddate"] = enddate;
        rv["inifile"] = inifile;
        rv["name"] = name;
        rv["clean"] = true;
        rv["reset_date"] = 0;
        
        return rv;
    }
    catch(h_exception e) {
        std::stringstream msg;
        msg << "During hector core setup: " << e;
        Rcpp::stop(msg.str());
    }
}


//' Shutdown a hector instance
//'
//' Shutting down an instance will free the instance itself and all of the objects it created. Any attempted
//' operation on the instance after that will raise an error.
//'
//' @section Caution:
//' This function should be called as \code{mycore <- shutdown(mycore)} so that the change
//' from active to inactive will be recorded in the caller.
//'
//' @param core Handle to a Hector instance
//' @return The Hector instance handle
//' @export
//' @family main user interface functions
// [[Rcpp::export]]
Environment shutdown(Environment core)
{
    int idx = core["coreidx"];
    Hector::Core::delcore(idx);

    return core;
}


//' Reset a Hector instance to an earlier date
//'
//' Resetting the model returns it to its state at a previous time.  If the requested time
//' is before the model start date (any value will do; conventionally zero is used), then
//' the spinup will be rerun, and the model will be
//' left ready to run at the start date.  (By contrast, resetting \emph{to} the start
//' date leaves the model ready to run at the start date, but without having rerun the
//' spinup.)
//'
//' @param core Handle for the Hector instance that is to be reset.
//' @param date Date to reset to.  The default is to reset to the model start date with
//' a rerun of the spinup.
//' @family main user interface functions
//' @export
// [[Rcpp::export]]
Environment reset(Environment core, double date=0)
{
    Hector::Core *hcore = gethcore(core);
    try {
        hcore->reset(date);
    }
    catch(h_exception e) {
        std::stringstream msg;
        msg << "Error resetting to date= " << date
            << " :  " << e;
        Rcpp::stop(msg.str());
    }

    double rd = core["reset_date"];
    if(date <= rd)
        core["clean"] = true;
    
    return core;
}


//' Run the Hector climate model
//'
//' Run Hector up through the specified time.  This function does not return the results
//' of the run.  To get results, run \code{fetch}.
//'
//' @param core Handle to the Hector instance that is to be run.
//' @param runtodate Date to run to.  The default is to run to the end date configured
//' in the input file used to initialize the core.
//' @return The Hector instance handle
//' @export
//' @family main user interface functions
// [[Rcpp::export]]
Environment run(Environment core, double runtodate=-1.0)
{
    if(!core["clean"])
        reset(core, core["reset_date"]);

    Hector::Core *hcore = gethcore(core);
    if(runtodate > 0 && runtodate < hcore->getCurrentDate()) {
        std::stringstream msg;
        msg << "Requested run date " << runtodate << " is prior to the current date of "
            << hcore->getCurrentDate() << ". Run reset() to reset to an earlier date.";
        Rcpp::stop(msg.str());
    }

    try {
        hcore->run(runtodate);
    }
    catch(h_exception e) {
        std::stringstream msg;
        msg << "Error while running hector:  " << e;
        Rcpp::stop(msg.str());
    }

    return core;
}


//' \strong{getdate}: Get the current date for a Hector instance
//'
//' @rdname hectorutil
//' @export
// [[Rcpp::export]]
double getdate(Environment core)
{
    Hector::Core *hcore = gethcore(core);
    return hcore->getCurrentDate();
}

//' Retrieve the current list of biomes for a Hector instance
//'
//' @param core Handle to the Hector instance from which to retrieve
//'   the biome list.
//' @export
// [[Rcpp::export]]
std::vector<std::string> get_biome_list(Environment core)
{
    Hector::Core *hcore = gethcore(core);
    std::vector<std::string> biome_list = hcore->getBiomeList();
    return biome_list;
}

//' Create a biome
//'
//' @param core Handle to the Hector instance that is to be run.
//' @param biome (character) Name of new biome
// [[Rcpp::export]]
Environment create_biome_impl(Environment core, std::string biome)
{
    Hector::Core *hcore = gethcore(core);
    hcore->createBiome(biome);
    return core;
}

//' Delete a biome
//'
//' @param core Handle to the Hector instance that is to be run.
//' @param biome (character) Name of biome to delete
// [[Rcpp::export]]
Environment delete_biome_impl(Environment core, std::string biome)
{
    Hector::Core *hcore = gethcore(core);
    hcore->deleteBiome(biome);
    return core;
}

//' Rename an existing biome
//'
//' This will create a new biome called `newname`, assign it all of
//' the C stocks and parameter values from biome `oldname`, and delete
//' biome `oldname`.
//'
//' @param core Handle to the Hector instance that is to be run.
//' @param oldname (character) Name of existing biome to be replaced
//' @param newname (character) Name of new biome
//' @export
// [[Rcpp::export]]
Environment rename_biome(Environment core, std::string oldname, std::string newname)
{
    Hector::Core *hcore = gethcore(core);
    hcore->renameBiome(oldname, newname);
    return core;
}


//' Send a message to a Hector instance
//'
//' Messages are the mechanism used to get data from Hector model components and
//' to set values within components.
//' 
//' A message comprises a type (e.g. GETDATA to retrieve data from a component, or SETDATA to
//' set data in a component), a capability, which identifies the information to be operated
//' on (e.g. Atmospheric CO2 concentration, or global total radiative forcing), and an optional
//' structure of extra data (comprising a date and a numerical value with units).
//'
//' The arguments to this function are organized in a slightly more R-friendly way.  The message
//' type and capability are each passed as a single string.  The date portion of the extra
//' data is passed as a numeric vector (one message will be generated for each date).  The value
//' portion of the extra data is a numeric vector with a length of either 1 or the same length
//' as the date vector.  The units portion is a single string (we don't support sending a vector
//' of values with heterogeneous units in a single call.
//'
//' Either the date or the value (or both) may be NA.  The date should be NA in cases where the
//' parameter being referenced doesn't change with time.  The value should be NA in cases where
//' the optional data will be ignored.
//'
//' @param core a Hector instance
//' @param msgtype (String) type of message. Usually either GETDATA or SETDATA
//' @param capability (String) capability being targeted by the message.  The core will use
//' this information to look up the component to route the message to.
//' @param date (NumericVector) Date for which to set/get the variable.  Use NA if there is
//' no applicable date.
//' @param value (NumericVector) Numeric portion of the optional data (in case of setting
//' a value this will be the value to set).  The length of this vector should match that of
//' the time, or it should be length 1 (in which case it will be recycled).
//' @param unit (String) Unit for the value vector.
//' @export
// [[Rcpp::export]]
DataFrame sendmessage(Environment core, String msgtype, String capability, NumericVector date,
                      NumericVector value, String unit)
{
    Hector::Core *hcore = gethcore(core);
    
    if(value.size() != date.size() && value.size() != 1) {
        Rcpp::stop("Value must have length 1 or same length as date.");
    }

    int N = date.size();
    std::string msgstr = msgtype;
    std::string capstr = capability;

    // We need to convert the unit string into an enumerated type
    std::string unitstr = unit;
    Hector::unit_types utype;
    try {
        utype = Hector::unitval::parseUnitsName(unitstr);
    }
    catch(h_exception e) {
        // std::stringstream emsg;
        // emsg << "sendmessage: invalid unit type: " << unitstr;
        // Rcpp::stop(emsg.str());
        utype = Hector::U_UNDEFINED;
    }

    NumericVector valueout(N);
    StringVector unitsout(N);

    try {
        for(size_t i=0; i<N; ++i) {
            // Construct the inputs to sendmessage
            int ival;               // location of the value we're looking for
            if(value.size() == 1)
                ival = 0;
            else
                ival = i;
            
            double tempval;
            if(NumericVector::is_na(value[ival]))
                tempval = 0;
            else
                tempval = value[ival];
            
            double tempdate;
            if(NumericVector::is_na(date[i]))
                tempdate = Hector::Core::undefinedIndex();
            else
                tempdate = date[i];
            
            Hector::message_data info(tempdate, Hector::unitval(tempval, utype));
            
            Hector::unitval rtn = hcore->sendMessage(msgstr, capstr, info);
            
            unitsout[i] = rtn.unitsName();
            valueout[i] = rtn.value(rtn.units());
        }
    }
    catch(h_exception e) {
        std::stringstream emsg;
        emsg << "sendmessage: " << e;
        Rcpp::stop(emsg.str());
    }

    // Assemble a data frame with the results: date, var, value, units
    DataFrame result =
        DataFrame::create(Named("year")=date, Named("variable")=capability,
                          Named("value")=valueout,
                          Named("units")=unitsout,
                          Named("stringsAsFactors")=false);

    return result;
}

// helper for isactive()
// [[Rcpp::export]]
bool chk_core_valid(Environment core)
{
    int idx = core["coreidx"];
    Hector::Core *hcore = Hector::Core::getcore(idx);
    
    return hcore != NULL;
}
