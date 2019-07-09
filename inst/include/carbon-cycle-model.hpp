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

#include "imodel_component.hpp"
#include "core.hpp"
#include "unitval.hpp"
#include "h_util.hpp"

// CO2 ppmv in molar mass dimension
// 1 ppm by volume of atmosphere CO2 = 2.13 Gt C, from http://cdiac.ornl.gov/pns/convert.html
//  and Wigley (1993)
// So to convert 1 Pg C to 1 ppmv CO2...
#define PGC_TO_PPMVCO2 (1.0/2.13)
#define PPMVCO2_TO_PGC (1.0/PGC_TO_PPMVCO2)

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
    
    //! Record the final state at the end of a time step
    
    //! \details This method should copy all state variables into a
    //! time-indexed array.  Mostly this will allow the object to
    //! reset to a previous time, but the object may also use these
    //! arrays to provide other components with data from the entire
    //! history of the run.  If a carbon cycle class stores
    //! time-indexed values in the course of its normal operation,
    //! then there might not be any need to do this copying.  In that
    //! case, the class can inherit the default implementation of the
    //! method, which does nothing.
    virtual void record_state(double t) {}

    // Create, delete, and rename biomes. These must be defined here
    // because some C cycle models (e.g. the ocean C cycle component)
    // will not have biomes, but are members of the `CarbonCycleModel`
    // class. For more details and reference implementation, see the
    // `SimpleNBox` model.
    inline
    virtual void createBiome(const std::string& biome) {
        H_THROW("`createBiome` is not defined for this component.")
    }
    inline
    virtual void deleteBiome(const std::string& biome) {
        H_THROW("`deleteBiome` is not defined for this component.")
    };
    inline
    virtual void renameBiome(const std::string& oldname, const std::string& newname){
        H_THROW("`renameBiome` is not defined for this component.")
    }

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
