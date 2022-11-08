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

#include "carbon-cycle-model.hpp"
#include "fluxpool.hpp"
#include "ocean_component.hpp"
#include "temperature_component.hpp"
#include "tseries.hpp"
#include "unitval.hpp"

#define SNBOX_ATMOS 0
#define SNBOX_VEG 1
#define SNBOX_DET 2
#define SNBOX_SOIL 3
#define SNBOX_OCEAN 4
#define SNBOX_EARTH 5
#define MB_EPSILON 0.001    //!< allowed tolerance for mass-balance checks, Pg C
#define SNBOX_PARSECHAR "." //!< input separator between <biome> and <pool>
#define SNBOX_DEFAULT_BIOME "global" //!< value if no biome supplied

namespace Hector {

/*! \brief The simple global carbon model, not including the ocean
 *
 *  SimpleNbox tracks atmosphere (1 pool), land (3 pools), ocean (1 pool from
 * its p.o.v.), and earth (1 pool). The ocean component handles ocean processes;
 * SimpleNbox just tracks the total ocean C.
 */
class SimpleNbox : public CarbonCycleModel {
  friend class CSVOutputVisitor;
  friend class CSVOutputStreamVisitor;
  friend class CSVFluxPoolVisitor;

public:
  SimpleNbox();
  ~SimpleNbox() {}

  // overrides for IModelComponent methods
  std::string getComponentName() const {
    return std::string(SIMPLENBOX_COMPONENT_NAME);
  }
  std::vector<std::string> getBiomeList() const { return (biome_list); }

  virtual void init(Core *core);

  virtual unitval sendMessage(const std::string &message,
                              const std::string &datum,
                              const message_data info = message_data());

  virtual void setData(const std::string &varName, const message_data &data);

  virtual void prepareToRun();

  virtual void run(const double runToDate);

  virtual bool run_spinup(const int step);

  virtual void reset(double date);

  virtual void shutDown();

  // IVisitable methods
  virtual void accept(AVisitor *visitor);

  // Carbon cycle model interface
  void getCValues(double t, double c[]);
  int calcderivs(double t, const double c[], double dcdt[]) const;
  void slowparameval(double t, const double c[]);
  void stashCValues(double t, const double c[]);
  void record_state(
      double t); //!< record the state variables at the end of the time step

  void createBiome(const std::string &biome);
  void deleteBiome(const std::string &biome);
  void renameBiome(const std::string &oldname, const std::string &newname);

private:
  virtual unitval getData(const std::string &varName, const double date);

  // typedefs for two map types, to make things easier
  // TODO: these should probably be defined in h_util.hpp or someplace similar?
  typedef std::map<std::string, fluxpool> fluxpool_stringmap;
  typedef std::map<std::string, double> double_stringmap;

  bool has_been_run_before; //!<  Has run() been called once already?
  
  /*****************************************************************
   * Component state
   * All of this information will be saved at the end of each time
   * step so that we can reset to any arbitrary past time.
   *****************************************************************/

  // List of biomes
  std::vector<std::string> biome_list;

  // Carbon pools -- global
  fluxpool earth_c; //!< earth pool, Pg C; for mass-balance
  fluxpool atmos_c; //!< atmosphere pool, Pg C

  // Carbon pools -- biome-specific
  fluxpool_stringmap veg_c;      //!< vegetation pools, Pg C
  fluxpool_stringmap detritus_c; //!< detritus pools, Pg C
  fluxpool_stringmap soil_c;     //!< soil pool, Pg C

  // Carbon fluxes -- biome-specific
  fluxpool_stringmap
      final_npp; //!< final NPP after any NBP constraint accounted for, Pg C/yr
  fluxpool_stringmap
      final_rh; //!< final RH after any NBP constraint accounted for, Pg C/yr

  unitval Ca_residual; //!< residual (when constraining [CO2]) flux, Pg C

  double_stringmap tempfertd,
      tempferts; //!< temperature effect on respiration (unitless)

  /*****************************************************************
   * Records of component state
   * These vectors record the component state over time.  When we do
   * a reset, we will retrieve the state at the reset time from these
   * arrays.
   *****************************************************************/
  tseries<fluxpool> earth_c_ts; //!< Time series of earth carbon pool
  tseries<fluxpool> atmos_c_ts; //!< Time series of atmosphere carbon pool

  tvector<fluxpool_stringmap>
      veg_c_tv; //!< Time series of biome-specific vegetation carbon pools
  tvector<fluxpool_stringmap>
      detritus_c_tv; //!< Time series of biome-specific detritus carbon pools
  tvector<fluxpool_stringmap>
      soil_c_tv; //!< Time series of biome-specific soil carbon pools
  tvector<fluxpool_stringmap>
      final_npp_tv; //!< Time series of biome-specific final NPP
  tvector<fluxpool_stringmap>
      final_rh_tv; //!< Time series of biome-specific final RH

  tseries<unitval> Ca_residual_ts; //!< Time series of residual flux values

  tvector<double_stringmap> tempfertd_tv,
      tempferts_tv; //!< Time series of temperature effect on respiration

  /*****************************************************************
   * Derived quantities
   * Unlike state variables, these can be calculated from other
   * information; therefore, they need not be stored over time, but
   * they do need to be recalculated whenever we reset.
   *****************************************************************/

  double_stringmap co2fert;     //!< CO2 fertilization effect (unitless)
  tseries<double> Tland_record; //!< Record of mean land surface/air temperature
                                //!< values, for computing soil RH
  bool in_spinup;               //!< flag tracking spinup state
  double tcurrent;              //!< Current time (last completed time step)
  double masstot;               //!< tracker for mass conservation
  unitval nbp; //!< Atmosphere -> land C flux (=net biome production, positive
               //!< is to land)
  tseries<unitval> nbp_ts; //!< Atmosphere -> land C flux (time series)

  /*****************************************************************
   * Input data
   * This information isn't part of the state; it's either read from
   * an input file or pushed in by another model.
   *****************************************************************/

  // Carbon fluxes
  tseries<fluxpool>
      ffiEmissions; //!< fossil fuels and industry emissions, Pg C/yr
  tseries<fluxpool>
      daccsUptake; //!< direct air carbon capture and storage, Pg C/yr
  tseries<fluxpool> lucEmissions; //!< land use change emissions, Pg C/yr
  tseries<fluxpool> lucUptake;    //!< land use change uptake, Pg C/yr

  // Albedo
  tseries<unitval> Falbedo; //!< terrestrial albedo forcing, W/m2

  // Constraints
  tseries<fluxpool> CO2_constrain; //!< input [CO2] record to constrain model to
  tseries<unitval>
      NBP_constrain; //!< input net biome production (atmosphere-land C flux,
                     //!< positive = flux to land) to constrain model to

  /*****************************************************************
   * Model parameters
   * If you change any of these (in a Monte Carlo run, for example),
   * you will at the very least need to reset to the beginning of the
   * run.  You may need to redo the spinup.
   *****************************************************************/

  // Partitioning
  double_stringmap f_nppv,
      f_nppd;                 //!< fraction NPP into vegetation and detritus
  double_stringmap f_litterd; //!< fraction of litter to detritus

  // Initial fluxes
  fluxpool_stringmap npp_flux0; //!< preindustrial NPP

  // Variables needed to adjust NPP for LUC
  unitval cum_luc_va;
  tseries<unitval> cum_luc_va_ts;
  fluxpool end_of_spinup_vegc;
  double npp_luc_adjust;
  
  // Atmospheric CO2, temperature, and their effects
  fluxpool C0; //!< preindustrial [CO2], ppmv

  // Slowly-changing variables
  // These get computed only once per year, in slowparameval()
  fluxpool current_luc_e,  //!< Current year LUC emissions (/yr)
    current_luc_u,         //!< Current year LUC uptake (/yr)
    current_ffi_e,         //!< Current year FFI emissions (/yr)
    current_daccs_u;       //!< Current year DACCS uptake (/yr)
  double_stringmap beta;   //!< shape of CO2 response
  double_stringmap
      warmingfactor;       //!< regional warming relative to global (1.0=same)
  double_stringmap q10_rh; //!< Q10 for heterotrophic respiration (1.0=no response, unitless)

  /*****************************************************************
   * Functions computing sub-elements of the carbon cycle
   *****************************************************************/
  fluxpool
  CO2_conc(double time = Core::undefinedIndex()) const; //!< current [CO2], ppmv
  double calc_co2fert(std::string biome, double time = Core::undefinedIndex())
      const; //!< calculates co2 fertilization factor.
  fluxpool npp(std::string biome, double time = Core::undefinedIndex())
      const; //!< calculates NPP for a biome
  fluxpool sum_npp(double time = Core::undefinedIndex())
      const; //!< calculates NPP, global total
  fluxpool rh_fda(std::string biome, double time = Core::undefinedIndex())
      const; //!< calculates RH from detritus for a biome
  fluxpool rh_fsa(std::string biome, double time = Core::undefinedIndex())
      const; //!< calculates RH from soil for a biome
  fluxpool
  rh(std::string biome,
     double time = Core::undefinedIndex()) const; //!< calculates RH for a biome
  fluxpool sum_rh(double time = Core::undefinedIndex())
      const; //!< calculates RH, global total

  /*****************************************************************
   * Private helper functions
   *****************************************************************/
  fluxpool sum_map(fluxpool_stringmap pool)
      const; //!< sums a unitval map (collection of data)
  double sum_map(
      double_stringmap pool) const; //!< sums a double map (collection of data)
  void log_pools(const double t,
                 const string msg); //!< prints pool status to the log file
  void set_c0(double newc0); //!< set initial co2 and adjust total carbon mass
  fluxpool sum_fluxpool_biome_ts(const string varName, const double date,
                                 const string biome, fluxpool_stringmap pool,
                                 tvector<fluxpool_stringmap> pool_tv);
  bool has_biome(const std::string &biome);

  OceanComponent *omodel; //!< pointer to the ocean model in use

  // Add a biome to a time-series map variable (e.g. veg_c_tv)
  template <class T_data>
  void add_biome_to_ts(tvector<std::map<std::string, T_data>> &ts,
                       const std::string &biome, T_data init_value) {
    // First, check if a biome of this name already exists in the data
    if (ts.get(ts.firstdate()).count(biome)) {
      H_THROW("Biome '" + biome + "' already exists in data.");
    }

    // Loop over time steps, and set the variable to the provided `init_value`
    std::map<std::string, T_data> newval;
    for (double i = ts.firstdate(); i < ts.lastdate(); i++) {
      if (ts.exists(i)) {
        newval = ts.get(i);
        newval[biome] = init_value;
        ts.set(i, newval);
      }
    }
  }

  // Remove a biome from a time-series map variable
  template <class T_map>
  void remove_biome_from_ts(tvector<T_map> &ts, const std::string &biome) {
    // We don't need to check for presence of `biome` here because the
    // `<std::map>.erase()` method is effectively a no-op when given a
    // non-existent key.
    T_map currval;
    for (double i = ts.firstdate(); i < ts.lastdate(); i++) {
      if (ts.exists(i)) {
        currval = ts.get(i);
        currval.erase(biome);
        ts.set(i, currval);
      }
    }
  }

  // Rename a biome in a time-series map variable. At each time
  // step, this creates a new biome called `newname`, gives it all
  // of the values of `oldname`, and then erases that time step's
  // `oldname`.
  template <class T_map>
  void rename_biome_in_ts(tvector<T_map> &ts, const std::string &oldname,
                          const std::string &newname) {
    if (!ts.get(ts.firstdate()).count(oldname)) {
      H_THROW("Biome '" + oldname + "' not found in data.");
    }
    if (ts.get(ts.firstdate()).count(newname)) {
      H_THROW("Biome '" + newname + "' already exists in data.");
    }

    T_map currval;
    for (double i = ts.firstdate(); i < ts.lastdate(); i++) {
      if (ts.exists(i)) {
        currval = ts.get(i);
        currval[newname] = currval.at(oldname);
        currval.erase(oldname);
        ts.set(i, currval);
      }
    }
  }

  /*****************************************************************
   * Tracking Helper Functions
   *****************************************************************/
  void start_tracking() {
    earth_c.tracking = true;
    atmos_c.tracking = true;
    for (auto biome : biome_list) {
      veg_c[biome].tracking = true;
      soil_c[biome].tracking = true;
      detritus_c[biome].tracking = true;
    }
  }
};

} // namespace Hector

#endif
