/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef CARBON_CYCLE_MODEL_HPP_
#define CARBON_CYCLE_MODEL_HPP_

/* carbon-cycle-model.hpp
 *
 * header file for carbon cycle model abstract class.
 *
 * Created by Robert, March 2011.
 */

#include <string>

#include "components/imodel_component.hpp"
#include "core/core.hpp"
#include "data/unitval.hpp"
#include "h_util.hpp"

// CO2 ppmv in molar mass dimension
// 1 ppm by volume of atmosphere CO2 = 2.13 Gt C, from http://cdiac.ornl.gov/pns/convert.html
//  and Wigley (1993)
// So to convert 1 Pg C to 1 ppmv CO2...
#define PGC_TO_PPMVCO2   (1/2.13)


// Signal from model to the solver that, while we haven't failed,
// need to stash C values and re-try reaching next timestep
#define CARBON_CYCLE_RETRY 1234

// Signal from model to the solver that all calculations were successful
#define ODE_SUCCESS 0

namespace Hector {

/*! \brief Carbon cycle model class
 *
 *  This is the base class for all carbon cycle models.  The function
 *  of classes in this tree is to implement the right-hand side of the
 *  system of differential equations that describes a carbon cycle.
 *  See also: carbon-cycle-solver.hpp, which solves these models.
 */
class CarbonCycleModel : public IModelComponent {
    
public:
    CarbonCycleModel( int ncpools=0 ) : nc( ncpools ) {}
    virtual ~CarbonCycleModel() {}
    // We will want to put some data accessor methods here, but it's not
    // clear what would be appropriate.
    
    int ncpool() const { return nc; }
    
    // IModelComponent methods
    std::string getComponentName() const { return std::string( CCS_COMPONENT_NAME ); }
    
    virtual void init( Core* core );
    
    virtual unitval sendMessage( const std::string& message,
                                const std::string& datum,
                                const message_data info=message_data() ) throw ( h_exception );
    
    virtual void setData( const std::string& varName,
                          const message_data& data ) throw ( h_exception );
    
    virtual void prepareToRun() throw ( h_exception );
    
    virtual void run( const double runToDate ) throw ( h_exception ) = 0;
    
    //    virtual bool run_spinup( const int step ) throw ( h_exception );
    
    virtual void shutDown();
    
    // The model interface, as seen by the solver
    
    //! Copy the values of the carbon pools into the input array,
    //! stripping off units as necessary.
    virtual void getCValues( double t, double c[] ) = 0;
    
    //! Calculate the derivatives of the carbon pool values and store in
    //! dcdt.
    //! \details The function calcderivs is evaluating is dc/dt =
    //! F(c; t, params).  c and t are passed in; params can be any state
    //! that is stored in the class's member variables.
    virtual int calcderivs( double t, const double c[], double dcdt[] ) const = 0;
    
    //! Calculate updates to the model's "slowly varying" variables.
    
    //! \details The model is allowed to have certain variables that are
    //! assumed to be expensive enough that we want to calculate them
    //! only once per time step and slowly varying enough that we can
    //! get away with doing so (e.g. temperature).  The solver calls
    //! this function when necessary to perform that update.  The
    //! variables should be updated with the c and t values passed in,
    //! as these will generally be different from the last time variable
    //! the model saw from the core.
    virtual void slowparameval( double t, const double c[] ) = 0;
    
    //! Copy the C values back into the model, restore units, etc.
    virtual void stashCValues( double t, const double c[] ) = 0;
    
protected:
    //! Number of carbon pools in the model.
    //! \details nc must be constant over the life of the calculation.
    //! Often it will be known from the moment the model is
    //! instantiated, but we leave open the possibility that it might be
    //! affected by some user settable parameters (e.g. number of deep
    //! ocean boxes).
    int nc;
    
    // Time
    double ODEstartdate;                 //!< Date tracking
    
    //! logger for use by subclasses
    Logger logger;
    
    //! Pointers to the core
    Core *core;
};

}

#endif
