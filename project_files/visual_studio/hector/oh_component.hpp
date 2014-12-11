
#ifndef OH_COMPONENT_H
#define OH_COMPONENT_H
/*
 *  oh_component.hpp
 *  hector
 *
 *  Created by Corinne on 11/20/2014.
 *
 */

#include "components/imodel_component.hpp"
#include "core/logger.hpp"
#include "data/tseries.hpp"
#include "data/unitval.hpp"

namespace Hector {
  
//------------------------------------------------------------------------------
/*! \brief Methane model component.
 *
 *  This doesn't do much yet.
 */
class OHComponent : public IModelComponent {
    friend class INIRestartVisitor; // To allow creation of a restart file.
    
public:
    OHComponent();
    ~OHComponent();
    
    // IModelComponent methods
    virtual std::string getComponentName() const;
    
    virtual void init( Core* core );
    
    virtual unitval sendMessage( const std::string& message,
                                const std::string& datum,
                                const message_data info=message_data() ) throw ( h_exception );
    
    virtual void setData( const std::string& varName,
                          const message_data& data ) throw ( h_exception );
    
    virtual void prepareToRun() throw ( h_exception );
    
    virtual void run( const double runToDate ) throw ( h_exception );
    
    virtual void shutDown();
    
    // IVisitable methods
    virtual void accept( AVisitor* visitor );
    
	unitval TOH0;            // preindustrial OH lifetime
    double const CCO;       // coefficent for CO
    double const CNMVOC;    // coefficent for NMVOC
    double const CNOX;      // coefficent for NOX
    double const CCH4;      // coefficent for CH4

   private:
    virtual unitval getData( const std::string& varName,
                            const double date ) throw ( h_exception );   
     //! emissions time series
    tseries<unitval> CO_emissions;
    tseries<unitval> NOX_emissions;
    tseries<unitval> NMVOC_emissions;
   
    // logger
    Logger logger;

	Core *core;
    double oldDate;
};

}

#endif // OH_COMPONENT_H

