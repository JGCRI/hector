/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  csv_trackedpool_visitor.cpp
 *  hector
 *
 *  Created by Skylar Gering on 21 January 2021.
 *
 */

#include <fstream>
#include <boost/lexical_cast.hpp>
#include "core.hpp"
#include "simpleNbox.hpp"
#include "csv_trackedpool_visitor.hpp"
#include "unitval.hpp"
#include "trackedval.hpp"
#include "h_util.hpp"

namespace Hector{

using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 *  \param filename The file to write the csv output to.
 */
CSVTrackedPoolVisitor::CSVTrackedPoolVisitor( ostream& outputStream, const bool printHeader )
:csvFile( outputStream )
{
    if( printHeader ) {
        // Print model version header
        csvFile << "# Output from " << MODEL_NAME << " version " << MODEL_VERSION << endl;

        // Print table header
        csvFile << "year" << DELIMITER << "run_name" << DELIMITER << "spinup" << DELIMITER
             << "pool_name" << DELIMITER << "pool_values" << endl;
    }
    run_name = "";
    current_date = 0;
    datestring = "";
    spinupstring = "";
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
CSVTrackedPoolVisitor::~CSVTrackedPoolVisitor() {
}

//------------------------------------------------------------------------------
// documentation is inherited
bool CSVTrackedPoolVisitor::shouldVisit( const bool is, const double date ) {

    current_date = date;
    in_spinup = is;
    datestring = boost::lexical_cast<string>( date );
    spinupstring = boost::lexical_cast<string>( in_spinup );

    // visit all model periods
    return true;
}

//------------------------------------------------------------------------------
/*! \brief Return text that starts every output line
 */
std::string CSVTrackedPoolVisitor::linestamp() {
    return( datestring + DELIMITER + run_name + DELIMITER + spinupstring + DELIMITER );
}

//------------------------------------------------------------------------------
// documentation is inherited
void CSVTrackedPoolVisitor::visit( Core* c ) {
    run_name = c->getRun_name();
    core = c;
}

// Macro to send a variable with associated unitval units to some output stream
// Takes s (stream), c (component), xname (variable name), x (output variable)
#define STREAM_POOL( s, c, xname, x ) { \
s << linestamp() << DELIMITER << xname << DELIMITER << x << std::endl; \
}

// Macro to send a variable with associated unitval units to some output stream
// This uses new sendMessage interface in imodel_component
// Takes s (stream), c (component), xname (variable name), date
#define STREAM_MESSAGE( s, c, xname ) { \
unitval x = c->sendMessage( M_GETDATA, xname ); \
s << linestamp() << xname << DELIMITER << x << std::endl; \
}
//------------------------------------------------------------------------------
// documentation is inherited
void CSVTrackedPoolVisitor::visit( SimpleNbox* c ) {
    if( !core->outputEnabled( c->getComponentName() ) ) return;

    // Global outputs
    STREAM_MESSAGE( csvFile, c, D_ATMOSPHERIC_C );
    STREAM_MESSAGE( csvFile, c, D_EARTHC );
}

}