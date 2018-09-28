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
    initbox( unitval( 0.0, U_PGC ), "?" );
    surfacebox = false;
    preindustrial_flux.set( 0.0, U_PGC_YR );
    warmingfactor = 1.0;      // by default warms exactly as global
    Tbox = unitval( -999, U_DEGC );
    atmosphere_flux.set( 0.0, U_PGC );
}

//------------------------------------------------------------------------------
/*! \brief sets the amount of carbon in this box
 */
void oceanbox::set_carbon( const unitval C) {
	carbon = C;
	OB_LOG( logger, Logger::WARNING ) << Name << " box C has been set to " << carbon << endl;
	carbonHistory.insert( carbonHistory.begin(), C.value( U_PGC ) );
}

//------------------------------------------------------------------------------
/*! \brief initialize basic information in an oceanbox
 */
void oceanbox::initbox( unitval C, string N ) {
    // Reset the box to its pristine state
    connection_list.clear();
    connection_k.clear();
    carbonHistory.clear(); 
    carbonLossHistory.clear();
    connection_window.clear();
    annual_box_fluxes.clear();
    
    set_carbon( C );
    if( N != "" ) Name = N;
    CarbonToAdd.set( 0.0, U_PGC );  // each box is separate from each other, and we keep track of carbon in each box.
    active_chemistry = false;
    
    OB_LOG( logger, Logger::NOTICE) << "hello " << N << endl;
}

//------------------------------------------------------------------------------
/*! \brief          Add carbon to an oceanbox
 *  \param[in] C    amount of carbon to add to this box
 *
 *  Carbon flows between boxes arrive via this method. Any carbon (it must be
 *  a positive value) is scheduled for addition; the actual increment happens
 *  in update_state().
 */
void oceanbox::add_carbon( unitval C ) {
	H_ASSERT( C.value( U_PGC ) >= 0.0, "add_carbon called with negative value" );
	CarbonToAdd = CarbonToAdd + C;
	OB_LOG( logger, Logger::DEBUG) << Name << " receiving " << C << " (" << CarbonToAdd << ")" << endl;
}

//------------------------------------------------------------------------------
/*! \brief          Compute absolute temperature of box in C
 *  \param[in] Tgav Mean global temperature change from preindustrial, C
 *  \returns        Absolute temperature of box, C
 */
unitval oceanbox::compute_tabsC( const unitval Tgav ) const {
    return Tgav * warmingfactor + unitval( MEAN_GLOBAL_TEMP, U_DEGC ) + deltaT;
}

//------------------------------------------------------------------------------
/*! \brief          Is box C oscillating?
 *  \param[in] lookback     Window size to look back into the past
 *  \param[in] maxamp       Max amplitude (% of box size) allowed
 *  \param[in] maxflips     Max flips (direction reversals) allowed
 *  \returns                bool indicating whether box C is oscillating recently
 *  \details                How do we identify is the box is oscillating? Count how many
 *  sign flips there are in the deltas (C(t)-C(t-1)) of a certain magnitude.
 */
bool oceanbox::oscillating( const unsigned lookback, const double maxamp, const int maxflips ) const {
    
#define sgn( x ) ( x > 0 ) - ( x < 0 )
    
    if( carbonHistory.size() < lookback ) return false;
    
    const double currentC = carbon.value( U_PGC );
    double minC = currentC, maxC = currentC;
    double lastdelta = currentC - carbonHistory[ 0 ];
    int flipcount = 0;
    for ( unsigned i=0; i<lookback-1; i++ )  {
        double delta = carbonHistory[ i ]-carbonHistory[ i+1 ];
        flipcount += ( sgn( lastdelta ) != sgn( delta ) );
        lastdelta = delta;
        minC = min( minC, carbonHistory[ i ] );
        maxC = max( maxC, carbonHistory[ i ] );
    }
    return flipcount>maxflips && ( maxC-minC )/currentC*100 > maxamp;
}

//------------------------------------------------------------------------------
/*! \brief                  Calculate mean past box carbon
 *  \param[in] lookback     Window size to look back into the past (including current value)
 *  \returns                bool indicating whether box C is oscillating recently
 *  \exception              lookback must be non-negative
 */
double oceanbox::vectorHistoryMean( std::vector<double> v, int lookback ) const {
    H_ASSERT( lookback > 0, "lookback must be >0" );
    H_ASSERT( v.size() > 0, "vector size must be >0" );

    lookback = min<int>( int( v.size() ), lookback );

    double sum = 0.0;
    for ( int j=0; j<lookback; j++ )  {
        sum += v[ j ]; // sum up the past states
    }
    return sum / lookback;
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
    unitval futurec = carbon + CarbonToAdd + atmosphere_flux;
	OB_LOG( logger, Logger::DEBUG) << "   carbon = " << carbon << " -> " << futurec << endl;
	OB_LOG( logger, Logger::DEBUG) << "   T=" << Tbox << ", surfacebox=" << surfacebox << ", active_chemistry=" << active_chemistry << endl;
	OB_LOG( logger, Logger::DEBUG) << "   CarbonToAdd = " << CarbonToAdd << " ("
        << ( CarbonToAdd/carbon*100 ) << "%)" << endl;
    if( surfacebox ) {
        OB_LOG( logger, Logger::DEBUG) << "   FPgC = " << atmosphere_flux << " ("
            << ( atmosphere_flux/carbon*100 ) << "%)" << endl;
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
/*! \brief Compute one flux from this to another box
 * \param[in] i     connection number
 * \param[in] yf    year fraction (0-1)
 * \returns         flux in Pg C, accounting for yf, connection strength
 */
unitval oceanbox::compute_connection_flux( int i, double yf ) const {
    // Compute the mean_carbon over connection-specific history window
    double mean_carbon = carbon.value( U_PGC );
    if( connection_window[ i ] )
        mean_carbon = vectorHistoryMean( carbonHistory, connection_window[ i ] );
    
    unitval closs( mean_carbon * connection_k[ i ] * yf, U_PGC );
    return closs;
}

//------------------------------------------------------------------------------
/*! \brief Compute all fluxes between boxes
 * \param[in] Ca                atmospheric CO2
 * \param[in] yf                year fraction (0-1)
 * \param[in] do_circ           flag: do circulation, or not?
 */
void oceanbox::compute_fluxes( const unitval current_Ca, const double yf, const bool do_circ ) {
    
    Ca = current_Ca;
    
	// Step 1 : run chemistry mode, if applicable
	if( active_chemistry ) {
		OB_LOG( logger, Logger::DEBUG) << Name << " running ocean_csys" << endl;
        H_ASSERT( Tbox.value( U_DEGC ) > -999, "bad tbox value" );        // TODO: this isn't a good temperature check
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

    // Step 3: check if this box is oscillating
    /*
    const bool osc = oscillating( 10, // over the last 10 states,
                                   1,   // has box C varied by >1%
                                   3 ); // while changing direction 3+ times?
    */
    
	// Step 4 : calculate the carbon transports between the boxes
    if( do_circ ) {
        double unstable_box_flux_adjust = 1.0;
        
        // 'Preflight' the outbound fluxes, i.e. get estimate of their total
        unitval closs_total( 0.0, U_PGC );
        for( unsigned i=0; i < connection_window.size(); i++ ){
            closs_total = closs_total + compute_connection_flux( i, yf );
        } // for i

        if( 0 /* osc */ ) {
            const double mean_past_loss = vectorHistoryMean( carbonLossHistory, 10 );
            unstable_box_flux_adjust = mean_past_loss / closs_total.value( U_PGC );
            
            OB_LOG( logger, Logger::DEBUG) << Name << "is oscillating." << std::endl;
            std::cout << "Preflighted connection fluxes = " << closs_total << std::endl;
            std::cout << "Recent mean loss = " << mean_past_loss << std::endl;
            std::cout << "unstable_box_flux_adjust = " << unstable_box_flux_adjust << std::endl;
            // The box's C is oscillating
        }
                
        closs_total.set( 0.0, U_PGC );
        for( unsigned i=0; i < connection_window.size(); i++ ){

            unitval closs = compute_connection_flux( i, yf ) * unstable_box_flux_adjust;

            OB_LOG( logger, Logger::DEBUG) << Name << " conn " << i << " flux= " << closs << endl;
                 
            connection_list[ i ]->add_carbon( closs );
            CarbonToAdd = CarbonToAdd - closs;  // PgC
            closs_total = closs_total + closs;
           annual_box_fluxes[ connection_list[ i ] ] = annual_box_fluxes[ connection_list[ i ] ] +
                unitval( closs.value( U_PGC ), U_PGC_YR );
        } // for i
        
        carbonLossHistory.insert( carbonLossHistory.begin (), closs_total.value( U_PGC ) );
        
    } // if do_circulation
}

//------------------------------------------------------------------------------
/*! \brief    Function to calculate Revelle Factor
*/

// 2 ways of solving for the Revelle factor
// keep track of last year pCO2 in the ocean and DIC
unitval oceanbox::calc_revelle() {
    H_ASSERT( active_chemistry, "Active Chemistry required");
        
//    unitval deltapco2 = Ca - pco2_lastyear;
    unitval deltadic = mychemistry.convertToDIC(carbon) - dic_lastyear;
    
    H_ASSERT( deltadic.value( U_UMOL_KG) != 0, "DeltaDIC can not be zero");
    
    // Revelle Factor can be calculated multiple ways.  
    // Based on changing atmospheric conditions as well approximated via DIC and CO3
     return unitval ( mychemistry.convertToDIC( carbon ) / mychemistry.CO3, U_UNITLESS ); 
    // under high CO2, the HL box numbers are potentially unrealistic. 
}

//------------------------------------------------------------------------------
/*! \brief Update to a new carbon state
 */
void oceanbox::update_state() {
    
	carbonHistory.insert( carbonHistory.begin (), carbon.value( U_PGC ) );
	
	carbon = carbon + CarbonToAdd + atmosphere_flux;
    
	H_ASSERT( carbon.value( U_PGC ) >= 0.0, "box carbon is negative" );
	CarbonToAdd.set( 0.0, U_PGC );
}

//------------------------------------------------------------------------------
/*! \brief          A new year is starting. Zero flux variables.
 *  \param[in] t    Mean global temperature this year
 */
void oceanbox::new_year( const unitval Tgav ) {
    
    for( unsigned i=0; i < connection_window.size(); i++ ){
        annual_box_fluxes[ connection_list[ i ] ] = unitval( 0.0, U_PGC_YR );
    }
    atmosphere_flux.set( 0.0, U_PGC );
    Tbox = compute_tabsC( Tgav );

    // save for Revelle Calc
    pco2_lastyear = Ca;
    dic_lastyear = mychemistry.convertToDIC( carbon );
}

//------------------------------------------------------------------------------
/*! \brief              Function that chem_equilibrate tries to minimize
 *  \param[in] alk      alkalinity value to try
 *  \param[in] *params  pointer to other parameters (just f_target currently)
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
 *  \param[in] Ca           Atmospheric CO2 (ppmv)
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
//	mychemistry.ocean_csys_run( Tbox, carbon );
    
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

}
