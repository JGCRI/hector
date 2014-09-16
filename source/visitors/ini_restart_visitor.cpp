/*
 *  ini_restart_visitor.cpp
 *  hector
 *
 *  Created by Pralit Patel on 11/1/10.
 *
 */

#include <fstream>

#include "components/forcing_component.hpp"
#include "components/halocarbon_component.hpp"
#include "core/core.hpp"
#include "data/tseries.hpp"
#include "h_util.hpp"
#include "models/simpleNbox.hpp"
#include "visitors/ini_restart_visitor.hpp"

using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 *  \param filename The file to write the restart file to.
 */
INIRestartVisitor::INIRestartVisitor( const string& filename, const double restartDate )
:restartFile( filename.c_str(), ios::out ),
restartDate( restartDate )
{
    restartFile.precision( 10 );
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
INIRestartVisitor::~INIRestartVisitor() {
    restartFile.close();
}

bool INIRestartVisitor::shouldVisit( const bool in_spinup, const double date ) {
    // hmm maybe we will need something more sophisticated so we can know things
    // like start, end, and time-steps
    return date == restartDate;
}

//------------------------------------------------------------------------------
// documentation is inherited
void INIRestartVisitor::visit( Core* c ) {
    restartFile << "[" << c->getComponentName() << "]" << endl;
    restartFile << D_START_DATE << "= " << restartDate << endl;
}

//------------------------------------------------------------------------------
// documentation is inherited
void INIRestartVisitor::visit( SimpleNbox* c ) {
/*
 // Commented out BBL 1/13/14 - isn't getting used. Can we delete restart visitor?
 restartFile << "[" << c->getComponentName() << "]" << endl;
    restartFile << D_EARTHC << "= " << c->earth_c.value( U_PGC ) << endl;
    restartFile << D_ATMOSPHERIC_C << "= " << c->atmos_c.value( U_PGC ) << endl;
    restartFile << D_VEGC << "= " << c->sum_map( veg_c ).value( U_PGC ) << endl;
    restartFile << D_DETRITUSC << "= " << c->sum_map( detritus_c ).value( U_PGC ) << endl;
    restartFile << D_SOILC << "= " << c->sum_map( soil_c ).value( U_PGC ) << endl;
*/
}

//------------------------------------------------------------------------------
// documentation is inherited
void INIRestartVisitor::visit( ForcingComponent* c ) {
    restartFile << "[" << c->getComponentName() << "]" << endl;
    // Forcing does not currently have any state necessary for restart.
}

//------------------------------------------------------------------------------
// documentation is inherited
void INIRestartVisitor::visit( HalocarbonComponent* c ) {
    restartFile << "[" << c->getComponentName() << "]" << endl;
    restartFile << D_HC_CALCDATE << "= " << restartDate << endl;
}
