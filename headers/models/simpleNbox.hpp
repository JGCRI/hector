/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef SIMPLE_N_BOX_HPP_
#define SIMPLE_N_BOX_HPP_

/* SimpleNbox.hpp
 *
 * Header file for the simple box model.
 *
 */

#include "components/temperature_component.hpp"
#include "components/ocean_component.hpp"
#include "data/tseries.hpp"
#include "data/unitval.hpp"
#include "models/carbon-cycle-model.hpp"

#define SNBOX_ATMOS 0
#define SNBOX_VEG 1
#define SNBOX_DET 2
#define SNBOX_SOIL 3
#define SNBOX_OCEAN 4
#define SNBOX_EARTH 5
#define MB_EPSILON 0.001                //!< allowed tolerance for mass-balance checks, Pg C
#define SNBOX_PARSECHAR "."             //!< input separator between <biome> and <pool>
#define SNBOX_DEFAULT_BIOME "global"    //!< value if no biome supplied

namespace Hector {

/*! \brief The simple global carbon model, not including the ocean
 *
 *  SimpleNbox tracks atmosphere (1 pool), land (3 pools), ocean (1 pool from its p.o.v.), 
 *  and earth (1 pool). The ocean component handles ocean processes; SimpleNbox just
 *  tracks the total ocean C.
 */
class SimpleNbox : public CarbonCycleModel {
    friend class CSVOutputVisitor;
    friend class CSVOutputStreamVisitor;
    
public:
    SimpleNbox();
    ~SimpleNbox() {}
    
    // overrides for IModelComponent methods
    std::string getComponentName() const { return std::string( SIMPLENBOX_COMPONENT_NAME ); }
    
    virtual void init( Core* core );
    
    virtual unitval sendMessage( const std::string& message,
                                const std::string& datum,
                                const message_data info=message_data() ) throw ( h_exception );
    
    virtual void setData( const std::string& varName,
                          const message_data& data ) throw ( h_exception );
    
    virtual void prepareToRun() throw ( h_exception );
    
    virtual void run( const double runToDate ) throw ( h_exception );
    
    virtual bool run_spinup( const int step ) throw ( h_exception );
    
    virtual void reset(double date) throw(h_exception); 

    virtual void shutDown();
    
    // IVisitable methods
    virtual void accept( AVisitor* visitor );
    
    // Carbon cycle model interface
    void getCValues( double t, double c[]);
    int  calcderivs( double t, const double c[], double dcdt[] ) const;
    void slowparameval( double t, const double c[] );
    void stashCValues( double t, const double c[] );
    void record_state(double t);                        //!< record the state variables at the end of the time step 
    
private:
    virtual unitval getData( const std::string& varName,
                            const double date ) throw ( h_exception );

    // typedefs for two map types, to make things easier
    // TODO: these should probably be defined in h_util.hpp or someplace similar?
    typedef std::map<std::string, unitval> unitval_stringmap;
    typedef std::map<std::string, double> double_stringmap;

    /*****************************************************************
     * Component state
     * All of this information will be saved at the end of each time
     * step so that we can reset to any arbitrary past time.
     *****************************************************************/
    
    // Carbon pools -- global
    unitval earth_c;                //!< earth pool, Pg C; for mass-balance
    unitval atmos_c;                //!< atmosphere pool, Pg C
    unitval    Ca;                  //!< current [CO2], ppmv
    
    // Carbon pools -- biome-specific
    unitval_stringmap veg_c;        //!< vegetation pools, Pg C
    unitval_stringmap detritus_c;   //!< detritus pools, Pg C
    unitval_stringmap soil_c;       //!< soil pool, Pg C
    
    unitval residual;               //!< residual (when constraining Ca) flux, Pg C

    double_stringmap tempfertd, tempferts; //!< temperature effect on respiration (unitless)
    

    /*****************************************************************
     * Records of component state
     * These vectors record the component state over time.  When we do
     * a reset, we will retrieve the state at the reset time from these
     * arrays.
     *****************************************************************/
    tseries<unitval> earth_c_ts;  //!< Time series of earth carbon pool
    tseries<unitval> atmos_c_ts;  //!< Time series of atmosphere carbon pool
    tseries<unitval> Ca_ts;       //!< Time series of atmosphere CO2 concentration

    tvector<unitval_stringmap> veg_c_tv;      //!< Time series of biome-specific vegetation carbon pools
    tvector<unitval_stringmap> detritus_c_tv; //!< Time series of biome-specific detritus carbon pools
    tvector<unitval_stringmap> soil_c_tv;     //!< Time series of biome-specific soil carbon pools

    tseries<unitval> residual_ts; //!< Time series of residual flux values

    tvector<double_stringmap> tempfertd_tv, tempferts_tv; //!< Time series of temperature effect on respiration


    /*****************************************************************
     * Derived quantities
     * Unlike state varaibles, these can be calculated from other
     * information; therefore, they need not be stored over time, but
     * they do need to be recalculated whenever we reset.
     *****************************************************************/
    
    double_stringmap co2fert;           //!< CO2 fertilization effect (unitless)
    tseries<double> Tgav_record;        //!< Record of global temperature values, for computing soil RH
    bool in_spinup;                     //!< flag tracking spinup state
    double tcurrent;                    //!< Current time (last completed time step)
    double masstot;                     //!< tracker for mass conservation

    
    /*****************************************************************
     * Input data
     * This information isn't part of the state; it's either read from
     * an input file or pushed in by another model.
     *****************************************************************/
    
    // Carbon fluxes
    tseries<unitval> ffiEmissions;  //!< fossil fuels and industry emissions, Pg C/yr
    tseries<unitval> lucEmissions;      //!< land use change emissions, Pg C/yr
    
    // Albedo
    tseries<unitval> Ftalbedo;   //!< terrestrial albedo forcing, W/m2

    // Constraints
    tseries<unitval> Ca_constrain;      //!< input [CO2] record to constrain model to
    
    /*****************************************************************
     * Model parameters
     * If you change any of these (in a Monte Carlo run, for example),
     * you will at the very least need to reset to the beginning of the
     * run.  You may need to redo the spinup.
     *****************************************************************/
    
    // Partitioning
    double f_nppv, f_nppd;      //!< fraction NPP into vegetation and detritus
    double f_litterd;           //!< fraction of litter to detritus
    double f_lucv, f_lucd;      //!< fraction LUC from vegetation and detritus
    
    // Initial fluxes
    unitval_stringmap npp_flux0;       //!< preindustrial NPP

    // Atmospheric CO2, temperature, and their effects
    unitval    C0;                      //!< preindustrial [CO2], ppmv
    
    double_stringmap beta,           //!< shape of CO2 response
    //                        sigma,          //!< shape of temperature response (not yet implemented)
        warmingfactor;  //!< regional warming relative to global (1.0=same)
    
    double q10_rh;                      //!< Q10 for heterotrophic respiration (unitless)

    /*****************************************************************
     * Functions computing sub-elements of the carbon cycle
     *****************************************************************/
    unitval npp( std::string biome ) const;     //!< calculates current NPP for a biome
    unitval sum_npp() const;                    //!< calculates current NPP, global total
    unitval rh_fda( std::string biome ) const;  //!< calculates current RH from detritus for a biome
    unitval rh_fsa( std::string biome ) const;  //!< calculates current RH from soil for a biome
    unitval rh( std::string biome ) const;      //!< calculates current RH for a biome
    unitval sum_rh() const;                     //!< calculates current RH, global total
    
    /*****************************************************************
     * Private helper functions
     *****************************************************************/
    void sanitychecks() throw( h_exception );           //!< performs mass-balance and other checks
    unitval sum_map( unitval_stringmap pool ) const;    //!< sums a unitval map (collection of data)
    double sum_map( double_stringmap pool ) const;      //!< sums a double map (collection of data)
    void log_pools( const double t );                   //!< prints pool status to the log file
    
    CarbonCycleModel *omodel;           //!< pointer to the ocean model in use

};

}

#endif
