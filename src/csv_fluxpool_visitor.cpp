/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  csv_fluxpool_visitor.cpp
 *  hector
 *
 *  Created by Skylar Gering on 21 January 2021.
 *
 */

#include <fstream>
#include <boost/lexical_cast.hpp>
#include "core.hpp"
#include "simpleNbox.hpp"
#include "csv_fluxpool_visitor.hpp"
#include "unitval.hpp"
#include "fluxpool.hpp"
#include "h_util.hpp"

namespace Hector {

using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 *  \param filename The file to write the csv output to.
 */
CSVFluxPoolVisitor::CSVFluxPoolVisitor( ostream& outputStream, const bool printHeader )
:csvFile( outputStream )
{
    if( printHeader ) {
        // Print table header
        csvFile << "year" << DELIMITER
                << "pool_name" << DELIMITER << "pool_value" << DELIMITER << "pool_units" << DELIMITER
                << "source_name" << DELIMITER << "source_fraction" <<endl;
    }
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
CSVFluxPoolVisitor::~CSVFluxPoolVisitor() {
}

//------------------------------------------------------------------------------
// documentation is inherited
bool CSVFluxPoolVisitor::shouldVisit( const bool in_spinup, const double date ) {
    // visit all non-spinup model periods
    datestring = boost::lexical_cast<string>( date );
    return !in_spinup;
}

//------------------------------------------------------------------------------
// documentation is inherited
void CSVFluxPoolVisitor::visit( Core* c ) {
    run_name = c->getRun_name();
    core = c;
}

//------------------------------------------------------------------------------
/*! \brief Print the sources, and associated fractions, of a SimpleNBox pool
 */
void CSVFluxPoolVisitor::print_pool(fluxpool x) {
    if(x.tracking) {
        vector<string> sources = x.get_sources();
        for (auto &s: sources) {
            csvFile << datestring << DELIMITER <<
                x.name << DELIMITER << x.value(U_PGC) << DELIMITER << x.unitsName() << DELIMITER <<
                s << DELIMITER << x.get_fraction(s) << endl;
        }
        csvFile << endl;
    }
}

//------------------------------------------------------------------------------
/*! \brief Print a double value and its name
 */
void CSVFluxPoolVisitor::print_diff(double x, string name){
     csvFile << datestring << DELIMITER <<
                "Diff" << DELIMITER << x << DELIMITER << "U_PGC" << DELIMITER <<
                name << DELIMITER << 0 << endl;
}

//------------------------------------------------------------------------------
// documentation is inherited
void CSVFluxPoolVisitor::visit( SimpleNbox* c ) {
    if( !core->outputEnabled( c->getComponentName() ) ) return;

    // The potentially tracked pools
    print_pool( c->atmos_c );
    print_pool( c->earth_c );
    print_pool( c->ocean_model_c );
    for( auto it = c->biome_list.begin(); it != c->biome_list.end(); it++ ) {
        std::string biome = *it;
        print_pool( c->veg_c[ biome ] );
        print_pool( c->detritus_c[ biome ] );
        print_pool( c->soil_c[ biome ] );

        // Temporary Print Outs
        print_diff(c->veg_diff[biome], "veg_diff");
        print_diff(c->soil_diff[biome], "soil_diff");
        print_diff(c->det_diff[biome], "det_diff");
    }
    print_diff(c->atmos_diff, "atmos_diff");
    print_diff(c->earth_diff, "earth_diff");
    csvFile << endl;
}

}
