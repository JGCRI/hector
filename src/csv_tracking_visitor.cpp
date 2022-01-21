/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  csv_tracking_visitor.cpp
 *  hector
 *
 *  Created by Skylar Gering on 21 January 2021.
 *
 */

#include <fstream>

// some boost headers generate warnings under clang; not our problem, ignore
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <boost/lexical_cast.hpp>
#pragma clang diagnostic pop

#include "core.hpp"
#include "simpleNbox.hpp"
#include "ocean_component.hpp"
#include "csv_tracking_visitor.hpp"
#include "unitval.hpp"
#include "h_util.hpp"

namespace Hector {

using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 *  \param outputStream The file to write the csv output to
 *  \param printHeader Boolean controlling whether we print a header or not
 */
CSVFluxPoolVisitor::CSVFluxPoolVisitor( ostream& outputStream, const bool printHeader ) :
    csvFile( outputStream )
{
    stringstream hdr;
    if( printHeader ) {
        // Store table header
        hdr << "year" << DELIMITER << "component" << DELIMITER << "pool_name" << DELIMITER << "pool_value"
        << DELIMITER << "pool_units" << DELIMITER << "source_name"
        << DELIMITER << "source_fraction" << endl;
    }
    header = hdr.str();
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
CSVFluxPoolVisitor::~CSVFluxPoolVisitor() {
    // Write out the buffer to the csv file before closing down
    
    if(csvBuffer.size()) {
        csvFile << header; // the header (or an empty string)
        for( double yr = csvBuffer.firstdate(); yr <= csvBuffer.lastdate(); yr++ ) {
            csvFile << csvBuffer.get(yr);
        }
    }
}

//------------------------------------------------------------------------------
// documentation is inherited
bool CSVFluxPoolVisitor::shouldVisit( const bool in_spinup, const double date ) {
    current_date = date;
    datestring = boost::lexical_cast<string>( date );
    // visit all model periods that are >= the initial tracking date
    return date >= tracking_date;
}

//------------------------------------------------------------------------------
// documentation is inherited
void CSVFluxPoolVisitor::visit( Core* c ) {
    run_name = c->getRun_name();
    core = c;
    tracking_date = core->getTrackingDate();
}

//------------------------------------------------------------------------------
/*! \brief Print the sources, and associated fractions, of a fluxpool
 */
void CSVFluxPoolVisitor::print_pool( const fluxpool x, const string cname ) {
    if(x.tracking) {
        stringstream output;
        // there might already be "diff" output
        if(csvBuffer.exists(current_date)) {
            output << csvBuffer.get(current_date);
        }
        vector<string> sources = x.get_sources();
        for (auto &s: sources) {
            output << datestring << DELIMITER << cname << DELIMITER << x.name << DELIMITER
            << x.value(U_PGC) << DELIMITER << x.unitsName() << DELIMITER
            << s << DELIMITER << x.get_fraction(s) << endl;
        }
        csvBuffer.set(current_date, output.str());
    }
}

//------------------------------------------------------------------------------
// documentation is inherited
void CSVFluxPoolVisitor::visit( SimpleNbox* c ) {
    if( !core->outputEnabled( c->getComponentName() ) ) return;

    const string cname = c->getComponentName();
    
    // The potentially tracked pools
    print_pool( c->atmos_c, cname );
    print_pool( c->earth_c, cname );
    for( auto it = c->biome_list.begin(); it != c->biome_list.end(); it++ ) {
        std::string biome = *it;
        print_pool( c->veg_c[ biome ], cname );
        print_pool( c->detritus_c[ biome ], cname );
        print_pool( c->soil_c[ biome ], cname );
    }
}

//------------------------------------------------------------------------------
// documentation is inherited
void CSVFluxPoolVisitor::visit( OceanComponent* c ) {
    if( !core->outputEnabled( c->getComponentName() ) ) return;

    const string cname = c->getComponentName();

    print_pool( c->surfaceHL.get_carbon(), cname );
    print_pool( c->surfaceLL.get_carbon(), cname );
    print_pool( c->inter.get_carbon(), cname );
    print_pool( c->deep.get_carbon(), cname );
}

//------------------------------------------------------------------------------
/*! \brief Assemble the time series strings into the given tracking output stream.
 *  \param tracking_out The output stream to write results into.
 */ 
void CSVFluxPoolVisitor::outputTrackingData( ostream& tracking_out ) const {
    if(csvBuffer.size()) {
        tracking_out << header; // the header (or an empty string)        
        for( double yr = csvBuffer.firstdate(); yr <= csvBuffer.lastdate(); yr++ ) {
            tracking_out << csvBuffer.get(yr);
        }
    }
}

//------------------------------------------------------------------------------
// documentation is inherited
void CSVFluxPoolVisitor::reset( const double reset_date ) {
    csvBuffer.truncate( reset_date );
}

}
