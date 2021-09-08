/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef FORCING_COMPONENT_H
#define FORCING_COMPONENT_H
/*
 *  forcing_component.hpp
 *  hector
 *
 *  Created by Ben on 02 March 2011.
 *
 */

#include "imodel_component.hpp"
#include "tseries.hpp"
#include "tvector.hpp"

namespace Hector {

// Need to forward declare the components which depend on each other
class SimpleNbox;
class HalocarbonComponent;


//------------------------------------------------------------------------------
/*! \brief The forcing component.
 *
 */
class ForcingComponent : public IModelComponent {
    friend class CSVOutputStreamVisitor;
public:

    ForcingComponent();
    ~ForcingComponent();

    //! IModelComponent methods
    virtual std::string getComponentName() const;

    virtual void init( Core* core );

    virtual unitval sendMessage( const std::string& message,
                                const std::string& datum,
                                const message_data info=message_data() );

    virtual void setData( const std::string& varName,
                          const message_data& data );

    virtual void prepareToRun();

    virtual void run( const double runToDate );

    virtual void reset(double date);

    virtual void shutDown();

    //! IVisitable methods
    virtual void accept( AVisitor* visitor );

    //! A list (map) of all computed forcings, with associated iterator
    typedef std::map<std::string, unitval > forcings_t;
    typedef std::map<std::string, unitval >::iterator forcingsIterator;

private:
    virtual unitval getData( const std::string& varName,
                            const double valueIndex );

    //! Base year forcings
    forcings_t baseyear_forcings;
    //! Forcings by year
    tvector<forcings_t> forcings_ts;

    double baseyear;        //! Year which forcing calculations will start
    double currentYear;     //! Tracks current year
    unitval C0;             //! Records base year atmospheric CO2
    unitval aCO2;           //! alpha CO2, forcing efficiency for CO2 (W/m2)
    tseries<unitval> Ftot_constrain; //! Total forcing can be supplied


    // CH4 parameters
    const double a3 = -8.9603e-5;       // (W m–2 ppb–1) IPCC AR6 parameter for CH4 SARF Table 7.SM.1
    const double b3 = -1.2462e-4;       // (W m–2 ppb–1) IPCC AR6 parameter for CH4 SARF Table 7.SM.1
    const double d3 = 0.045194;         // (W m–2 ppb–1/2) IPCC AR6 parameter for CH4 SARF Table 7.SM.1
    unitval delta_ch4;                  // forcing tropospheric adjustment for CH4 see 7.3.2.2 of IPCC AR6



    Core* core;             //! Core
    Logger logger;          //! Logger

    static const char *adjusted_halo_forcings[]; //! Capability strings for halocarbon forcings
    static const char *halo_forcing_names[];  //! Internal names of halocarbon forcings
    static std::map<std::string, std::string> forcing_name_map;
};

}

#endif // FORCING_COMPONENT_H
