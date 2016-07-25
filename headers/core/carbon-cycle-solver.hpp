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
#ifndef CARBON_CYCLE_SOLVER_HPP_
#define CARBON_CYCLE_SOLVER_HPP_

/* carbon-cycle-solver.hpp
 *
 * header file for carbon cycle ODE solver.  See notes
 * in carbon-cycle-solver.cpp
 *
 */

#include <string>
#if HAVE_GSL
#include <gsl/gsl_odeiv2.h>
#endif // HAVE_GSL

#include "core/logger.hpp"
#include "models/carbon-cycle-model.hpp"
#include "h_util.hpp"

#define MAX_CARBON_MODEL_RETRIES 8

namespace Hector {
  
/*! \brief The carbon cycle solver component
 *
 * The strategy in this solver is to write the carbon cycle as
 * dC/dt = [A]C + G,
 * where A=A(t) is a matrix of carbon fluxes, assumed to be a
 * slowly-varying function of time, and G=G(t) is a vector of
 * exogenous carbon fluxes, potentially rapidly varying.
 *
 * We further assume that all rapidly varying quantities are
 * "easy" to calculate (i.e., simple functions of C and t),
 * while slowly varying functions are assumed to be potentially
 * "difficult" to calculate.
 *
 * Therefore, we adopt a two-stage integration.  The easy/rapidly
 * varying variables we integrate using a standard integrator from
 * the GSL/odeint.  To integrate from t_i -> t_i+1 we first integrate
 * to t_i+1/2.  We update A(t=t_i+1/2), then we integrate
 * t_i -> t_i+1 using the updated A values.
 *
 */
class CarbonCycleSolver : public IModelComponent {
    
public:
    CarbonCycleSolver();
    virtual ~CarbonCycleSolver();
    
    //! Return a carbon pool value. Components will know which one they want.
    double cpool( int i ) const { return c[ i ]; }
    //! Vector of all carbon pool values
    //const double *allcpool( void ) const { return c; }
    
    
    // IModelComponent methods
    std::string getComponentName() const { return std::string( CCS_COMPONENT_NAME ); }
    
    virtual void init( Core* core );
    
    virtual unitval sendMessage( const std::string& message,
                                const std::string& datum,
                                const message_data info=message_data() ) throw ( h_exception );
    
    virtual void setData( const std::string& varName,
                          const message_data& data ) throw ( h_exception );
    
    virtual void prepareToRun() throw ( h_exception );
    
    virtual void run( const double runToDate ) throw ( h_exception );
    
    virtual bool run_spinup( const int step ) throw ( h_exception );
    
    virtual void shutDown();
    
    
    // IVisitable methods
    virtual void accept( AVisitor* visitor );
    
private:
    virtual unitval getData( const std::string& varName,
                            const double valueIndex ) throw ( h_exception );
    
    //! Number of variables to integrate
    int nc;
    //! Array of carbon pools (and such other vars as need to be integrated)
    std::vector<double> c;
    //! Temporary array for carbon pools (for 2nd order integration)
    //double *cc;
    //! time counter
    double t;
    
    //settable parameters
    //! Absolute error tolerance for integration
    double eps_abs;
    //! Relative error tolerance for integration
    double eps_rel;
    //! Default stepsize (years) -- the integrator will adjust this as required
    double dt;
    
    unitval eps_spinup;     //! spinup epsilon (drift/tolerance), Pg C
    
#if HAVE_GSL
    //! Wrapper function to pass to ODE solver
    static int odeeval( double t, const double y[], double dydt[], void *modelptr );
    //! function def'n struct for gsl ODE solver
    gsl_odeiv2_system odesys;
    //! GSL stepper
    gsl_odeiv2_step *stepper;
    //! GSL step size controller
    gsl_odeiv2_control *controller;
    //! GSL ODE evolver
    gsl_odeiv2_evolve *evolver;
#else
    struct bad_derivative_exception {
        bad_derivative_exception(const int status):errorFlag(status) { }
        int errorFlag;
    };
    // A functor to provide callbacks for the ODE solver. 
    struct ODEEvalFunctor {
        ODEEvalFunctor( CarbonCycleModel* cmodel, double* time ):modelptr(cmodel), t(time) { }
        void operator()( const std::vector<double>& y, std::vector<double>& dydt, double t ) throw( bad_derivative_exception );
        void operator()( const std::vector<double>& y, double t );
        CarbonCycleModel* modelptr;
        double* t;
    };
#endif // HAVE_GSL
    
    void failure( int stat, double t0, double tmid ) throw( h_exception );
    
    bool in_spinup;
    
    // Pointers to other components
    Core *core;
    CarbonCycleModel *cmodel;
    
    //! Logger for solver
    Logger logger;

    //! Internal working space
    std::vector<double> c_original;
    std::vector<double> c_old;
    std::vector<double> c_new;
    std::vector<double> dcdt;
};

}

#endif
