/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
 /*
 * carbon-cycle-solver.cpp
 *
 * ODE solver for integrating the carbon cycle in hector.
 *
 * See notes in carbon-cycle-solver.cpp
 *
 */

#include <math.h>
#include <string>
#if HAVE_GSL
#include <gsl/gsl_odeiv2.h>
#include <gsl/gsl_errno.h>
#else
#include <boost/numeric/odeint.hpp>
#endif // HAVE_GSL
#include "boost/lexical_cast.hpp"

#include "core/carbon-cycle-solver.hpp"
#include "visitors/avisitor.hpp"

namespace Hector {
  
//------------------------------------------------------------------------------
/*! \brief Constructor
 */
CarbonCycleSolver::CarbonCycleSolver() : nc( 0 ),
eps_abs( 1.0e-6 ),eps_rel( 1.0e-6 ),
dt( 0.3 )
{
#if HAVE_GSL
    // set up GSL ode solver
    odesys.function = odeeval;
    odesys.jacobian = NULL; // use algorithms that don't require
    // Jacobian, since we can't easily calculate it.
    odesys.dimension = 0;   // set once we have a pointer to the model
    odesys.params = NULL;   // same
    
    eps_spinup.set( 0.001, U_PGC );     // default: 1 Tg C
    
    // can't set this up until we know how many variables we will be
    // integrating.
    stepper = NULL;
    // controller needs error tolerance as input
    // TODO: carbon models should return errors, esp. the ocean
    controller = NULL;
    // evolver also needs to know how many variables to integrate
    evolver = NULL;
#endif // HAVE_GSL
}

//------------------------------------------------------------------------------
/*! \brief Deconstructor
 */
CarbonCycleSolver::~CarbonCycleSolver()
{
#if HAVE_GSL
    if( stepper )
        gsl_odeiv2_step_free( stepper );
    if( controller )
        gsl_odeiv2_control_free( controller );
    if( evolver )
        gsl_odeiv2_evolve_free( evolver );
#endif // HAVE_GSL
}

//------------------------------------------------------------------------------
// documentation is inherited
void CarbonCycleSolver::init( Core* coreptr ) {
    logger.open( getComponentName(), false, Logger::NOTICE );
    H_LOG( logger, Logger::DEBUG ) << getComponentName() << " initialized." << std::endl;
    
    core = coreptr;
    
    in_spinup = false;
    
    // We want to run after the carbon box models, to give them a chance to initialize
    core->registerDependency( D_ATMOSPHERIC_C, getComponentName() );
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval CarbonCycleSolver::sendMessage( const std::string& message,
                                       const std::string& datum,
                                       const message_data info ) throw ( h_exception )
{
    unitval returnval;
    
    if( message==M_GETDATA ) {          //! Caller is requesting data
        return getData( datum, info.date );
        
    } else if( message==M_SETDATA ) {   //! Caller is requesting to set data
        //TODO: call setData below
        //TODO: change core so that parsing is routed through sendMessage
        //TODO: make setData private
        
    } else {                        //! We don't handle any other messages
        H_THROW( "Caller sent unknown message: "+message );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void CarbonCycleSolver::setData( const std::string &varName,
                                 const message_data& data ) throw ( h_exception )
{
    using boost::lexical_cast;
    
    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << data.date << "]=" << data.value_str << std::endl;
    
    try {
        if( varName == D_CCS_EPS_ABS ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            eps_abs = lexical_cast<double>( data.value_str );
        }
        else if( varName == D_CCS_EPS_REL ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            eps_rel = lexical_cast<double>( data.value_str );
        }
        else if( varName == D_CCS_DT ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            dt = lexical_cast<double>( data.value_str );
        }
        else if( varName == D_EPS_SPINUP ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            eps_spinup = unitval::parse_unitval( data.value_str, data.units_str, U_PGC );
        }
        else {
            H_LOG( logger, Logger::SEVERE ) << "Unknown variable " << varName << std::endl;
            H_THROW( "Unknown variable name while parsing "+ getComponentName() + ": "
                    + varName );
        }
    } catch( boost::bad_lexical_cast& castException ) {
        H_LOG( logger, Logger::SEVERE ) << "Could not convert " << varName << std::endl;
        H_THROW( "Could not convert var: "+varName+", value: " + data.value_str + ", exception: "
                +castException.what() );
    } catch( h_exception& parseException ) {
        H_RETHROW( parseException, "Could not parse var: "+varName );
    }
    
    return;
}

//------------------------------------------------------------------------------
// documentation is inherited
void CarbonCycleSolver::prepareToRun() throw( h_exception )
{
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
    
    cmodel = static_cast<CarbonCycleModel*>( core->getComponentByCapability( D_ATMOSPHERIC_C ) );
    
    // initialize the solver's internal data
    t = core->getStartDate();
    nc = cmodel->ncpool();
    H_LOG( logger, Logger::DEBUG ) << "Carbon model in use is " << cmodel->getComponentName() << std::endl;
    H_LOG( logger, Logger::DEBUG ) << "Carbon model pools: " << nc << std::endl;
    H_ASSERT( nc > 0, "nc must be > 0" );
    
    c      = std::vector<double>( nc );
    
#if HAVE_GSL
    // set up the integrator
    odesys.dimension = nc;
    odesys.params    = cmodel;
    stepper          = gsl_odeiv2_step_alloc( gsl_odeiv2_step_rkf45, nc );
    controller       = gsl_odeiv2_control_y_new( eps_abs, eps_rel );
    evolver          = gsl_odeiv2_evolve_alloc( nc );
#endif // HAVE_GSL
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval CarbonCycleSolver::getData( const std::string& varName,
                                   const double date ) throw ( h_exception ) {
    
    unitval returnval;
    
    H_ASSERT( date == Core::undefinedIndex(), "Date not allowed for CarbonCycleSolver" );
    
    H_THROW( "Caller is requesting unknown variable: " + varName );
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void CarbonCycleSolver::shutDown()
{
	H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}


#if HAVE_GSL
//------------------------------------------------------------------------------
/*! \brief              Dispatch function called by ODE solver
 *  \param[in] t        time
 *  \param[in] y        pools
 *  \param[in] dydt     pool changes
 *  \param[in] modelptr pointer to the carbon model
 *  \returns            flag indicating success or failure
 */
int CarbonCycleSolver::odeeval( double t, const double y[], double dydt[], void *modelptr )
{
    const CarbonCycleModel *mod = static_cast<const CarbonCycleModel*>( modelptr );
    
    return mod->calcderivs( t, y, dydt );
}
#else
//------------------------------------------------------------------------------
/*! \brief              Dispatch function called by ODE solver
 *  \param[in] y        pools
 *  \param[in] dydt     pool changes
 *  \param[in] t        time
 *  \exception          If the carbon model returned failure flag we must throw
 *                      an exception to stop the ODE solver.
 */
void CarbonCycleSolver::ODEEvalFunctor::operator()( const std::vector<double>& y,
                                                    std::vector<double>& dydt,
                                                    double t ) throw ( bad_derivative_exception )
{
    // Note the std garuntees vetors are contigous so we can convert to array by
    // taking the address of the first value.
    int status = modelptr->calcderivs( t, &y[0], &dydt[0] );

    if( status != ODE_SUCCESS ) {
        bad_derivative_exception e(status);
        throw e;
    }
}

//------------------------------------------------------------------------------
/*! \brief              Observer callback called by ODE solver when a successful
 *                      step has been taken.
 *  \details            We use this callback to update our state variable time (t)
 *                      and in principle carbon pools (c) however the later is
 *                      already updated by the solver (pass by reference) so we will
 *                      skip copying here.
 *  \param[in] y        pools
 *  \param[in] t        time
 */
void CarbonCycleSolver::ODEEvalFunctor::operator()( const std::vector<double>& y, double t ) {
    // copy the current time to the original in CarbonCycleSolver
    (*this->t) = t;
}
#endif // HAVE_GSL

//------------------------------------------------------------------------------
/*! \brief Support function for gsl_ode failure
 *  \param[in] stat     failure code
 *  \param[in] t0       start of time step
 *  \param[in] tmid     middle of time step
 *  \exception          will always happen; gsl solver has failed
 */
void CarbonCycleSolver::failure( int stat, double t0, double tmid ) throw( h_exception ) {
    H_LOG( logger, Logger::SEVERE ) << "gsl_ode_evolve_apply failed at t= " <<
    t0 << "  tinit= " << t << "  tmid = " << tmid << "  last dt= " <<
    dt << "\nError code: " << stat << "\ncvals:\n";
    for( int i=0; i<nc; ++i )
        H_LOG( logger,Logger::SEVERE ) << c[ i ] << "  ";
    H_LOG( logger,Logger::SEVERE ) << std::endl;
    H_THROW( "gsl_ode_evolve_apply failed." );
}

//------------------------------------------------------------------------------
// documentation is inherited
void CarbonCycleSolver::run( const double tnew ) throw ( h_exception )
{
    H_ASSERT( tnew > t, "solver tnew is not greater than t" );
    
    // Get the initial state data from the box model. c will be filled in
    // Note that we rely on the box model to handle the units.  Inside the
    // solver we strip the unit values and work with raw numbers.
    cmodel->getCValues( t, &c[0] );

/*
    // Copy c to use in the midpoint evaluation
    for( int i=0; i<nc; ++i )
        cc[ i ] = c[ i ];

    // Integrate cc to midpoint
    double tmid = 0.5*( t+tnew );
 */
    double t0   = t;
/*
    // Have the model evaluate its slowly varying parameters (e.g. temperature).
    // It will store them internally for use by calcderivs.
    cmodel->slowparameval( tmid, cc );
    
    while ( t0<tmid ) {
        int stat = gsl_odeiv_evolve_apply( evolver, controller, stepper, &odesys,
                                          &t0, tmid, &dt, cc );
        if( stat != GSL_SUCCESS )
            failure( stat, t0, tmid );
    }
    H_ASSERT( t0 == tmid, "solver t0 != tmid" );
*/    
    // Now integrate from the beginning of the time step using the updated
    // slow params.  Note we can discard t0 and the values in cc
    t0 = t;  // stash this in case we need to report & diagnose an error
    cmodel->slowparameval( t, &c[0] );
//    cmodel->slowparameval( t, cc );
    int retry = 0;
    
    H_LOG( logger, Logger::DEBUG ) << "Entering ODE solver " << t << "->" << tnew << std::endl;
    while( t < tnew && retry < MAX_CARBON_MODEL_RETRIES ) {

        H_LOG( logger, Logger::DEBUG ) << "Resetting evolver and stepper" << std::endl;
#if HAVE_GSL
        gsl_odeiv2_evolve_reset( evolver );
        gsl_odeiv2_step_reset( stepper );
#endif // HAVE_GSL
        double t_start = t;
        double t_target = tnew;
        
        while( t < t_target && retry < MAX_CARBON_MODEL_RETRIES ) {
            H_LOG( logger, Logger::DEBUG ) << "Attempting ODE solver " << t << "->" << t_target << " (" << t0 << "->" << tnew << ")" << std::endl;
            std::cout << "Attempting ODE solver " << t << "->" << t_target << " (" << t0 << "->" << tnew << ")" << std::endl;
            
#if HAVE_GSL
            int stat = gsl_odeiv2_evolve_apply( evolver, controller, stepper, &odesys,
                                               &t, t_target, &dt, &c[0] );
#else
            int stat = ODE_SUCCESS;
            ODEEvalFunctor odeFunctor( cmodel, &t );
            try {
                using namespace boost::numeric::odeint;
                typedef runge_kutta_dopri5<std::vector<double> > error_stepper_type;
                integrate_adaptive( make_controlled<error_stepper_type>( eps_abs, eps_rel ),
                         odeFunctor, c, t_start, t_target, dt, odeFunctor );
            } catch( bad_derivative_exception& e ) {
                stat = e.errorFlag;
            }
#endif // HAVE_GSL
            
            if( stat == CARBON_CYCLE_RETRY ) {
                H_LOG( logger, Logger::NOTICE ) << "Carbon model requests retry #" << ++retry << " at t= " << t << std::endl;
                t_target = t_start + ( t_target - t_start ) / 2.0;
                t = t_start;
                
#if HAVE_GSL
                gsl_odeiv2_evolve_reset( evolver ); // reset everything: evolver, stepper, step size
                gsl_odeiv2_step_reset( stepper );
#endif // HAVE_GSL
                dt = t_target - t;
                cmodel->getCValues( t, &c[0] );     // reset pools and inform model of new starting point
                H_LOG( logger, Logger::DEBUG ) << "New target is " << t_target << std::endl;
                std::cout << "New target is " << t_target << std::endl;
            } else if( stat != ODE_SUCCESS )
                failure( stat, t_start, t_target );
        }
        
        // We have exited GSL solver loop, but did we make it?
        if( retry < MAX_CARBON_MODEL_RETRIES ) {
            H_LOG( logger, Logger::DEBUG ) << "Success: we have reached " << t_target << std::endl;
            if( retry )
                std::cout << "Success after failure: we have reached " << t_target << std::endl;

            retry = 0;
            cmodel->stashCValues( t, &c[0] );   // update state
        } else {
            std::cout << "Failure after failure: t is " << t << "; we have not reached " << t_target << std::endl;
            
        }
    }
    H_ASSERT( t == tnew, "solver failure: t != tnew" );
    
    // If we get here, we're done
//    cmodel->stashCValues( t, c );
        
    H_LOG( logger, Logger::NOTICE ) << "ODE solver success at t= " << t <<
    "  last dt= " << dt << std::endl;
    H_LOG( logger, Logger::DEBUG ) << "cvals\terrors\n";
#if HAVE_GSL
    for( int i=0; i<nc; ++i)
        H_LOG( logger, Logger::DEBUG ) << c[i] << "\t" << evolver->yerr[i] << std::endl;
#endif // HAVE_GSL
    
    H_LOG( logger, Logger::NOTICE ) << std::endl;
}

//------------------------------------------------------------------------------
/*! \brief      Run the carbon cycle to steady state, if possible
 *  \exception  epsilon must be >0
 *
 *  We normally want the carbon model to be in steady state, i.e. with unchanging
 *  carbon pools, at the end of the preindustrial period before the main run.
 *  To reach this point, run the carbon model until all dc/dt are ~0.
 */
bool CarbonCycleSolver::run_spinup( const int step ) throw( h_exception )
{
    
    if( !in_spinup ) {  // first time
        in_spinup = true;
        t = step-1;
        c_original.resize(nc);
        c_old.resize(nc);
        c_new.resize(nc);
        dcdt.resize(nc);
        // initialize to zero
        for(int i=0; i<nc; ++i)
          c_original[i] = c_old[i] = c_new[i] = dcdt[i] = 0.0;
        
        cmodel->getCValues( t, &c_original[0] );
    }
    
    cmodel->getCValues( t, &c_old[0] );
    run( step );
    cmodel->getCValues( step, &c_new[0] );
    
    int max_dcdt_pool = 0;
    double max_dcdt = 0.0;
    
    for( int i=0; i<nc; i++ ) {     // find the biggest difference
        dcdt[ i ] = fabs( c_new[ i ] - c_old[ i ] );
        c_old[ i ] = c_new[ i ];
        if (dcdt[ i ] > max_dcdt) {
            max_dcdt = dcdt[i];
        }
    }

    bool spunup = ( max_dcdt < eps_spinup.value( U_PGC ) );
    
    if( spunup ) {
        Logger& glog = Logger::getGlobalLogger();
        H_LOG( glog, Logger::NOTICE ) << "Carbon model is spun up after " << step << " steps" << std::endl;
        H_LOG( logger, Logger::NOTICE ) << "Carbon model spun up after " << step << " steps. Max residual dc/dt="
        << max_dcdt << " (pool " << max_dcdt_pool << ")" << std::endl;
        for( int i=0; i<nc; i++ ) {
            H_LOG( logger, Logger::NOTICE ) << "New pool " << i << ":" << c_new[ i ]
            << " (delta=" << c_new[ i ]-c_original[ i ] << ")" << std::endl;
        }
        t = core->getStartDate();
        H_LOG( logger, Logger::NOTICE ) << "Resetting solver time counter to t= " << t << std::endl;
    }
    
    return spunup;
}

//------------------------------------------------------------------------------
/*! \brief visitor accept code
 */
void CarbonCycleSolver::accept( AVisitor* visitor )
{
    visitor->visit( this );
}

}
