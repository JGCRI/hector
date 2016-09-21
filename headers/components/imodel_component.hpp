/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef IMODEL_COMPONENT_H
#define IMODEL_COMPONENT_H
/*
 *  imodel_component.h
 *  hector
 *
 *  Created by Pralit Patel on 10/29/10.
 *
 */

#include "components/component_names.hpp"
#include "components/component_data.hpp"
#include "core/ivisitable.hpp"
#include "data/unitval.hpp"
#include "data/message_data.hpp"
#include "h_exception.hpp"

namespace Hector {
  
class Core;
class DependencyFinder;

//------------------------------------------------------------------------------
/*! \brief IModelComponent interface
 *
 *  An interface which defines what it means to be part of Hector!
 */
class IModelComponent : public IVisitable {
public:
    inline virtual ~IModelComponent();
    
    //------------------------------------------------------------------------------
    /*! \brief A name which suitably describes this component.
     *
     * \return A string name for the model component.
     */
    virtual std::string getComponentName() const = 0;
    
    //------------------------------------------------------------------------------
    /*! \brief Perform and initializations necessary before setting data.
     *
     *  This would most frequently be used to initialize data structures.
     */
    virtual void init( Core* core ) = 0;
    
    //------------------------------------------------------------------------------
    /*! \brief Handles a message to the component.
     *
     *  Components can send messages to each other, typically requesting
     *  internal data. (E.g., forcing needs to know atmospheric CO2.)
     *
     *  \param message  The message (required).
     *  \param datum    The relevant piece of data (optional).
     *  \param info     A message_data struct (optional), giving more information.
     *  \exception h_exception If message or datum not recognized.
     */
    virtual unitval sendMessage( const std::string& message,
                                const std::string& datum,
                                const message_data info=message_data() ) throw ( h_exception ) = 0;
    
    //------------------------------------------------------------------------------
    /*! \brief Sets the variable specified by varName with the given data.
     *
     *  The model component will be responsible for parsing data.  Chiefly the 
     *  value_str field should be converted to the appropriate data type.  The date
     *  field may be set if varName is part of a time series otherwise the model
     *  component will expect the date field to be equal to Core::undefinedIndex().
     *  In the case of unitvals the units_str field may also be set.  Note that
     *  even if units_str is not set it may be the case that units are provided
     *  unparsed in value_str.  The utility unitval::parse_untival(string, string, unit_types)
     *  should be used to ensure proper parsing of units in all cases.
     *
     *  \param varName The name of the variable to set.
     *  \param data The message_data who's fields will be appropriately set to
     *              contain all relevant information to parse the variable.
     *  \exception h_exception If varName was not recognized or could not convert
     *                         value to the appropriate type.
     */
    virtual void setData( const std::string& varName,
                          const message_data& data ) throw ( h_exception ) = 0;
    
    //------------------------------------------------------------------------------
    /*! \brief A notification that all data are set and the component should prepare to run.
     *
     *  Perform error checking on input data and one-time initializations before
     *  the model starts running. After this, the component should be prepared to
     *  provide data (e.g. during spinup, even if it's not itself spinning up).
     *
     *  \param core A pointer to the core should this model component need to get
     *              additional information from it.
     *  \exception h_exception If there was any inconsistencies with input data.
     */
    virtual void prepareToRun() throw ( h_exception ) = 0;
    
    //------------------------------------------------------------------------------
    /*! \brief Run the component up to the given date.
     *
     *  Run the component from the date of which it was last run (which is presumably
     *  kept track of, if necessary).
     *
     *  \param runToDate The date to run up to.
     *  \pre runToDate is greater than the last date at which the model component
     *       was run and is within the given start and end dates given during
     *       initialization.
     *  \exception h_exception If an error occurred for any reason while running.
     */
    virtual void run( const double runToDate ) throw ( h_exception ) = 0;
    
    //------------------------------------------------------------------------------
    /*! \brief Run the component in spinup mode.
     *
     *  Run the component for a spinup step, signalling whether component is
     *  successfully spun up. Most components don't need this capability, and simply
     *  inherit the implementation below.
     *
     *  \param      step The spinup step to run to.
     *  \return     A bool indicating whether the component is spun up.
     *  \exception  h_exception If an error occurred for any reason while running.
     */
    virtual bool run_spinup( const int step ) throw ( h_exception ) { return true; };
    
    //------------------------------------------------------------------------------
    /*! \brief We will no longer attempt to run the model; perform any cleanup.
     *
     *  Note that reporting will be handled by a visitor.
     */
    virtual void shutDown() = 0;
    
private:
    //------------------------------------------------------------------------------
    /*! \brief Gets the variable specified with by varName with the given value.
     *
     *  The varname string will be exactly the same as it is called in code.  A
     *  model component must add all parsable variable *what*.  What to say about
     *  arrayed inputs?
     *
     *  \param varName The name of the variable to set.
     *  \param value The string value of the variable to set.  Conversion to the
     *               appropriate type will be made by the IModelComponent subclass
     *               on a per variable basis.
     *  \param valueIndex The time series index value to use when setting value.
     *                    If varName is not a time series valueIndex should equal
     *                    Core::undefinedIndex().
     *  \exception h_exception If varName was not recognized or could not convert
     *                         value to the appropriate type.
     */
    virtual unitval getData( const std::string& varName,
                            const double date ) throw ( h_exception ) = 0;
};

// Inline methods
IModelComponent::~IModelComponent() {
}

}

#endif // IMODEL_COMPONENT_H
