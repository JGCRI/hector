/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  oceanbox.cpp
 *  hector
 *
 *  Created by Corinne Hartin on 1/31/13
 *
 */

#include <boost/math/tools/minima.hpp>
#include <iomanip>

#include "oceanbox.hpp"

namespace Hector {

using namespace std;

//------------------------------------------------------------------------------
/*! \brief a new oceanbox logger
 *
 * The oceanbox logger may or may not be defined and therefore we check before logging
 */
#define OB_LOG(log, level)  \
if( log != NULL ) H_LOG( (*log), level )

//------------------------------------------------------------------------------
/*! \brief Constructor
 */
oceanbox::oceanbox() {
    logger = NULL;
    deltaT.set( 0.0, U_DEGC );
    surfacebox = false;
    Tbox = unitval( -999, U_DEGC );
    atmosphere_flux.set( 0.0, U_PGC );
    preindustrial_flux.set( 0.0, U_PGC_YR );
    ao_flux.set( 0.0, U_PGC );
    oa_flux.set( 0.0, U_PGC );
}

//------------------------------------------------------------------------------
/*! \brief sets the amount of carbon in this box
 */
void oceanbox::set_carbon( const unitval C) {
    carbon.adjust_pool_to_val( C.value( U_PGC ));
	//OB_LOG( logger, Logger::WARNING ) << Name << " box C has been set to " << carbon << endl;
}

//------------------------------------------------------------------------------
/*! \brief initialize all needed information in an oceanbox
 */
void oceanbox::initbox( double boxc, string name ) {
    connection_list.clear();
    connection_k.clear();
    connection_window.clear();
    annual_box_fluxes.clear();

     // Each box is separate from each other, and we keep track of carbon in each box
    Name = name;
    carbon.set( boxc, U_PGC, false, name );
    CarbonAdditions.set( 0.0, U_PGC, false, name );
    CarbonSubtractions.set( 0.0, U_PGC, false, name );
    active_chemistry = false;

    OB_LOG( logger, Logger::NOTICE) << "hello " << name << endl;
}

//------------------------------------------------------------------------------
/*! \brief          Add carbon to an oceanbox
 *  \param[in] carbon    Amount of carbon to add to this box
 *
 *  Carbon flows between boxes arrive via this method. Any carbon (it must be
 *  a positive value) is scheduled for addition; the actual increment happens
 *  in update_state().
 */
void oceanbox::add_carbon( fluxpool carbon ) {
	CarbonAdditions = CarbonAdditions + carbon;
	OB_LOG( logger, Logger::DEBUG) << Name << " receiving " << carbon << " (" << CarbonAdditions << CarbonSubtractions << ")" << endl;
}

//------------------------------------------------------------------------------
/*! \brief          Compute absolute temperature of box in C
 *  \param[in] SST Mean ocean temperature change from preindustrial, C
 *  \returns        Absolute temperature of box, C
 */
unitval oceanbox::compute_tabsC( const unitval SST ) const {
    return SST + unitval( MEAN_GLOBAL_TEMP, U_DEGC ) + deltaT;
}

//------------------------------------------------------------------------------
/*! \brief          Add (or replace) a box-to-box connection
 *  \param[in] ob   pointer to another oceanbox
 *  \param[in] k    weight of the connection
 *  \param[in] ws   connection window size: 0=current state only, 1=mean of current and 1 year back, etc.
 *  \exception      if ob is same as current box
 *
 *  Establishes a one-way connection from the current box to another, with a weight of
 *  k. If a connection already exists, it will be overwritten (i.e. only two connections
 *  between any two boxes are allowed, one in each direction).  Window establishes how many previous
 *  time step connections to use.
 */
void oceanbox::make_connection( oceanbox* ob, const double k, const int ws ) { //, window=1 or whatever we are averaging over.  use curent state of 1 or will use what we tell it to use.

	H_ASSERT( ob != this, "can't make connection to same box" );
	OB_LOG( logger, Logger::NOTICE) << "Adding connection " << Name << " to " << ob->Name << ", k=" << k << endl;

	// If a connection to this box already exists, replace it
	for( unsigned i=0; i<connection_list.size(); i++ ) {
		if( connection_list[ i ]==ob ) {
			connection_k[ i ] = k;
			connection_window[ i ] = ws;
			OB_LOG( logger, Logger::WARNING) << "** overwriting connection in " << Name << " ** " << endl;
			OB_LOG( logger, Logger::WARNING) << "** Are you sure about this? ** " << endl;
			return;
		}
	}

	// Otherwise, make a new connection
	connection_list.push_back( ob ); // add new element to vector
	connection_k.push_back( k );
	H_ASSERT( ws >= 0, "window negative number" );
	connection_window.push_back( ws );
}

//------------------------------------------------------------------------------
double round( const double d ) { return floor( d + 0.5 ); }

//------------------------------------------------------------------------------
/*! \brief Log the current box state
 *
 *  Writes a variety of information (carbon, temperature, DIC, etc.),
 *  as well as connection summaries, to the active log.
 */
void oceanbox::log_state() {
	OB_LOG( logger, Logger::DEBUG) << "----- State of " << Name << " box -----" << endl;
    fluxpool futurec = carbon + CarbonAdditions + ao_flux - oa_flux - CarbonSubtractions;
    OB_LOG( logger, Logger::DEBUG) << "   carbon = " << carbon.value(U_PGC) << " -> " << futurec.value(U_PGC) << endl;
	OB_LOG( logger, Logger::DEBUG) << "   T=" << Tbox << ", surfacebox=" << surfacebox << ", active_chemistry=" << active_chemistry << endl;
    OB_LOG( logger, Logger::DEBUG) << "   CarbonAdditions = " << CarbonAdditions.value(U_PGC) << " ("
        << ( CarbonAdditions / carbon*100 ) << "%)" << endl;
    OB_LOG( logger, Logger::DEBUG) << "   CarbonSubtractions = " << CarbonSubtractions.value(U_PGC) << " ("
        << ( CarbonSubtractions / carbon * 100 ) << "%)" << endl;
    if( surfacebox ) {
        OB_LOG( logger, Logger::DEBUG) << "   FPgC = " << atmosphere_flux.value(U_PGC) << " ("
            << ( atmosphere_flux / carbon * 100 ) << "%)" << endl;
        OB_LOG( logger, Logger::DEBUG) << "   ao_flux = " << ao_flux.value(U_PGC) << " ("
            << ( ao_flux / carbon * 100 ) << "%)" << endl;
        OB_LOG( logger, Logger::DEBUG) << "   oa_flux = " << oa_flux.value(U_PGC) << " ("
            << ( oa_flux / carbon * 100 ) << "%)" << endl;
    }

    unitval K0 = mychemistry.get_K0();
    unitval Tr = mychemistry.get_Tr();
	OB_LOG( logger, Logger::DEBUG) << "   K0 = " << K0 << " " << "Tr = " << Tr << endl;

	if( active_chemistry ) {
        unitval dic = mychemistry.convertToDIC( carbon );
		OB_LOG( logger, Logger::DEBUG) << "   Surface DIC = " << dic << endl;
    }
	for( unsigned i=0; i<connection_list.size(); i++ ) {
		OB_LOG( logger, Logger::DEBUG) << "   Connection #" << i << " to " << connection_list[ i ]->Name << ", k=" << connection_k[ i ] << ", window=" << connection_window[ i ] << endl;
	}
}

//------------------------------------------------------------------------------
/*! \brief Compute all fluxes between boxes
 * \param[in] current_Ca                atmospheric CO2
 * \param[in] yf                year fraction (0-1)
 * \param[in] do_circ           flag: do circulation, or not?
 */
void oceanbox::compute_fluxes( const unitval current_Ca, const fluxpool atmosphere_cpool, const double yf, const bool do_circ ) {

    Ca = current_Ca;

	// Step 1 : run chemistry mode, if applicable
	if( active_chemistry ) {
		OB_LOG( logger, Logger::DEBUG) << Name << " running ocean_csys" << endl;

		mychemistry.ocean_csys_run( Tbox, carbon );

        atmosphere_flux = unitval( mychemistry.calc_annual_surface_flux( Ca ).value( U_PGC_YR ), U_PGC );
	} else  {
		// No active chemistry, so atmosphere-box flux is simply a function of the
		// difference between box carbon and atmospheric carbon s.t. it is preindustrial_flux
		if( surfacebox )
            atmosphere_flux = unitval( preindustrial_flux.value( U_PGC_YR ), U_PGC );
        else
            atmosphere_flux = unitval( 0.0, U_PGC );
	}

    // Step 2 : account for partial year
    atmosphere_flux = atmosphere_flux * yf;

    separate_surface_fluxes( atmosphere_cpool );

    // Step 3: check if this box is oscillating
    /*
    const bool osc = oscillating( 10, // over the last 10 states,
                                   1,   // has box C varied by >1%
                                   3 ); // while changing direction 3+ times?
    */

	// Step 4 : calculate the carbon transports between the boxes
    if( do_circ ) {
        fluxpool closs_total( 0.0, U_PGC, carbon.tracking );

        for( unsigned i=0; i < connection_window.size(); i++ ){

            fluxpool closs = carbon * connection_k[ i ] * yf;

            OB_LOG( logger, Logger::DEBUG) << Name << " conn " << i << " flux= " << closs << endl;

            connection_list[ i ]->add_carbon( closs );
            CarbonSubtractions = CarbonSubtractions + closs;  // PgC
            closs_total = closs_total + closs;
            annual_box_fluxes[ connection_list[ i ] ] = annual_box_fluxes[ connection_list[ i ] ] +
                unitval( closs.value( U_PGC ), U_PGC_YR );
        } // for i

    } // if do_circulation
}

void oceanbox::separate_surface_fluxes( fluxpool atmosphere_pool ) {
    // Set the fluxpool values from the current atmosphere_flux unitval
    if(atmosphere_flux > 0) {
        ao_flux = atmosphere_pool.flux_from_unitval(atmosphere_flux);
        oa_flux = carbon.flux_from_unitval(unitval( 0.0, U_PGC ));
    } else {
        ao_flux = atmosphere_pool.flux_from_unitval(unitval( 0.0, U_PGC ));
        oa_flux = carbon.flux_from_unitval(-atmosphere_flux);
    }
}

//------------------------------------------------------------------------------
/*! \brief    Function to calculate Revelle Factor
*/
// 2 ways of solving for the Revelle factor
// keep track of last year pCO2 in the ocean and DIC
unitval oceanbox::calc_revelle() {
    H_ASSERT( active_chemistry, "Active chemistry required");

//    unitval deltapco2 = Ca - pco2_lastyear;
    unitval deltadic = mychemistry.convertToDIC(carbon) - dic_lastyear;

    H_ASSERT( deltadic.value( U_UMOL_KG) != 0, "DeltaDIC cannot be zero");

    // Revelle Factor can be calculated multiple ways:
    // based on changing atmospheric conditions as well as approximated via DIC and CO3
     return unitval ( mychemistry.convertToDIC( carbon ) / mychemistry.CO3, U_UNITLESS );
    // under high CO2, the HL box numbers are potentially unrealistic.
}

//------------------------------------------------------------------------------
/*! \brief Update to a new carbon state
 */
void oceanbox::update_state() {

	carbon = carbon + CarbonAdditions + ao_flux - oa_flux - CarbonSubtractions;
    // these start with 0 from themselves (this box)
    CarbonAdditions.set( 0.0, U_PGC, carbon.tracking, Name );
    CarbonSubtractions.set( 0.0, U_PGC, carbon.tracking, Name );
}

//------------------------------------------------------------------------------
/*! \brief          A new year is starting. Zero flux variables.
 *  \param[in] SST    Mean ocean surface temperature this year
 */
void oceanbox::new_year( const unitval SST ) {

    for( unsigned i=0; i < connection_window.size(); i++ ){
        annual_box_fluxes[ connection_list[ i ] ] = unitval( 0.0, U_PGC_YR );
    }
    atmosphere_flux.set( 0.0, U_PGC );
    Tbox = compute_tabsC( SST );

    // save for Revelle Calc
    pco2_lastyear = Ca;
    dic_lastyear = mychemistry.convertToDIC( carbon );
}

//------------------------------------------------------------------------------
/*! \brief              Function that chem_equilibrate tries to minimize
 *  \param[in] alk      alkalinity value to try
 *  \param[in] params  pointer to other parameters (just f_target currently)
 *  \returns            double, difference between flux and target flux
 *
 *  The gsl minimization algorithm calls this function, which slots alk
 *  into the csys chemistry input, runs csys, and reports back the difference
 *  between csys's computed ocean-atmosphere flux and the target flux.
 */
double oceanbox::fmin( double alk, void *params ) {

	// Call the chemistry model with new value for alk
	mychemistry.set_alk( alk );
	mychemistry.ocean_csys_run( Tbox, carbon );

	double f_target = *( double * )params;
	double diff = fabs( mychemistry.calc_annual_surface_flux( Ca ).value( U_PGC_YR ) - f_target );
	//    OB_LOG( logger, Logger::DEBUG) << "fmin at " << alk << ", f_target=" << f_target << ", returning " << diff << endl;

	return diff;
}

//------------------------------------------------------------------------------
/*! \brief Functor wrapper for minimization function
 */
struct FMinWrapper {
    FMinWrapper(oceanbox* instance, const double f_targetIn):
        object_which_will_handle_signal(instance),
        f_target(f_targetIn)
    {
    }
    double operator()(const double alk) {
        return object_which_will_handle_signal->fmin(alk, &f_target);
    }

    private:
    oceanbox* object_which_will_handle_signal;
    double f_target;
};

//------------------------------------------------------------------------------
/*! \brief                  Equilibrate the chemistry model to a given flux
 *  \param[in] current_Ca           Atmospheric CO2 (ppmv)
 *
 *  \details The global carbon cycle can be spun up with ocean chemistry either
 *  on or off. In the former case, the ocean continually equilibrates with the
 *  atmosphere during spinup, and the entire system should come to steady state.
 *  In the latter case, the ocean is sealed off from the atmosphere during
 *  spinup, with no surface chemistry; this presumes we know exactly how much
 *  carbon should be in the preindustrial ocean. Before chemistry is re-enabled
 *  for the main run, however, we need to adjust ocean conditions (alkalinity)
 *  such that the chemistry model will produce the specified spinup-condition
 *  fluxes. That's what this method does. Note that we pass in current CO2 because
 *  in case the model was NOT spun up, need to set the box's internal tracking var.
 */
void oceanbox::chem_equilibrate( const unitval current_Ca ) {

	using namespace std;

    Ca = current_Ca;

	H_ASSERT( active_chemistry, "chemistry not turned on" );
	OB_LOG( logger, Logger::DEBUG) << "Equilibrating chemistry for box " << Name << endl;

	// Initialize the box chemistry values: temperature, atmospheric CO2, DIC

    unitval dic = mychemistry.convertToDIC( carbon );
	OB_LOG( logger, Logger::DEBUG) << "Ca=" << Ca << ", DIC=" << dic <<  endl;

	// Tune the chemistry model's alkalinity parameter to produce a particular flux.
	// This happens after the box model has been spun up with chemistry turned off, before the chemistry
	// model is turned on (because we don't want it to suddenly produce a larger ocean-atmosphere flux).

	// Here we use the Brent algorithm
	//      http://www.gnu.org/software/gsl/manual/html_node/Minimization-Algorithms.html
	// to minimize abs(f-f0), where f is computed by the csys chemistry code and f0 passed in

	double alk_min = 2100e-6, alk_max = 2750e-6;
	double f_target = preindustrial_flux.value( U_PGC_YR );

	// Find a best-guess point; the GSL algorithm seems to need it
	OB_LOG( logger, Logger::DEBUG) << "Looking for best-guess alkalinity" << endl;
	const int w = 12;
	OB_LOG( logger, Logger::DEBUG) << setw( w ) << "Alk" << setw( w ) << "FPgC"
        << setw( w ) << "f_target" << setw( w ) << "diff" << endl;
	double min_diff = 1e6;
	double min_point = ( alk_min + alk_max ) / 2.0;
	for( double alk1=alk_min; alk1 <= alk_max; alk1 += (alk_max-alk_min)/20 ) {
		double diff = fmin( alk1, &f_target );
		if( diff < min_diff ) {
			min_diff = diff;
			min_point = alk1;
		}
		OB_LOG( logger, Logger::DEBUG) << setw( w ) << alk1 << setw( w ) << atmosphere_flux
        << setw( w ) << f_target << setw( w ) << diff << endl;
	}
	double alk = min_point;        // this is our best guess
	OB_LOG( logger, Logger::DEBUG) << "Best guess minimum is at " << alk << endl;

    FMinWrapper fFunctor(this, f_target);
    // arbitrarily solve unil 60% of the digits are correct.
    const int digits = numeric_limits<double>::digits;
    int get_digits = static_cast<int>(digits * 0.6);
    std::pair<double, double> r = boost::math::tools::brent_find_minima(fFunctor, alk_min, alk_max, get_digits);
	OB_LOG( logger, Logger::DEBUG) << setw( w ) << "Alk" << setw( w ) << "FPgC"
        << setw( w ) << "f_target" << setw( w ) << "diff" << endl;
    OB_LOG( logger, Logger::DEBUG) << setw( w ) << r.first << setw( w ) << atmosphere_flux
        << setw( w ) << f_target << setw( w ) << r.second << endl;
}

//------------------------------------------------------------------------------
/*! \brief        Start tracking mode for this oceanbox
*/
void oceanbox::start_tracking() {
    carbon.tracking = true;
    CarbonAdditions.tracking = true;
    CarbonSubtractions.tracking = true;
}

}
