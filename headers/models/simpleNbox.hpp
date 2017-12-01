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
    
    virtual void shutDown();
    
    // IVisitable methods
    virtual void accept( AVisitor* visitor );
    
    // Carbon cycle model interface
    void getCValues( double t, double c[]);
    int  calcderivs( double t, const double c[], double dcdt[] ) const;
    void slowparameval( double t, const double c[] );
    void stashCValues( double t, const double c[] );
    
private:
    virtual unitval getData( const std::string& varName,
                            const double date ) throw ( h_exception );

    // typedefs for two map types, to make things easier
    // TODO: these should probably be defined in h_util.hpp or someplace similar?
    typedef std::map<std::string, unitval> unitval_stringmap;
    typedef std::map<std::string, double> double_stringmap;
    
    // Carbon pools -- global
    unitval earth_c;                //!< earth pool, Pg C; for mass-balance
    unitval atmos_c;                //!< atmosphere pool, Pg C
    
    // Carbon pools -- biome-specific
    unitval_stringmap veg_c;        //!< vegetation pools, Pg C
    unitval_stringmap detritus_c;   //!< detritus pools, Pg C
    unitval_stringmap soil_c;       //!< soil pool, Pg C
    
    //    unitval ocean_c;          //!< ocean pool, Pg C
    unitval residual;               //!< residual (when constraining Ca) flux, Pg C
    
    // Carbon fluxes
    tseries<unitval> anthroEmissions;   //!< anthropogenic emissions, Pg C/yr
    tseries<unitval> lucEmissions;      //!< land use change emissions, Pg C/yr
    
    // Albedo
    tseries<unitval> Ftalbedo;   //!< terrestrial albedo forcing, W/m2
    
    // Partitioning
    double f_nppv, f_nppd;      //!< fraction NPP into vegetation and detritus
    double f_litterd;           //!< fraction of litter to detritus
    double f_lucv, f_lucd;      //!< fraction LUC from vegetation and detritus
    
    // Initial fluxes
    unitval_stringmap npp_flux0;       //!< preindustrial NPP
  
    // Functions computing sub-elements of the carbon cycle
    unitval npp( std::string biome ) const;     //!< calculates current NPP for a biome
    unitval sum_npp() const;                    //!< calculates current NPP, global total
    unitval rh_fda( std::string biome ) const;  //!< calculates current RH from detritus for a biome
    unitval rh_fsa( std::string biome ) const;  //!< calculates current RH from soil for a biome
    unitval rh( std::string biome ) const;      //!< calculates current RH for a biome
    unitval sum_rh() const;                     //!< calculates current RH, global total
    
    unitval ch4ox_flux;         //!< oxidized CH4 of fossil fuel origin; +=to atmosphere
    
    // Atmospheric CO2, temperature, and their effects
    unitval    Ca;                      //!< current [CO2], ppmv
    unitval    C0;                      //!< preindustrial [CO2], ppmv
    tseries<unitval> Ca_constrain;      //!< input [CO2] record to constrain model to
    
    double_stringmap    beta,           //!< shape of CO2 response
//                        sigma,          //!< shape of temperature response
                        warmingfactor,  //!< regional warming relative to global (1.0=same)
                        co2fert,        //!< CO2 fertilization effect (unitless)
                        tempfertd, tempferts;        //!< temperature effect on respiration (unitless)
    
    double q10_rh;                      //!< Q10 for heterotrophic respiration (unitless)
    tseries<double> Tgav_record;        //!< Record of global temperature values, for computing soil RH
    
    // Private helper functions
    void sanitychecks() throw( h_exception );           //!< performs mass-balance and other checks
    unitval sum_map( unitval_stringmap pool ) const;    //!< sums a unitval map (collection of data)
    double sum_map( double_stringmap pool ) const;      //!< sums a double map (collection of data)
    void log_pools( const double t );                   //!< prints pool status to the log file
    
    bool in_spinup;                     //!< flag tracking spinup state
    
    CarbonCycleModel *omodel;           //!< pointer to the ocean model in use

    // persistent workspace
    double m_last_tempferts;

};

}

#endif
