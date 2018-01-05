/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  core.cpp
 *  hector
 *
 *  Created by Pralit Patel on 10/29/10.
 *
 */

#include <boost/lexical_cast.hpp>

#include "components/imodel_component.hpp"
#include "components/halocarbon_component.hpp"
#include "components/oh_component.hpp"
#include "components/ch4_component.hpp"
#include "components/n2o_component.hpp"
#include "components/bc_component.hpp"
#include "components/oc_component.hpp"
#include "components/so2_component.hpp"
#include "components/forcing_component.hpp"
#include "components/slr_component.hpp"
#include "components/ocean_component.hpp"
#include "components/onelineocean_component.hpp"
#include "components/o3_component.hpp"
#include "components/temperature_component.hpp"
#include "core/core.hpp"
#include "core/dependency_finder.hpp"
#include "core/logger.hpp"
#include "core/carbon-cycle-solver.hpp"
#include "h_util.hpp"
#include "models/simpleNbox.hpp"
#include "visitors/avisitor.hpp"

namespace Hector {
  
using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 *
 *  Perform only minimal initialization.  More involved initializations should
 *  go in init.
 *
 *  \sa init()
 */
Core::Core() :
  run_name( "" ), 
  startDate( -1.0 ),
endDate( -1.0 ),
lastDate( -1.0), 
isInited( false ),
do_spinup( true ),
max_spinup( 2000 ),
in_spinup( false )
{
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 *  \note Memory for visitors is not handled by the core.
 */
Core::~Core() {
    for( CNameComponentIterator it = modelComponents.begin(); it != modelComponents.end(); ++it ) {
        delete( *it ).second;
    }
}

//------------------------------------------------------------------------------
/*! \brief A name which suitably describes this component.
 *
 * \return A string name for the Core.
 */
const string& Core::getComponentName() const {
    const static string NAME = CORE_COMPONENT_NAME;
    return NAME;
}

//------------------------------------------------------------------------------
/*! \brief Perform initializations before setting data.
 *
 *  All model subcomponents will also be initialized.
 */
void Core::init() {
    H_ASSERT( !isInited, "core has already been initialized" );
    
    // TODO: maybe some model component factory?
    IModelComponent *temp;
	
    temp = new SimpleNbox();
    modelComponents[ temp->getComponentName() ] = temp;
    temp = new CarbonCycleSolver();
    modelComponents[ temp->getComponentName() ] = temp;
    
    temp = new OHComponent();
    modelComponents[ temp->getComponentName() ] = temp;
    temp = new CH4Component();
    modelComponents[ temp->getComponentName() ] = temp;
    temp = new N2OComponent();
    modelComponents[ temp->getComponentName() ] = temp;

    temp = new OzoneComponent();
    modelComponents[ temp->getComponentName() ] = temp;

	temp = new ForcingComponent();
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new slrComponent();
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new OceanComponent();
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new OneLineOceanComponent();
    modelComponents[ temp->getComponentName() ] = temp;
    temp = new TemperatureComponent();
    modelComponents[ temp->getComponentName() ] = temp;
    
    temp = new HalocarbonComponent( CF4_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
    temp = new HalocarbonComponent( C2F6_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( HFC23_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( HFC32_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( HFC4310_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( HFC125_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
    temp = new HalocarbonComponent( HFC134a_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
    temp = new HalocarbonComponent( HFC143a_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
    temp = new HalocarbonComponent( HFC227ea_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
    temp = new HalocarbonComponent( HFC245fa_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( SF6_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( HCF22_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( CFC11_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
    temp = new HalocarbonComponent( CFC12_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( CFC113_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( CFC114_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( CFC115_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( CCl4_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( CH3CCl3_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( HCF141b_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( HCF142b_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( halon1211_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( halon1301_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( halon2402_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( CH3Cl_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new HalocarbonComponent( CH3Br_COMPONENT_BASE );
    modelComponents[ temp->getComponentName() ] = temp;
	
    temp = new BlackCarbonComponent();
    modelComponents[ temp->getComponentName() ] = temp;
    temp = new OrganicCarbonComponent();
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new SulfurComponent();
    modelComponents[ temp->getComponentName() ] = temp;
	temp = new OzoneComponent();
    modelComponents[ temp->getComponentName() ] = temp;
    
    //    Logger& glog = Logger::getGlobalLogger();
    for( NameComponentIterator it = modelComponents.begin(); it != modelComponents.end(); ++it ) {
        //        H_LOG( glog, Logger::DEBUG ) << "Initializing "
        //            << ( *it ).second->getComponentName() << endl;
        ( *it ).second->init( this );
    }
    
    // Set that the core has now been initialized.
    isInited = true;
}

//------------------------------------------------------------------------------
/*! \brief Route a setData to the component specified by componentName or parse
 *         the data if componentName is equal to Core::getComponentName().
 *  \param componentName The name of the component to forward the setData to.
 *  \param data The message_data containing the information to set.
 *  \exception h_exception If either the componentName or varName was not recognized.
 */
void Core::setData( const string& componentName, const string& varName,
                    const message_data& data ) throw ( h_exception )
{
    using namespace boost;
    
    if( componentName == getComponentName() ) {
        try {
            if( varName == D_RUN_NAME ) {
                H_ASSERT( data.date == undefinedIndex(), "date not allowed" );
                run_name = data.value_str;
            } else if( varName == D_START_DATE ) {
                H_ASSERT( data.date == undefinedIndex(), "date not allowed" );
                lastDate = startDate = lexical_cast<double>( data.value_str );
            } else if( varName == D_END_DATE ) {
                H_ASSERT( data.date == undefinedIndex(), "date not allowed" );
                endDate = lexical_cast<double>( data.value_str );
            } else if( varName == D_DO_SPINUP ) {
                H_ASSERT( data.date == undefinedIndex(), "date not allowed" );
                do_spinup = lexical_cast<bool>( data.value_str );
            } else if( varName == D_MAX_SPINUP ) {
                H_ASSERT( data.date == undefinedIndex(), "date not allowed" );
                max_spinup = lexical_cast<int>( data.value_str );
            } else {
                H_THROW( "Unknown variable name while parsing "+ getComponentName() + ": "
                        + varName );
            }
        } catch( bad_lexical_cast& castException ) {
            H_THROW( "Could not convert var: "+varName+", value: " + data.value_str + ", exception: "
                    +castException.what() );
        }
    } else {    // data is not intended for us
        IModelComponent* component = getComponentByName( componentName );
        
        if( varName == D_ENABLED ) {
            // The core intercepts "enabled=xxx" lines to mark components as disabled
            if( !lexical_cast<bool>( data.value_str ) ) {
                Logger& glog = Logger::getGlobalLogger();
                H_LOG( glog, Logger::WARNING ) << "Disabling " << componentName << endl;
                disabledComponents.push_back( componentName );
            }
        } else if( varName == D_OUTPUT_ENABLED ) {
            // The core intercepts "output=xxx" lines to mark components as disabled
            if( !lexical_cast<bool>( data.value_str ) ) {
                Logger& glog = Logger::getGlobalLogger();
                H_LOG( glog, Logger::WARNING ) << "Disabling output for " << componentName << endl;
                disabledOutputComponents.push_back( componentName );
            }
        } else {
            component->setData( varName, data );   // route data
        }
    }
}

//------------------------------------------------------------------------------
/*! \brief Add a visitor which will be called after each model time-step.
 *
 *  Each set visitor will be called after all models have solved a time-step.  It
 *  is up to the visitor to determine if it needs to do anything at that date.
 *  Visitors can be guaranteed to be called in the order in which they were added.
 *
 *  \param visitor A visitor to add.
 *  \note The memory for the given visitor must still be managed by the caller.
 */
void Core::addVisitor( AVisitor* visitor ) {
    Logger& glog = Logger::getGlobalLogger();
    H_LOG( glog, Logger::DEBUG) << "Core adding a visitor" << endl;
    
    modelVisitors.push_back( visitor );
}


//------------------------------------------------------------------------------
/*! \brief Prepare model components to run
 *  \details This subroutine does the last phase of the setup.  It comprises all
 *           of the things that need to be done after parsing is complete but before
 *           we begin the runs proper.  As such, this subroutine should be called only
 *           once per run.  The steps performed are:
 *
 *           1) Remove disabled components
 *           2) Construct dependency graph
 *           3) Topological sort components over dependency graph
 *           4) Call each component's prepareToRun() subroutine
 *           5) Run spin-up, if required
 *  
 *  \exception h_exception An error which may occur at any stage of the process.
 */
void Core::prepareToRun(void) throw (h_exception)
{
    
    Logger& glog = Logger::getGlobalLogger();
    
    // ------------------------------------
    // 1. Go through the list of components that have been disabled and remove them
    // from the capability and component lists
    
    for( vector<string>::iterator it=disabledComponents.begin(); it != disabledComponents.end(); ++it ) {
        H_LOG( glog, Logger::WARNING ) << "Disabling " << *it << endl;
        IModelComponent * mcomp = getComponentByName( *it );
        mcomp->shutDown();
        delete mcomp;
        modelComponents.erase( *it );
        
        componentMapIterator it2 = componentCapabilities.begin();
        while( it2 != componentCapabilities.end() ) {
            if( it2->second==*it ) {
                H_LOG( glog, Logger::DEBUG) << "--erasing " << it2->first << " " << it2->second << endl;
                componentCapabilities.erase( it2++ );
            } else {
                ++it2;
            }
        } // while
    } // for
    
    // ------------------------------------
    // 2. At this point all components should have registered both their capabilities
    // and dependencies. The latter are registered as dependencies on capabilities,
    // so now we go through the componentDependencies map, find the associated
    // component, and register the link with depFinder.
    DependencyFinder depFinder;
    H_LOG( glog, Logger::NOTICE ) << "Computing dependencies and re-ordering components..." << endl;
    for( componentMapIterator it = componentDependencies.begin(); it != componentDependencies.end(); ++it ) {
        //        H_LOG( glog, Logger::DEBUG) << it->first << " " << it->second << endl;
        if( checkCapability( it->second ) ) {
            depFinder.addDependency( it->first, getComponentByCapability( it->second )->getComponentName() );
        } else {
            H_LOG( glog, Logger::SEVERE) << "Capability " << it->second << " not found but requested by " << it->first << endl;
            H_LOG( glog, Logger::WARNING) << "The model will almost certainly not run successfully!" << endl;
        }
    }
    
    // ------------------------------------
    // 3. Now that all dependency information has been resolved and entered, create an ordering,
    // and then re-sort the modelComponents map based on this new ordering
    depFinder.createOrdering();
    DependencyOrderingComparator comp( depFinder.getOrdering() );
    modelComponents = map<string, IModelComponent*, DependencyOrderingComparator>(
                                                                                  modelComponents.begin(),
                                                                                  modelComponents.end(),
                                                                                  comp );
    
    // ------------------------------------
    // 4. Tell model components we are finished sending data and about to start running.
    H_LOG( glog, Logger::NOTICE) << "Preparing to run..." << endl;
    for( NameComponentIterator it = modelComponents.begin(); it != modelComponents.end(); ++it ) {
        //       H_LOG( glog, Logger::DEBUG) << "Preparing " << (*it).second->getComponentName() << " to run" << endl;
        ( *it ).second->prepareToRun();
    }
    
    // ------------------------------------
    // 5. Spin up the model
    if( do_spinup ) {
        H_LOG( glog, Logger::NOTICE) << "Spinning up model..." << endl;
        
        in_spinup = true;
        bool spunup = false;
        int step = 0;
        while( !spunup && ++step<max_spinup ) {
            spunup = true;
            for( NameComponentIterator it = modelComponents.begin(); it != modelComponents.end(); ++it )
                spunup = spunup && ( *it ).second->run_spinup( step );

            // Let visitors attempt to collect data if necessary
            for( VisitorIterator visitorIt = modelVisitors.begin(); visitorIt != modelVisitors.end(); ++visitorIt ) {
                if( ( *visitorIt )->shouldVisit( in_spinup, step ) ) {
                    accept( *visitorIt );
                }
            } // for
        } // while
        
        if( spunup ) {
            H_LOG( glog, Logger::NOTICE) << "Model spun up after " << step << " steps" << endl;
        } else {
            H_LOG( glog, Logger::SEVERE) << "Model failed to spin up after " << step << " steps" << endl;
        }
        in_spinup = false;
    } else {
        H_LOG( glog, Logger::WARNING) << "No model spinup was requested" << endl;
    } // if 
}

//------------------------------------------------------------------------------
/*! \brief Run the components for one-year time steps through runtodate
 *
 *  \details This subroutine runs the model components.  The argument
 *           runtodate determines how far to advance the model.  It's
 *           given as a double, but since the time is advanced in
 *           one-year time steps, it needs to be an integer value.
 *           For backward compatibility the runtodate argument can be
 *           omitted, in which case we run to the end date configured
 *           for the core.  The end date still serves as a guarantee
 *           to the components about the latest date they will be
 *           required to run to.  Components are not required to be
 *           valid after the end date; therefore, trying to run past
 *           the configured end date produces a warning.  However, in
 *           these cases the model will gamely try to press on in the
 *           face of adversity.  Hector is nothing if not plucky.
 *
 *           At the end of each time step visitors will visit all the
 *           model components, and then the next time step will run.
 *           Once all the time steps up to the run-to date have run,
 *           this subroutine will exit.  It can be called repeatedly
 *           with progressively larger run-to dates to pick up each
 *           time where it left off before.
 *
 *  \exception h_exception An error which may occur at any stage of the process.
 */

void Core::run(double runtodate) throw ( h_exception ) {
    Logger& glog = Logger::getGlobalLogger();
    if(runtodate < 0.0) {
        // run to the configured default enddate.  This is mainly for
        // backward compatibility.  The input run-to date will always
        // override the enddate stored in the core object.
        runtodate = endDate;
    }
    else if(runtodate < lastDate+1) {
        H_LOG(glog, Logger::WARNING)
            << "Requested run-to date is less than 1+lastDate.  Models not run." << endl;
        return;
    }
    else if(runtodate > endDate) {
        H_LOG(glog, Logger::WARNING)
            << "Requested run-to date is after the configured end date.  "
            << "Components are not guaranteed to be valid after endDate." << endl;
    }

    // ------------------------------------
    // 6. Run all model dates.
    H_LOG( glog, Logger::NOTICE) << "Running..." << endl; 
    for(double currDate = lastDate+1.0; currDate <= runtodate; currDate += 1.0 ) {
        for( NameComponentIterator it = modelComponents.begin(); it != modelComponents.end(); ++it ) {
            ( *it ).second->run( currDate );
        }
        
        // Let visitors attempt to collect data if necessary
        for( VisitorIterator visitorIt = modelVisitors.begin(); visitorIt != modelVisitors.end(); ++visitorIt ) {
            if( ( *visitorIt )->shouldVisit( in_spinup, currDate ) ) {
                accept( *visitorIt );
            }
        }
    }
    // Record the last finished date.  We will resume here the next time run is called
    lastDate = runtodate;
}

/*! \brief Shut down all model components 
 *  \details After this function is called no components are valid,
 *           and you must not call run() again.
 */
void Core::shutDown()
{
    // ------------------------------------
    // 7. Tell model components we are finished.
    for( NameComponentIterator it = modelComponents.begin(); it != modelComponents.end(); ++it ) {
        ( *it ).second->shutDown();
    }
}

//------------------------------------------------------------------------------
/*! \brief Returns the model component with the associated name.
 *  \param componentName The name of the component to retrieve.
 *  \exception h_exception If the componentName was not recognized.
 */
IModelComponent* Core::getComponentByName( const string& componentName ) const throw ( h_exception )
{
    CNameComponentIterator it = modelComponents.find( componentName );
    
    // throw an exception for an unknown component
    string err = "Unknown model component: " + componentName;
    H_ASSERT( it != modelComponents.end(), err );
    
    return ( *it ).second;
}

//------------------------------------------------------------------------------
/*! \brief Returns the model component with a particular capability.
 *  \param componentName The capability of the component to retrieve.
 *  \exception h_exception If the capabilityName was not recognized.
 */
IModelComponent* Core::getComponentByCapability( const string& capabilityName ) const throw ( h_exception )
{
    H_ASSERT( isInited, "getComponentByCapability not available until core is initialized")
    
    multimap<string,string>::const_iterator it = componentCapabilities.find( capabilityName );
    
    // Note that even if multiple components registered a capability, this will return only the first
    
    // throw an exception for an unknown capability
    string err = "Unknown model capability: " + capabilityName;
    H_ASSERT( componentCapabilities.count( capabilityName ), err );
    
    return getComponentByName( ( *it ).second );
}

//------------------------------------------------------------------------------
/*! \brief Register a capability as associated with a component.
 *  \param capabilityName The capability of the component to register.
 *  \param componentName The name of the associated component.
 *  \exception h_exception If the componentName was not recognized.
 */
void Core::registerCapability( const string& capabilityName, const string& componentName
                              )  throw ( h_exception ){
    H_ASSERT( !isInited, "registerCapability not available after core is initialized")
    
    //    Logger& glog = Logger::getGlobalLogger();
    //    H_LOG( glog, Logger::DEBUG ) << componentName << " is registering capability " << capabilityName << endl;
    
    if( componentCapabilities.count( capabilityName ) ) {
        Logger& glog = Logger::getGlobalLogger();
        H_LOG( glog, Logger::WARNING ) << componentName << " is declaring capability " << capabilityName << " previously registered" << endl;
    }
    componentCapabilities.insert( pair<string, string>( capabilityName, componentName ) );
}

//------------------------------------------------------------------------------
/*! \brief Register a component as accepting a certain input
 *  \param inputName The name of the input the component can accept
 *  \param componentName The name of the component.
 *  \note It is permissible for more than one component to accept the same
 *        inputs.
 */
void Core::registerInput( const string& inputName, const string& componentName ) {
    H_ASSERT( !isInited, "registerInput not available after core is initialized") 
    componentInputs.insert( pair<string, string>( inputName, componentName ) );
} 

//------------------------------------------------------------------------------
/*! \brief Check whether a capability has been registered with the core
 *  \param capabilityName The capability of the component to register.
 *  \param componentName The name of the associated component.
 *  \returns int Count of the components registered to provide this.
 */
int Core::checkCapability( const string& capabilityName ) {
    
    return( int( componentCapabilities.count( capabilityName ) ) );
}

//------------------------------------------------------------------------------
/*! \brief Register a dependency as associated with a component.
 *  \param capabilityName The capability on which the component depends.
 *  \param componentName The name of the component.
 */
void Core::registerDependency( const string& capabilityName, const string& componentName ) {
    H_ASSERT( !isInited, "registerDependency not available after core is initialized")
    
    //    Logger& glog = Logger::getGlobalLogger();
    //    H_LOG( glog, Logger::DEBUG) << componentName << " is registering dependency " << capabilityName << endl;
    
    componentDependencies.insert( pair<string, string>( componentName, capabilityName ) );
}
   

//------------------------------------------------------------------------------
/*! \brief Look up component and send message in one operation without any need
 *         to send message data.
 *  \param message  The message to pass (typically "getData").
 *  \param datum    The datum caller is interested in.
 *  \exception h_exception If the componentName was not recognized.
 */
unitval Core::sendMessage( const std::string& message,
                          const std::string& datum ) throw ( h_exception )
{
    return sendMessage( message, datum, message_data() );
}

//------------------------------------------------------------------------------
/*! \brief Look up component and send message in one operation.
 *  \param message  The message to pass (typically "getData").
 *  \param datum    The datum caller is interested in.
 *  \param info     Extra information, message-specific.
 *  \exception h_exception If the componentName was not recognized.
 */
unitval Core::sendMessage( const std::string& message,
                          const std::string& datum,
                          const message_data& info ) throw ( h_exception )
{
    if (message == M_GETDATA || message == M_DUMP_TO_DEEP_OCEAN) {
        // M_GETDATA is used extensively by components to query each other re state
        // M_DUMP_TO_DEEP_OCEAN is a special message used only to constrain the atmosphere
        // We can treat it like a M_GETDATA message

        if(!isInited) {
            H_LOG(Logger::getGlobalLogger(), Logger::SEVERE)
                << "message getData not available until core is initialized."
                << "\n\tmessage: " << message << "\tdatum: " << datum << endl;
            H_THROW("Invalid sendMessage/GETDATA.  Check global log for details.");
        }
        else {
            componentMapIterator it = componentCapabilities.find( datum );
            
            string err = "Unknown model datum: " + datum;
            H_ASSERT( checkCapability( datum ), err );
            return getComponentByName( ( *it ).second )->sendMessage( message, datum, info );
        }
    }
    else if (message == M_SETDATA ) {
        if( isInited ) {
            // If core initialization has been completed, then the only
            // setdata messages allowed are ones keyed to a date that we
            // haven't gotten to yet.
            if(info.date == Core::undefinedIndex()) {
                H_LOG(Logger::getGlobalLogger(), Logger::SEVERE)
                    << "Once core is initialized, the only SETDATA messages allowed are for dates after the current model date.\n"
                    << "\tdatum: " << datum
                    << "\tcurrent date: " << lastDate << "\tmessage date: " << info.date
                    << std::endl;
                H_THROW("Invalid sendMessage/SETDATA.  Check global log for details.");
            }
        }
        
        // locate the components that take this kind of input.  If
        // there are multiple, we send the message to all of them.
        pair<componentMapIterator, componentMapIterator> itpr =
            componentInputs.equal_range(datum);
        if(itpr.first == itpr.second) {
            H_LOG(Logger::getGlobalLogger(), Logger::SEVERE)
                << "No such input: " << datum << "  Aborting.";
            H_THROW("Invalid datum in sendMessage/SETDATA.");
        }
        for(componentMapIterator it=itpr.first; it != itpr.second; ++it)
            getComponentByName(it->second)->sendMessage(message, datum, info);
        
        return info.value_unitval;
    }
    else {
        H_LOG(Logger::getGlobalLogger(), Logger::SEVERE)
            << "Unknown message: " << message << "  Aborting.";
        H_THROW("Invalid message type in sendMessage.");
    } 
}
    
//------------------------------------------------------------------------------
/*! \brief Add an additional model component to be run.
 *  \param modelComponent The model component to add.
 *  \exception h_exception If the core has already been initialized model components
 *                         may not be added.
 *  \note The core assumes ownership of memory associated with all components.
 */
void Core::addModelComponent( IModelComponent* modelComponent ) throw ( h_exception ) {
    H_ASSERT( !isInited, "Model components can only be added before initialization." );
    
    modelComponents[ modelComponent->getComponentName() ] = modelComponent;
}

//------------------------------------------------------------------------------
// documentation is inherited
void Core::accept( AVisitor* visitor ) {
    visitor->visit( this );
    
    // forward the accept to the contained model components
    for( NameComponentIterator it = modelComponents.begin(); it != modelComponents.end(); ++it ) {
        ( *it ).second->accept( visitor );
    }
}

//------------------------------------------------------------------------------
/*! \brief Return the constant value used to signify a time series index has not
 *         been set.
 *  \return A constant double value which can be compared against to indicate
 *          and index value has not been set.
 */
double Core::undefinedIndex() {
    return -1;
}

}
