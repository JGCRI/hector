/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef CORE_H
#define CORE_H
/*
 *  core.h - The core model which will manage model components
 *  hector
 *
 *  Created by Pralit on 10/29/10.
 *
 */

#include <map>
#include <string>
#include <vector>
#include <algorithm>

#include "logger.hpp"
#include "h_exception.hpp"
#include "ivisitable.hpp"

namespace Hector {
  
class unitval;
struct message_data;
class IModelComponent;

//------------------------------------------------------------------------------
/*! \brief Core class.
 *
 *  Creates model components and manages them.
 */
class Core : public IVisitable {
public:
    Core(Logger::LogLevel loglvl = Logger::DEBUG, bool echotoscreen=true, bool echotofile=true);
    ~Core();
    
    const std::string& getComponentName() const;
    
    void init();
    
    void setData( const std::string& componentName, const std::string& varName,
                  const message_data& data ) throw ( h_exception );
    
    void addVisitor( AVisitor* visitor );
    
    void prepareToRun() throw ( h_exception );
    
    void run(double runtodate=-1.0) throw ( h_exception );

    void reset(double resetdate);
    
    void shutDown();

    Logger &getGlobalLogger() {return glog;}
    
    IModelComponent* getComponentByCapability( const std::string& capabilityName
                                              ) const throw ( h_exception );
    
    void registerCapability(const std::string& capabilityName, const std::string& componentName, bool warndupe=true
                            )  throw ( h_exception );
    
    int checkCapability( const std::string& capabilityName );

    void registerDependency( const std::string& capabilityName, const std::string& componentName );
    void registerInput(const std::string &inputName, const std::string &componentName);
    
    unitval sendMessage( const std::string& message,
                        const std::string& datum ) throw ( h_exception );
    
    unitval sendMessage( const std::string& message,
                        const std::string& datum,
                        const message_data& info ) throw ( h_exception );
    
    double getStartDate() const { return startDate; };
    double getEndDate() const { return endDate; };
    double getCurrentDate() const {return lastDate;}
    std::string getRun_name() const { return run_name; };
    bool inSpinup() const { return in_spinup; };
    bool outputEnabled( std::string componentName ) { return std::find( disabledOutputComponents.begin(),
                disabledOutputComponents.end(), componentName) == disabledOutputComponents.end(); }
    void addModelComponent( IModelComponent* modelComponent ) throw ( h_exception );
    
    // IVisitable methods
    virtual void accept( AVisitor* visitor );
    
    static double undefinedIndex();
    
    IModelComponent* getComponentByName( const std::string& componentName
                                        ) const throw ( h_exception );

    //! Manage cores in the global registry
    static int mkcore(bool logtofile=false,
                      Logger::LogLevel loglvl=Logger::NOTICE, 
                      bool logtoscrn=false);
    static Core *getcore(int idx);
    static void delcore(int idx);

    std::vector<std::string> getBiomeList() const;
    void createBiome(const std::string& biome);
    void deleteBiome(const std::string& biome);
    void renameBiome(const std::string& oldname, const std::string& newname);

private:
    //! Registry of instantiated cores 
    //! \details This is used when you are instantiating hector cores
    //! from a language other than C++.  Instead of trying to convert
    //! C++ pointers to something that can be stored in the other
    //! language's data structures, you just register the core(s) you
    //! create in this vector and refer to them by index.
    static std::vector<Core *> core_registry;

    Logger glog;

    // indicator for whether setup has been completed.  See notes in the body of
    // prepareToRun.
    bool setup_complete;
        
        
    //! Cause all components to run their spinup procedure.
    bool run_spinup();

    
    //------------------------------------------------------------------------------
    //! Current run name.
    std::string run_name;
    
    //------------------------------------------------------------------------------
    //! The start date for the models.
    double startDate;
    
    //------------------------------------------------------------------------------
    //! The end date for the models.
    double endDate;

    //------------------------------------------------------------------------------
    //! The last date we've run up to
    double lastDate; 
    
    //------------------------------------------------------------------------------
    //! A flag to indicate that the core has been initialized.
    bool isInited;
    
    //------------------------------------------------------------------------------
    //! A flag (can be set from input) to indicate whether to spin up.
    bool do_spinup;
    
    //------------------------------------------------------------------------------
    //! Maximum number of spinup steps allowed.
    int max_spinup;
    
    //------------------------------------------------------------------------------
    //! A comparison object to ensure modelComponents are ordered according to
    //! dependencies.
    class DependencyOrderingComparator {
    public:
        DependencyOrderingComparator(){
        }
        DependencyOrderingComparator( std::vector<std::string> ordering ):orderedComponents( ordering )
        {
        }
        
        bool operator()( const std::string& lhs, const std::string& rhs ) const {
            std::vector<std::string>::const_iterator lhsIt =
            find( orderedComponents.begin(), orderedComponents.end(), lhs );
            std::vector<std::string>::const_iterator rhsIt =
            find( orderedComponents.begin(), orderedComponents.end(), rhs );
            
            // If both sides are not explicitly ordered then just order by name
            if( lhsIt == orderedComponents.end() && rhsIt == orderedComponents.end() ) {
                return lhs < rhs;
            }
            
            return lhsIt < rhsIt;
        }
    private:
        //! An inorder list of model components by name.  Note that this list
        //! is not exhaustive.  Model components not in this list are assumed to
        //! be independent.
        std::vector<std::string> orderedComponents;
    };
    
    // A named list of model components to run.
    std::map<std::string, IModelComponent*, DependencyOrderingComparator> modelComponents;
    
    // A map of component capabilities (as reported by the components).
    std::multimap<std::string, std::string> componentCapabilities;
    
    // A map of component dependencies (depending on CAPABILITY, not component name).
    std::multimap<std::string, std::string> componentDependencies;

    // Map of component inputs, as reported by the components.  This
    // map doesn't play any role in establishing the dependency graph
    // (see capabilities for that).  Instead it's used by outside
    // functions to figure out where to send the data they're pushing
    // into the model (e.g., emissions).
    std::multimap<std::string, std::string> componentInputs;
    
    // A list of components that have been disabled
    // When a component is disabled, it still receives input data
    // But its capabilities aren't honored, and it won't be called
    std::vector<std::string> disabledComponents;

    // A list of components whose output has been disabled
    std::vector<std::string> disabledOutputComponents;

    // Some helpful typedefs to clean up syntax
    typedef std::multimap<std::string, std::string>::iterator componentMapIterator;
    typedef std::map<std::string, IModelComponent*,
    DependencyOrderingComparator>::iterator NameComponentIterator;
    typedef std::map<std::string, IModelComponent*,
    DependencyOrderingComparator>::const_iterator CNameComponentIterator;
    
    
    //! Flag: are we currently in spinup mode?
    bool in_spinup;
    
    //! List of visitors which may need to take action after a model time-step.
    std::vector<AVisitor*> modelVisitors;
    // Some helpful typedefs to clean up syntax
    typedef std::vector<AVisitor*>::iterator VisitorIterator;
};

}

#endif // CORE_H
