/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  csv_output_visitor.cpp
 *  hector
 *
 *  Created by Pralit Patel on 11/4/10.
 *
 */

#include <fstream>

#include "visitors/csv_output_visitor.hpp"

namespace Hector {
  
using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 *  \param filename The file to write the csv output to.
 */
CSVOutputVisitor::CSVOutputVisitor( const string& filename )
:csvFile( filename.c_str(), ios::out )
{
    // Print model version header
    csvFile << "# Output from " << MODEL_NAME << " version " << MODEL_VERSION << endl;

    // Print table header
    csvFile << "run_name" << DELIMITER << "Year" << DELIMITER << D_ATMOSPHERIC_CO2
        << DELIMITER << D_RF_TOTAL << std::endl;
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
CSVOutputVisitor::~CSVOutputVisitor() {
    csvFile.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
bool CSVOutputVisitor::shouldVisit( const bool in_spinup, const double date ) {

    currDate = date;
    
    // CSV output doesn't occur in spinup
    return !in_spinup;
}

//------------------------------------------------------------------------------
// documentation is inherited
void CSVOutputVisitor::visit( Core* c ) {
    csvFile << c->getRun_name();
    csvFile << DELIMITER << currDate;
    csvFile << DELIMITER << c->sendMessage( M_GETDATA, D_ATMOSPHERIC_CO2 ).value( U_PPMV_CO2 );
    csvFile << DELIMITER << c->sendMessage( M_GETDATA, D_RF_TOTAL ).value( U_W_M2 );
    csvFile << std::endl;
}

}
