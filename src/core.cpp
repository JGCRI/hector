/* Hector -- A Simple Climate Model
   Copyright (C) 2022  Battelle Memorial Institute

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

#include <fstream>
// some boost headers generate warnings under clang; not our problem, ignore
// 2023 and Boost 1.81.0_1: string.hpp still generates two warnings
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "boost/algorithm/string.hpp"
#pragma clang diagnostic pop

#include "avisitor.hpp"
#include "bc_component.hpp"
#include "carbon-cycle-solver.hpp"
#include "ch4_component.hpp"
#include "core.hpp"
#include "csv_tracking_visitor.hpp"
#include "dependency_finder.hpp"
#include "forcing_component.hpp"
#include "h_util.hpp"
#include "halocarbon_component.hpp"
#include "imodel_component.hpp"
#include "logger.hpp"
#include "n2o_component.hpp"
#include "nh3_component.hpp"
#include "o3_component.hpp"
#include "oc_component.hpp"
#include "ocean_component.hpp"
#include "oh_component.hpp"
#include "simpleNbox.hpp"
#include "slr_component.hpp"
#include "so2_component.hpp"
#include "temperature_component.hpp"

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
Core::Core(Logger::LogLevel loglvl, bool echotoscreen, bool echotofile)
    : setup_complete(false), run_name(""), startDate(-1.0), endDate(-1.0),
      lastDate(-1.0), trackingDate(9999), isInited(false), do_spinup(true),
      max_spinup(2000), in_spinup(false) {
  glog.open(string(MODEL_NAME), echotoscreen, echotofile, loglvl);
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 *  \note Memory for visitors is not handled by the core.
 */
Core::~Core() {
  for (auto mc : modelComponents) {
    delete mc.second;
  }
}

//------------------------------------------------------------------------------
/*! \brief A name which suitably describes this component.
 *
 * \return A string name for the Core.
 */
const string &Core::getComponentName() const {
  const static string NAME = CORE_COMPONENT_NAME;
  return NAME;
}

//------------------------------------------------------------------------------
/*! \brief Perform initializations before setting data.
 *
 *  All model subcomponents will also be initialized.
 */
void Core::init() {
  H_ASSERT(!isInited, "core has already been initialized");

  // We handle this input; need to register it here so that R users can change
  // it
  registerInput(D_TRACKING_DATE, CORE_COMPONENT_NAME);

  IModelComponent *temp;

  temp = new SimpleNbox();
  modelComponents[temp->getComponentName()] = temp;
  temp = new CarbonCycleSolver();
  modelComponents[temp->getComponentName()] = temp;

  temp = new OHComponent();
  modelComponents[temp->getComponentName()] = temp;
  temp = new CH4Component();
  modelComponents[temp->getComponentName()] = temp;
  temp = new N2OComponent();
  modelComponents[temp->getComponentName()] = temp;

  temp = new ForcingComponent();
  modelComponents[temp->getComponentName()] = temp;
  temp = new slrComponent();
  modelComponents[temp->getComponentName()] = temp;
  temp = new OceanComponent();
  modelComponents[temp->getComponentName()] = temp;
  temp = new TemperatureComponent();
  modelComponents[temp->getComponentName()] = temp;

  temp = new HalocarbonComponent(CF4_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(C2F6_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(HFC23_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(HFC32_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(HFC4310_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(HFC125_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(HFC134a_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(HFC143a_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(HFC227ea_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(HFC245fa_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(SF6_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(HCFC22_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(CFC11_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(CFC12_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(CFC113_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(CFC114_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(CFC115_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(CCl4_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(CH3CCl3_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(HCFC141b_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(HCFC142b_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(halon1211_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(halon1301_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(halon2402_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(CH3Cl_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;
  temp = new HalocarbonComponent(CH3Br_COMPONENT_BASE);
  modelComponents[temp->getComponentName()] = temp;

  temp = new BlackCarbonComponent();
  modelComponents[temp->getComponentName()] = temp;
  temp = new OrganicCarbonComponent();
  modelComponents[temp->getComponentName()] = temp;
  temp = new NH3Component();
  modelComponents[temp->getComponentName()] = temp;
  temp = new SulfurComponent();
  modelComponents[temp->getComponentName()] = temp;
  temp = new OzoneComponent();
  modelComponents[temp->getComponentName()] = temp;

  for (auto mc : modelComponents) {
    try {
      mc.second->init(this);
    } catch (h_exception &e) {
      H_LOG(glog, Logger::SEVERE)
          << "error initializing component " << mc.first;
      throw;
    }
  }

  // Set that the core has now been initialized.
  isInited = true;
}

//------------------------------------------------------------------------------
/*! \brief Return the carbon tracking data stored in the csvFluxPoolVisitor
 */
std::string Core::getTrackingData() const {

  // use a string output stream to collect the results
  ostringstream tracking_out;
  for (auto visitorIt : modelVisitors) {
    visitorIt->outputTrackingData(tracking_out);
  }
  return tracking_out.str();
}

//------------------------------------------------------------------------------
/*! \brief Route a setData to the component specified by componentName or parse
 *         the data if componentName is equal to Core::getComponentName().
 *  \param componentName The name of the component to forward the setData to.
 *  \param data The message_data containing the information to set.
 *  \exception h_exception If either the componentName or varName was not
 * recognized.
 */
void Core::setData(const string &componentName, const string &varName,
                   const message_data &data) {
  if (componentName == getComponentName()) {
    try {
      if (varName == D_RUN_NAME) {
        H_ASSERT(data.date == undefinedIndex(), "date not allowed");
        run_name = data.value_str;
      } else if (varName == D_START_DATE) {
        H_ASSERT(data.date == undefinedIndex(), "date not allowed");
        lastDate = startDate = data.getUnitval(U_UNDEFINED);
      } else if (varName == D_END_DATE) {
        H_ASSERT(data.date == undefinedIndex(), "date not allowed");
        endDate = data.getUnitval(U_UNDEFINED);
      } else if (varName == D_TRACKING_DATE) {
        H_ASSERT(data.date == undefinedIndex(), "date not allowed");
        trackingDate = data.getUnitval(U_UNITLESS);
      } else if (varName == D_DO_SPINUP) {
        H_ASSERT(data.date == undefinedIndex(), "date not allowed");
        do_spinup = (data.getUnitval(U_UNDEFINED) > 0);
      } else if (varName == D_MAX_SPINUP) {
        H_ASSERT(data.date == undefinedIndex(), "date not allowed");
        max_spinup = data.getUnitval(U_UNDEFINED);
      } else {
        H_THROW("Unknown variable name while parsing " + getComponentName() +
                ": " + varName);
      }
    } catch (h_exception &parseException) {
      H_RETHROW(parseException, "Could not parse var: " + varName);
    }
  } else { // data is not intended for us
    IModelComponent *component = getComponentByName(componentName);

    if (varName == D_ENABLED) {
      // The core intercepts "enabled=xxx" lines to mark components as disabled
      if (data.getUnitval(U_UNDEFINED) <= 0) {
        H_LOG(glog, Logger::WARNING) << "Disabling " << componentName << endl;
        disabledComponents.push_back(componentName);
      }
    } else if (varName == D_OUTPUT_ENABLED) {
      // The core intercepts "output=xxx" lines to mark components as disabled
      if (data.getUnitval(U_UNDEFINED) <= 0) {
        H_LOG(glog, Logger::WARNING)
            << "Disabling output for " << componentName << endl;
        disabledOutputComponents.push_back(componentName);
      }
    } else {
      component->setData(varName, data); // route data
    }
  }
}

//------------------------------------------------------------------------------
/*! \brief Add a visitor which will be called after each model time-step.
 *
 *  Each set visitor will be called after all models have solved a time-step. It
 *  is up to the visitor to determine if it needs to do anything at that date.
 *  Visitors can be guaranteed to be called in the order in which they were
 * added.
 *
 *  \param visitor A visitor to add.
 *  \note The memory for the given visitor must still be managed by the caller.
 */
void Core::addVisitor(AVisitor *visitor) {
  H_LOG(glog, Logger::DEBUG) << "Core adding a visitor" << endl;

  modelVisitors.push_back(visitor);
}

//------------------------------------------------------------------------------
/*! \brief Prepare model components to run
 *  \details This subroutine does the last phase of the setup.  It comprises all
 *           of the things that need to be done after parsing is complete but
 * before we begin the runs proper.  As such, this subroutine should be called
 * only once per run.  The steps performed are:
 *
 *           1) Remove disabled components
 *           2) Construct dependency graph
 *           3) Topological sort components over dependency graph
 *           4) Call each component's prepareToRun() subroutine
 *           5) Run spin-up, if required
 *
 *  \exception h_exception An error which may occur at any stage of the process.
 */
void Core::prepareToRun(void) {
  /* Most of this stuff only needs to be done once, even if we reset the
   * model; therefore it would be a good candidate to go in init instead of
   * here.  However, in order to remove disabled components we have to first
   * read the input file so that we know which ones are meant to be disabled.
   * The input file doesn't get read until after init is run, so we have to
   * defer this setup until here and guard it so that it only gets run
   * once. */
  if (!setup_complete) {
    // ------------------------------------
    // 1. Go through the list of components that have been disabled and remove
    // them from the capability and component lists

    for (auto dc : disabledComponents) {
      H_LOG(glog, Logger::WARNING) << "Disabling " << dc << endl;
      IModelComponent *mcomp = getComponentByName(dc);
      mcomp->shutDown();
      delete mcomp;
      modelComponents.erase(dc);

      componentMapIterator it2 = componentCapabilities.begin();
      while (it2 != componentCapabilities.end()) {
        if (it2->second == dc) {
          H_LOG(glog, Logger::DEBUG)
              << "--erasing " << it2->first << " " << it2->second << endl;
          componentCapabilities.erase(it2++);
        } else {
          ++it2;
        }
      } // while
    }   // for

    // ------------------------------------
    // 2. At this point all components should have registered both their
    // capabilities and dependencies. The latter are registered as dependencies
    // on capabilities, so now we go through the componentDependencies map, find
    // the associated component, and register the link with depFinder.
    DependencyFinder depFinder;
    H_LOG(glog, Logger::NOTICE)
        << "Computing dependencies and re-ordering components..." << endl;
    for (auto dep : componentDependencies) {
      if (checkCapability(dep.second)) {
        depFinder.addDependency(
            dep.first,
            getComponentByCapability(dep.second)->getComponentName());
      } else {
        H_LOG(glog, Logger::SEVERE)
            << "Capability " << dep.second << " not found but requested by "
            << dep.first << endl;
        H_LOG(glog, Logger::WARNING)
            << "The model will almost certainly not run successfully!" << endl;
      }
    }

    // ------------------------------------
    // 3. Now that all dependency information has been resolved and entered,
    // create an ordering, and then re-sort the modelComponents map based on
    // this new ordering
    depFinder.createOrdering();
    DependencyOrderingComparator comp(depFinder.getOrdering());
    modelComponents =
        map<string, IModelComponent *, DependencyOrderingComparator>(
            modelComponents.begin(), modelComponents.end(), comp);
  }
  setup_complete = true;

  /* Everything from here on down is ok to run more than once */
  // ------------------------------------
  // 4. Tell model components we are finished sending data and about to start
  // running.
  H_LOG(glog, Logger::NOTICE) << "Preparing to run..." << endl;
  for (auto mc : modelComponents) {
    mc.second->prepareToRun();
  }

  // ------------------------------------
  // Visit all the visitors; this lets them record the core pointer, tracking
  // date, etc.
  for (auto vis : modelVisitors) {
    vis->visit(this);
  } // for

  // ------------------------------------
  // 5. Spin up the model
  if (do_spinup) {
    H_LOG(glog, Logger::NOTICE) << "Spinning up model..." << endl;
    run_spinup();
  } else {
    H_LOG(glog, Logger::WARNING) << "No model spinup was requested" << endl;
  } // if
}

bool Core::run_spinup() {
  in_spinup = true;
  bool spunup = false;
  int step = 0;
  while (!spunup && ++step < max_spinup) {
    spunup = true;
    for (auto mc : modelComponents)
      spunup = spunup && mc.second->run_spinup(step);
    // Let visitors attempt to collect data if necessary
    for (auto vis : modelVisitors) {
      if (vis->shouldVisit(in_spinup, step)) {
        accept(vis);
      }
    } // for
  }   // while

  if (spunup) {
    H_LOG(glog, Logger::NOTICE)
        << "Model spun up after " << step << " steps" << endl;
  } else {
    H_LOG(glog, Logger::SEVERE)
        << "Model failed to spin up after " << step << " steps" << endl;
  }
  in_spinup = false;

  return spunup;
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
void Core::run(double runtodate) {
  if (runtodate < 0.0) {
    // run to the configured default enddate.  This is mainly for
    // backward compatibility.  The input run-to date will always
    // override the enddate stored in the core object.
    runtodate = endDate;
  } else if (runtodate < lastDate + 1) {
    H_LOG(glog, Logger::WARNING)
        << "Requested run-to date is less than 1+lastDate.  Models not run."
        << endl;
    return;
  } else if (runtodate > endDate) {
    H_LOG(glog, Logger::WARNING)
        << "Requested run-to date is after the configured end date.  "
        << "Components are not guaranteed to be valid after endDate." << endl;
  }

  // ------------------------------------
  // 6. Run all model dates.
  H_LOG(glog, Logger::NOTICE) << "Running..." << endl;

  // Let visitors record initial model state
  // Note that just for this initial output, in calling shouldVisit below, we're
  // telling visitors that spinup is 'true' (which it's not). This is necessary
  // because the ocean component hasn't had a chance to initialize its active
  // chemistry yet, and without that calc_revelle() will throw an error. Kind of
  // hacky; but the only alternative I see is to create a whole new
  // done_with_spinup() signal to the components--a lot of work.
  for (auto visitorIt : modelVisitors) {
    if (visitorIt->shouldVisit(true, lastDate)) {
      accept(visitorIt);
    }
  }

  // Main model run loop
  for (double currDate = lastDate + 1.0; currDate <= runtodate;
       currDate += 1.0) {
    H_LOG(glog, Logger::NOTICE) << currDate << endl;

    // If we've hit the tracking start year, note this in the log
    if (currDate == trackingDate) {
      H_LOG(glog, Logger::NOTICE)
          << "Starting tracking in " << currDate << endl;
      // nb components are responsible for checking and acting on this
    }

    for (auto it : modelComponents) {
      it.second->run(currDate);
    }

    // Let visitors attempt to collect data if necessary
    for (auto vis : modelVisitors) {
      if (vis->shouldVisit(in_spinup, currDate)) {
        accept(vis);
      }
    }
  }

  // Record the last finished date; we will resume here the next time run is
  // called
  lastDate = runtodate;
}

void Core::reset(double resetdate) {
  bool rerun_spinup = false;
  H_LOG(glog, Logger::NOTICE) << "Resetting model to t= " << resetdate << endl;
  if (resetdate < getStartDate()) {
    if (do_spinup) {
      rerun_spinup = true;
      resetdate = 0; // t=0 is the first iteration of the spinup.
      H_LOG(glog, Logger::NOTICE) << "Rerunning spinup.\n";
    } else {
      H_LOG(glog, Logger::NOTICE) << "Requested reset time before start date.  "
                                  << "Resetting to start date.\n";
      resetdate = getStartDate();
    }
  }

  // Order all components to reset
  for (auto mc : modelComponents) {
    H_LOG(glog, Logger::DEBUG) << "Resetting component: " << mc.first << endl;
    mc.second->reset(resetdate);
  }

  // Inform all visitors of the reset as well
  // Currently (2021) only csvFluxPoolVisitor implements this
  for (auto vis : modelVisitors) {
    H_LOG(glog, Logger::DEBUG) << "Resetting visitor" << endl;
    vis->reset(resetdate);
  }

  // The prepareToRun function reruns all of the initial setup, including the
  // spinup. This is necessary because we may have changed some of the model
  // parameters, and for many components the parameters produce their effect
  // by influencing the initial state.
  if (rerun_spinup) {
    prepareToRun();
    lastDate = getStartDate();
  } else {
    lastDate = resetdate;
  }
}

/*! \brief Shut down all model components
 *  \details After this function is called no components are valid,
 *           and you must not call run() again.
 */
void Core::shutDown() {
  // ------------------------------------
  // 7. Tell model components we are finished.
  for (auto mc : modelComponents) {
    mc.second->shutDown();
  }
}

//------------------------------------------------------------------------------
/*! \brief Returns the model component with the associated name.
 *  \param componentName The name of the component to retrieve.
 *  \exception h_exception If the componentName was not recognized.
 */
IModelComponent *Core::getComponentByName(const string &componentName) const {
  CNameComponentIterator it = modelComponents.find(componentName);

  // throw an exception for an unknown component
  string err = "Unknown model component: " + componentName;
  H_ASSERT(it != modelComponents.end(), err);

  return (*it).second;
}

//------------------------------------------------------------------------------
/*! \brief Returns the model component with a particular capability.
 *  \param capabilityName The capability of the component to retrieve.
 *  \exception h_exception If the capabilityName was not recognized.
 */
IModelComponent *
Core::getComponentByCapability(const string &capabilityName) const {
  H_ASSERT(isInited,
           "getComponentByCapability not available until core is initialized")

  multimap<string, string>::const_iterator it =
      componentCapabilities.find(capabilityName);

  // Note that even if multiple components registered a capability, this will
  // return only the first

  // throw an exception for an unknown capability
  string err = "Unknown model capability: " + capabilityName;
  H_ASSERT(componentCapabilities.count(capabilityName), err);

  return getComponentByName((*it).second);
}

//------------------------------------------------------------------------------
/*! \brief Register a capability as associated with a component.
 *  \param capabilityName The capability of the component to register.
 *  \param componentName The name of the associated component.
 *  \param warndupe If true (the default) warn when a duplicate capability
 *                  is declared.  This should only be set to false when being
 *                  called from registerInput, which sometimes creates
 * duplicates for benign reasons. \exception h_exception If the componentName
 * was not recognized. \note By "capability" we mean any piece of data that the
 * component wants to expose to the model core or other components; this can be
 * an input, an output, or an internal variable.
 */
void Core::registerCapability(const string &capabilityName,
                              const string &componentName, bool warndupe) {
  H_ASSERT(!isInited,
           "registerCapability not available after core is initialized")

  // check whether the capability already exists
  int ncap = (int)componentCapabilities.count(capabilityName);
  if (ncap > 0 && warndupe) {
    // If this capability is a dupe, issue a warning, unless we
    // have been instructed not to.
    H_LOG(glog, Logger::WARNING)
        << componentName << " is declaring capability " << capabilityName
        << " previously registered" << endl;
  } else if (ncap == 0) {
    // Only add the capability if it doesn't already exist.
    // Adding a duplicate capability has no useful effect anyhow.
    componentCapabilities.insert(
        pair<string, string>(capabilityName, componentName));
    H_LOG(glog, Logger::DEBUG) << capabilityName << " registered to component "
                               << componentName << "\n";
  }
}

//------------------------------------------------------------------------------
/*! \brief Register a component as accepting a certain input
 *
 *  \details Associate the input capability with a component.  We also
 *           register the input as a capability (under the same name), allowing
 * other objects to query the host component for its value(s). so that other
 * components can read these values.
 *
 *  \param inputName The name of the input the component can accept
 *  \param componentName The name of the component.
 *  \note It is permissible for more than one component to accept the same
 *        inputs; however, only one of them should allow a corresponding
 *        capability to be declared.
 */
void Core::registerInput(const string &inputName, const string &componentName) {
  H_ASSERT(!isInited, "registerInput not available after core is initialized")
  componentInputs.insert(pair<string, string>(inputName, componentName));
  registerCapability(inputName, componentName, false);
}

//------------------------------------------------------------------------------
/*! \brief Check whether a capability has been registered with the core
 *  \param capabilityName The capability of the component to register.
 *  \returns int Count of the components registered to provide this.
 */
int Core::checkCapability(const string &capabilityName) {

  return (int(componentCapabilities.count(capabilityName)));
}

//------------------------------------------------------------------------------
/*! \brief Register a dependency as associated with a component.
 *  \param capabilityName The capability on which the component depends.
 *  \param componentName The name of the component.
 */
void Core::registerDependency(const string &capabilityName,
                              const string &componentName) {
  H_ASSERT(!isInited,
           "registerDependency not available after core is initialized")

  componentDependencies.insert(
      pair<string, string>(componentName, capabilityName));
}

//------------------------------------------------------------------------------
/*! \brief Look up component and send message in one operation without any need
 *         to send message data.
 *  \param message  The message to pass (typically "getData").
 *  \param datum    The datum caller is interested in.
 *  \exception h_exception If the componentName was not recognized.
 */
unitval Core::sendMessage(const std::string &message,
                          const std::string &datum) {
  return sendMessage(message, datum, message_data());
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval Core::getData(const std::string &varName, const double date) {

  unitval returnval;

  if (varName == D_TRACKING_DATE) {
    H_ASSERT(date == Core::undefinedIndex(),
             "Date not allowed for tracking date");
    returnval = unitval(trackingDate, U_UNITLESS);
  } else {
    H_THROW("Caller is requesting unknown variable: " + varName);
  }

  return returnval;
}

//------------------------------------------------------------------------------
/*! \brief Look up component and send message in one operation.
 *  \param message  The message to pass (typically "getData").
 *  \param datum    The datum caller is interested in.
 *  \param info     Extra information, message-specific.
 *  \exception h_exception If the componentName was not recognized.
 */
unitval Core::sendMessage(const std::string &message, const std::string &datum,
                          const message_data &info) {
  std::vector<std::string> datum_split;
  boost::split(datum_split, datum, boost::is_any_of(SNBOX_PARSECHAR));
  H_ASSERT(datum_split.size() < 3,
           "max of one separator allowed in variable names");
  std::string datum_capability;
  if (datum_split.size() == 2) {
    datum_capability = datum_split[1];
  } else {
    datum_capability = datum_split[0];
  }

  if (message == M_GETDATA || message == M_DUMP_TO_DEEP_OCEAN) {
    // M_GETDATA is used extensively by components to query each other re state
    // M_DUMP_TO_DEEP_OCEAN is a special message used only to constrain the
    // atmosphere We can treat it like a M_GETDATA message

    if (!isInited) {
      H_LOG(glog, Logger::SEVERE)
          << "message getData not available until core is initialized."
          << "\n\tmessage: " << message << "\tdatum: " << datum << endl;
      H_THROW("Invalid sendMessage/GETDATA.  Check global log for details.");
    } else {
      componentMapIterator it = componentCapabilities.find(datum_capability);

      string err = "Unknown model datum: " + datum;
      H_ASSERT(checkCapability(datum_capability), err);
      // Special case: core handles
      if ((*it).second == CORE_COMPONENT_NAME) {
        return this->getData(datum, info.date);
      } else {
        return getComponentByName((*it).second)
            ->sendMessage(message, datum, info);
      }
    }
  } else if (message == M_SETDATA) {
    // locate the components that take this kind of input.  If
    // there are multiple, we send the message to all of them.
    pair<componentMapIterator, componentMapIterator> itpr =
        componentInputs.equal_range(datum_capability);
    if (itpr.first == itpr.second) {
      H_LOG(glog, Logger::SEVERE)
          << "No such input: " << datum << "  Aborting.";
      H_THROW("Invalid datum in sendMessage/SETDATA.");
    }

    for (componentMapIterator it = itpr.first; it != itpr.second; ++it) {
      // Special case: core handles
      if (it->second == CORE_COMPONENT_NAME) {
        this->setData(CORE_COMPONENT_NAME, datum, info);
      } else {
        getComponentByName(it->second)->sendMessage(message, datum, info);
      }
    }

    return info.value_unitval;
  } else {
    H_LOG(glog, Logger::SEVERE)
        << "Unknown message: " << message << "  Aborting.";
    H_THROW("Invalid message type in sendMessage.");
  }
}

//------------------------------------------------------------------------------
/*! \brief Add an additional model component to be run.
 *  \param modelComponent The model component to add.
 *  \exception h_exception If the core has already been initialized model
 * components may not be added. \note The core assumes ownership of memory
 * associated with all components.
 */
void Core::addModelComponent(IModelComponent *modelComponent) {
  H_ASSERT(!isInited,
           "Model components can only be added before initialization.");

  modelComponents[modelComponent->getComponentName()] = modelComponent;
}

//------------------------------------------------------------------------------
// documentation is inherited
void Core::accept(AVisitor *visitor) {
  visitor->visit(this);

  // forward the accept to the contained model components
  for (auto mc : modelComponents) {
    mc.second->accept(visitor);
  }
}

//------------------------------------------------------------------------------
/*! \brief Return the constant value used to signify a time series index has not
 *         been set.
 *  \return A constant double value which can be compared against to indicate
 *          and index value has not been set.
 */
double Core::undefinedIndex() { return -1; }

std::vector<Core *> Core::core_registry;

/*! Create a core and add it to the registry
 */
int Core::mkcore(bool logtofile, Logger::LogLevel loglvl, bool logtoscrn) {
  // Create a dummy output filestream, essentially /dev/null
  std::ofstream dummy;
  CSVFluxPoolVisitor *visitr = new CSVFluxPoolVisitor(dummy);

  // Create the new core, add the visitor, and push onto the core registry
  Core *core = new Core(loglvl, logtoscrn, logtofile);
  core->addVisitor(visitr);
  core_registry.push_back(core);

  return (int)core_registry.size() - 1;
}

/*! Get a core by index
 *
 * \details Return a pointer to the core corresponding to the input
 * index.  If an invalid index is passed, return a null pointer.
 */
Core *Core::getcore(std::vector<Core *>::size_type idx) {
  if (idx < core_registry.size() && idx >= 0) {
    return core_registry[idx];
  } else {
    return NULL;
  }
}

/*! Shutdown a core in the global registry
 * \details Call the core's shutdown method, delete the core object,
 * and set its entry in the registry to a null pointer. If an invalid
 * index is passed, this is a no-op.
 */
void Core::delcore(std::vector<Core *>::size_type idx) {
  Core *core = getcore(idx);

  if (core) {
    core->shutDown();
    delete core;
    core_registry[idx] = NULL;
  }
  // If core is null, it's already been shutdown, so do nothing.
}

//! Retrieve the current biome list
std::vector<std::string> Core::getBiomeList() const {
  IModelComponent *cmodel_i = getComponentByCapability(D_VEGC);
  SimpleNbox *cmodel = dynamic_cast<SimpleNbox *>(cmodel_i);
  if (cmodel) {
    return (cmodel->getBiomeList());
  } else {
    H_THROW("Failed to retrieve biome list because of error in dynamic cast to "
            "`SimpleNbox`.")
  }
}

/*! Create a new biome
 * \details Add the biome to `biome_list`, set all pool values to
 *  zero, and set all parameters to the values of the previous biome.
 */
void Core::createBiome(const std::string &biome) {
  IModelComponent *cmodel_i = getComponentByCapability(D_VEGC);
  CarbonCycleModel *cmodel = dynamic_cast<CarbonCycleModel *>(cmodel_i);
  if (cmodel) {
    return (cmodel->createBiome(biome));
  } else {
    H_THROW("Failed to create biome because of error in dynamic cast to "
            "`SimpleNbox`.")
  }
}

/*! Delete a biome
 * \details Remove the biome from `biome_list` and `erase` all
 * associated pool and parameter values.
 */
void Core::deleteBiome(const std::string &biome) {
  IModelComponent *cmodel_i = getComponentByCapability(D_VEGC);
  CarbonCycleModel *cmodel = dynamic_cast<CarbonCycleModel *>(cmodel_i);
  if (cmodel) {
    return (cmodel->deleteBiome(biome));
  } else {
    H_THROW("Failed to delete biome because of error in dynamic cast to "
            "`SimpleNbox`.")
  }
}

/*! Rename a biome
 * \details Create a new biome called `newname` (see `createBiome`)
 * and set all pools and parameters to the values of `oldname`. Then,
 * delete `oldname` (see `deleteBiome`).
 */
void Core::renameBiome(const std::string &oldname, const std::string &newname) {
  IModelComponent *cmodel_i = getComponentByCapability(D_VEGC);
  CarbonCycleModel *cmodel = dynamic_cast<CarbonCycleModel *>(cmodel_i);
  if (cmodel) {
    return (cmodel->renameBiome(oldname, newname));
  } else {
    H_THROW("Failed to rename biome because of error in dynamic cast to "
            "`SimpleNbox`.")
  }
}

} // namespace Hector
