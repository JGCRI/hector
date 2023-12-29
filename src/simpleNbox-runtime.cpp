/* Hector -- A Simple Climate Model
   Copyright (C) 2022  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  simpleNbox-runtime.cpp
 *  The old simpleNbox.cpp file was getting very long; this file now holds the
 * functions focusing on the model runtime: prepareToRun() and run(),
 * solver-related funcs such as calcDerivs() and StashCValues(), etc. hector
 *
 *  Created by Ben on 2020-02-06.
 *
 */

// some boost headers generate warnings under clang; not our problem, ignore
// 2023 and Boost 1.81.0_1: string.hpp still generates two warnings
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "boost/algorithm/string.hpp"
#pragma clang diagnostic pop

#include "avisitor.hpp"
#include "dependency_finder.hpp"
#include "simpleNbox.hpp"

#include <algorithm>

namespace Hector {

using namespace boost;

//------------------------------------------------------------------------------
/*! \brief      Log pool states
 *  \param      t date
 *  \param      msg message
 */
void SimpleNbox::log_pools(const double t, const string msg) {
  // Log pool states
  H_LOG(logger, Logger::DEBUG)
      << "---- pool states at t=" << t << " " << msg << " ----" << std::endl;
  H_LOG(logger, Logger::DEBUG) << "Atmos = " << atmos_c << std::endl;
  H_LOG(logger, Logger::DEBUG)
      << "Biome \tveg_c \t\tdetritus_c \tsoil_c \tpermafrost_c "
         "\tthawed_permafrost_c \tstatic_c"
      << std::endl;
  for (auto biome : biome_list) {
    H_LOG(logger, Logger::DEBUG)
        << biome << "\t" << veg_c[biome].value(U_PGC) << "\t\t"
        << detritus_c[biome].value(U_PGC) << "\t\t"
        << soil_c[biome].value(U_PGC) << "\t\t"
        << permafrost_c[biome].value(U_PGC) << "\t\t"
        << thawed_permafrost_c[biome].value(U_PGC) << "\t\t" << std::endl;
  }
  H_LOG(logger, Logger::DEBUG) << "Earth = " << earth_c << std::endl;
}

//------------------------------------------------------------------------------
// documentation is inherited
void SimpleNbox::prepareToRun() {
  H_LOG(logger, Logger::DEBUG) << "prepareToRun " << std::endl;

  // If any 'global' settings, there shouldn't also be regional
  if ((has_biome(SNBOX_DEFAULT_BIOME)) & (biome_list.size() > 1)) {
    H_THROW("Cannot have both global and biome-specific data! "
            "Did you forget to rename the default ('global') biome?")
  }

  // Ensure consistency between biome_list and all pools and fluxes
  H_ASSERT(biome_list.size() == veg_c.size(),
           "veg_c and biome_list data not same size");
  H_ASSERT(biome_list.size() == detritus_c.size(),
           "detritus_c and biome_list not same size");
  H_ASSERT(biome_list.size() == soil_c.size(),
           "soil_c and biome_list not same size");
  H_ASSERT(biome_list.size() == permafrost_c.size(),
           "permafrost_c and biome_list not same size");
  H_ASSERT(biome_list.size() == npp_flux0.size(),
           "npp_flux0 and biome_list not same size");

  // Set end-of-spinup vegc (in case no spinup requested)
  end_of_spinup_vegc = sum_map(veg_c);

  for (auto biome : biome_list) {
    H_LOG(logger, Logger::DEBUG) << "Checking that data for biome '" << biome
                                 << "' is complete" << std::endl;
    // Carbon pools
    H_ASSERT(detritus_c.count(biome), "no detritus_c data for " + biome);
    H_ASSERT(soil_c.count(biome), "no soil_c data for " + biome);
    H_ASSERT(permafrost_c.count(biome), "no biome data for permafrost_c");
    H_ASSERT(npp_flux0.count(biome), "no npp_flux0 data for " + biome);
    // Beta and Q10
    H_ASSERT(beta.count(biome), "No beta entry for " + biome);
    H_ASSERT(beta.at(biome) >= 0.0, "beta < 0");
    H_ASSERT(q10_rh.count(biome), "No Q10 entry for " + biome);
    H_ASSERT(q10_rh.at(biome) > 0.0, "q10_rh <= 0.0");
    // Partitioning
    H_ASSERT(f_nppv.count(biome), "No f_nppv entry for " + biome);
    H_ASSERT(f_nppv.at(biome) >= 0.0, "f_nppv <0");
    H_ASSERT(f_nppd.count(biome), "No f_nppd entry for " + biome);
    H_ASSERT(f_nppd.at(biome) >= 0.0, "f_nppd <0");
    H_ASSERT(f_nppv.count(biome), "No f_nppv entry for " + biome);
    H_ASSERT(f_nppv.at(biome) + f_nppd.at(biome) <= 1.0, "f_nppv + f_nppd >1");
    H_ASSERT(f_litterd.count(biome), "No f_litterd entry for " + biome);
    H_ASSERT(f_litterd.at(biome) >= 0.0 && f_litterd.at(biome) <= 1.0,
             "f_litterd <0 or >1");
    // Warming factor
    if (!warmingfactor.count(biome)) {
      H_LOG(logger, Logger::NOTICE)
          << "No warmingfactor set for biome '" << biome << "'. "
          << "Setting to default value = 1.0" << std::endl;
      warmingfactor[biome] = 1.0;
    }

    if (!rh_ch4_frac.count(biome)) {
      H_LOG(logger, Logger::NOTICE)
          << "No RH CH4 fraction set for biome '" << biome << "'. "
          << "Setting to default value = 0.023" << std::endl;
      rh_ch4_frac[biome] = 0.023;
    }

    if (!pf_mu.count(biome)) {
      H_LOG(logger, Logger::NOTICE)
          << "No permafrost mu parameter set for biome '" << biome << "'. "
          << "Setting to default value = 1.67" << std::endl;
      pf_mu[biome] = 1.67;
    }

    if (!pf_sigma.count(biome)) {
      H_LOG(logger, Logger::NOTICE)
          << "No permafrost sigma parameter set for biome '" << biome << "'. "
          << "Setting to default value = 0.986" << std::endl;
      pf_sigma[biome] = 0.986;
    }

    if (!fpf_static.count(biome)) {
      H_LOG(logger, Logger::NOTICE)
          << "No thawed permafrost static fraction for biome '" << biome
          << "'. "
          << "Setting to default value = 0.74" << std::endl;
      fpf_static[biome] = 0.74;
    }

    // Thawed permafrost C starts at zero
    thawed_permafrost_c[biome].set(0.0, U_PGC, permafrost_c[biome].tracking,
                                   D_THAWEDPC);
  }
  // Lognormal distribution for current biome's mu and sigma
  // We precompute these (one for each biome) since they don't change over time
  // This is equation 10 in Woodard et al. 2021
  // https://doi.org/10.5194/gmd-14-4751-2021
  for (auto biome : biome_list) {
    boost::math::lognormal s(pf_mu.at(biome), pf_sigma.at(biome));
    pf_s[biome] = s;
  }

  // A flag that lets run() know the very first time it's called
  has_been_run_before = false;

  // If no albedo data, assume constant
  if (!Falbedo.size()) {
    unitval alb(-0.2, U_W_M2); // default is MAGICC value
    Falbedo.set(core->getStartDate(), alb);
    Falbedo.set(core->getEndDate(), alb);
  }

  // Set atmospheric C based on the requested preindustrial [CO2]
  atmos_c.set(C0.value(U_PPMV_CO2) * PPMVCO2_TO_PGC, U_PGC, atmos_c.tracking,
              atmos_c.name);
  atmos_c_ts.set(core->getStartDate(), atmos_c);

  // Constraint logging
  if (CO2_constrain.size()) {
    Logger &glog = core->getGlobalLogger();
    H_LOG(glog, Logger::WARNING)
        << "Atmospheric CO2 will be constrained to user-supplied values!"
        << std::endl;
  }
  if (NBP_constrain.size()) {
    Logger &glog = core->getGlobalLogger();
    H_LOG(glog, Logger::WARNING) << "NBP (land-atmosphere C exchange) will be "
                                    "constrained to user-supplied values!"
                                 << std::endl;
  }

  // Save a pointer to the ocean model in use
  omodel =
      dynamic_cast<OceanComponent *>(core->getComponentByCapability(D_OCEAN_C));
  // TODO: this is a hack, because currently we can't pass fluxpools around via
  // sendMessage
  omodel->set_atmosphere_sources(
      atmos_c); // inform ocean model what our atmosphere looks like
}

//------------------------------------------------------------------------------
/*! \brief                  Run code, called from core
 *  \param[in] runToDate    Date to which to run to, double
 *
 *  This run method doesn't do much, because it's the carbon-cycle-solver
 *  run that does all the work.
 */
void SimpleNbox::run(const double runToDate) {
  in_spinup = core->inSpinup();

  if (!has_been_run_before) {
    // Remember starting value of veg_c; used later in NPP adjustment for LUC
    end_of_spinup_vegc = sum_map(veg_c);
    has_been_run_before = true;
  }

  // If we've hit the tracking start year, enagage!
  const double tdate = core->getTrackingDate();
  if (!in_spinup && runToDate == tdate) {
    H_LOG(logger, Logger::NOTICE) << "Tracking start" << std::endl;
    start_tracking();
  }
  Tland_record.set(runToDate, core->sendMessage(M_GETDATA, D_LAND_TAS));

  // TODO: this is a hack, because currently we can't pass fluxpools around via
  // sendMessage
  omodel->set_atmosphere_sources(
      atmos_c); // inform ocean model what our atmosphere looks like
}

//------------------------------------------------------------------------------
/*! \brief                  Spinup run code, called from core
 *  \param[in] step         Spinup step number
 *
 *  This run_spinup method doesn't do much, because it's the carbon-cycle-solver
 *  run that does all the work.
 */
bool SimpleNbox::run_spinup(const int step) {
  in_spinup = true;

  return true; // solver will really be the one signalling
}

//------------------------------------------------------------------------------
/*! \brief            transfer model pools to flat array (for ODE solver)
 *  \param[in] t  time, double, the date from which ODE solver is starting
 *  \param[in] c  flat array of carbon pools (no units)
 */
void SimpleNbox::getCValues(double t, double c[]) {
  c[SNBOX_ATMOS] = atmos_c.value(U_PGC);
  c[SNBOX_VEG] = sum_map(veg_c).value(U_PGC);
  c[SNBOX_DET] = sum_map(detritus_c).value(U_PGC);
  c[SNBOX_SOIL] = sum_map(soil_c).value(U_PGC);
  c[SNBOX_PERMAFROST] = sum_map(permafrost_c).value(U_PGC);
  c[SNBOX_THAWEDP] = sum_map(thawed_permafrost_c).value(U_PGC);
  omodel->getCValues(t, c);
  c[SNBOX_EARTH] = earth_c.value(U_PGC);

  ODEstartdate = t;
}

//------------------------------------------------------------------------------
/*! \brief                  Transfer new model pools from ODE solver array back
 * to model pools \param[in] t            Time, double, the ending date of the
 * solver \param[in] c       Flat array of carbon pools (no units) \exception
 * h_exception  If ocean model diverges from our pool tracking all ocean C
 *  \exception h_exception  If mass is not conserved
 *
 *  \details Transfer solver pools (no units) back to our pools (with units),
 * and run a sanity check to make sure mass has been conserved.
 */
void SimpleNbox::stashCValues(double t, const double c[]) {
  // Solver has gone from ODEstartdate to t
  // Note this is NOT guaranteed to be a full year jump! So compute the fraction
  // of the year we have advanced for use below
  const double yf = (t - ODEstartdate);
  H_ASSERT(yf >= 0 && yf <= 1, "yearfraction out of bounds");

  H_LOG(logger, Logger::DEBUG)
      << "Stashing at t=" << t << ", solver pools at " << t << ": "
      << "  atm = " << c[SNBOX_ATMOS] << "  veg = " << c[SNBOX_VEG]
      << "  det = " << c[SNBOX_DET] << "  soil = " << c[SNBOX_SOIL]
      << "  permafrost = " << c[SNBOX_PERMAFROST]
      << "  thawed_p = " << c[SNBOX_THAWEDP] << "  ocean = " << c[SNBOX_OCEAN]
      << "  earth = " << c[SNBOX_EARTH] << std::endl;
  log_pools(t, "BEFORE update");

  // IMPORTANT NOTE ABOUT YEAR FRACTION
  // In general we keep fluxes as per year throughout stashCValues
  // This changes only when we start to apportion NPP and RH to biomes

  // get the UNTRACKED earth emissions (ffi) and uptake (ccs)
  fluxpool ffi_untracked = current_ffi_e;
  fluxpool ccs_untracked = current_daccs_u;
  // now construct the TRACKED versions
  // because earth_c is tracked, ffi_flux and ccs_flux automatically
  // become tracked as well
  fluxpool ffi_flux = earth_c.flux_from_fluxpool(ffi_untracked);
  fluxpool ccs_flux = atmos_c.flux_from_fluxpool(ccs_untracked);

  // current ocean fluxes
  omodel->stashCValues(t, c); // tell ocean model to store new C values (and
                              // compute final surface fluxes)
  // ...and now get those fluxes (and their source maps, if tracking)
  fluxpool oa_flux = omodel->get_oaflux();
  fluxpool ao_flux = omodel->get_aoflux();

  // Land-use change emissions and uptake
  fluxpool luc_e_untracked = current_luc_e;
  fluxpool luc_u_untracked = current_luc_u;

  // Calculate net primary production and heterotrophic respiration
  fluxpool npp_total = sum_npp();
  fluxpool rh_total = sum_rh();

  // Permafrost
  const fluxpool permafrost_total = sum_map(permafrost_c);
  const fluxpool thawedp_total = sum_map(thawed_permafrost_c);

  // Calculate NBP *before* any constraint adjustment
  double alf = npp_total.value(U_PGC_YR) - rh_total.value(U_PGC_YR) -
               luc_e_untracked.value(U_PGC_YR) +
               luc_u_untracked.value(U_PGC_YR);

  // Note: we calculate total NPP and RH and *don't* adjust it if there's an NBP
  // constraint, because it's used for weighting with the npp(biome) and
  // rh(biome) calls below. So we want it to keep its original total for proper
  // weighting
  fluxpool npp_rh_total = npp_total + rh_total; // these are both positive

  // Pre-NBP constraint new terrestrial pool values
  fluxpool newatmos(c[SNBOX_ATMOS], U_PGC);
  fluxpool newveg(c[SNBOX_VEG], U_PGC);
  fluxpool newdet(c[SNBOX_DET], U_PGC);
  fluxpool newsoil(c[SNBOX_SOIL], U_PGC);
  fluxpool newpermafrost(c[SNBOX_PERMAFROST], U_PGC);
  // The solver can give very small negative values for thawed permafrost.
  // If this happens, round to zero so as not to throw a fluxpool error
  double solver_tpf = c[SNBOX_THAWEDP];
  if (abs(solver_tpf) < 1e-10) {
    solver_tpf = 0.0;
  }
  fluxpool newthawedpf(solver_tpf, U_PGC);

  // If there an NBP constraint? If yes, at this point adjust npp_total,
  // rh_total, and the newveg/newdet/newsoil/newthawedpf variables
  double rh_nbp_constraint_adjust = 1.0;
  const int rounded_t = round(t);
  if (!core->inSpinup() && NBP_constrain.size() &&
      NBP_constrain.exists(rounded_t)) {
    const unitval nbp_constrained = NBP_constrain.get(rounded_t);
    const unitval diff = (nbp_constrained - unitval(alf, U_PGC_YR));

    // Adjust fluxes equally
    npp_total = npp_total + diff / 2.0;
    rh_nbp_constraint_adjust = (rh_total - diff / 2.0) / rh_total;
    rh_total = rh_total - diff / 2.0;

    // Adjust pools
    // NOTE we only adjust for whatever year fraction we're currently stashing
    unitval pool_diff = unitval(diff.value(U_PGC_YR), U_PGC) * yf;
    const double total_land =
        c[SNBOX_DET] + c[SNBOX_VEG] + c[SNBOX_SOIL] + c[SNBOX_THAWEDP];
    newdet = newdet + pool_diff * c[SNBOX_DET] / total_land;
    newveg = newveg + pool_diff * c[SNBOX_VEG] / total_land;
    newsoil = newsoil + pool_diff * c[SNBOX_SOIL] / total_land;
    newthawedpf = newthawedpf + pool_diff * c[SNBOX_THAWEDP] / total_land;

    // We do NOT adjust the `newatmos` variable, because doing so can put the
    // model into an atmos_C feedback; see
    // https://github.com/JGCRI/hector/issues/659 Instead, follow the CO2
    // constraint behavior and transfer any difference to the deep ocean
    H_LOG(logger, Logger::DEBUG) << "Sending NBP_constrain residual of "
                                 << pool_diff << " to deep ocean" << std::endl;
    core->sendMessage(M_DUMP_TO_DEEP_OCEAN, D_OCEAN_C,
                      message_data(-pool_diff));

    // Re-calculate atmosphere-land flux (NBP)
    alf = npp_total.value(U_PGC_YR) - rh_total.value(U_PGC_YR) -
          luc_e_untracked.value(U_PGC_YR) + luc_u_untracked.value(U_PGC_YR);
    H_LOG(logger, Logger::NOTICE)
        << "** NBP constraint " << nbp_constrained
        << " requested; final value was " << alf << " with final adjustment of "
        << diff << std::endl;
  }

  nbp.set(alf, U_PGC_YR);
  nbp_ts.set(t, nbp);

  // Track (as a unitval) the cumulative vegetation-derived LUC flux
  const double total = c[SNBOX_VEG] + c[SNBOX_DET] + c[SNBOX_SOIL];
  const double luc_e = luc_e_untracked.value(U_PGC_YR);
  const double luc_u = luc_u_untracked.value(U_PGC_YR);
  cum_luc_va =
      cum_luc_va + unitval((luc_e - luc_u) * c[SNBOX_VEG] / total, U_PGC);

  // Apportion NPP and RH among the biomes
  // This is done by NPP and RH; biomes with higher values get more of any C
  // change

  for (auto biome : biome_list) {
    // `wt` is the biome share of major C fluxes; used for apportionment below
    const double wt = (npp(biome) + rh(biome)) / npp_rh_total;
    // Permafrost is weighted not by NPP+RH but by the pool sizes
    const double wt_pf =
        permafrost_total > 0 ? permafrost_c.at(biome) / permafrost_total : 0;
    H_LOG(logger, Logger::DEBUG) << "Biome " << biome << " wt = " << wt
                                 << "wt_pf = " << wt_pf << std::endl;

    // Calculate luc emissons
    const double veg_frac = veg_c[biome].value(U_PGC) / total;
    const double det_frac = detritus_c[biome].value(U_PGC) / total;
    const double soil_frac = soil_c[biome].value(U_PGC) / total;
    // Note we don't need to include 'wt' here because the veg_frac, det_frac,
    // and soil_frac fractions calculated above handle that
    fluxpool luc_fva_biome_flux =
        yf * veg_c[biome].flux_from_fluxpool(luc_e_untracked * veg_frac);
    fluxpool luc_fda_biome_flux =
        yf * detritus_c[biome].flux_from_fluxpool(luc_e_untracked * det_frac);
    fluxpool luc_fsa_biome_flux =
        yf * soil_c[biome].flux_from_fluxpool(luc_e_untracked * soil_frac);
    // Calculate luc uptake; it all goes to vegetation
    fluxpool luc_fav_biome_flux =
        yf * atmos_c.flux_from_fluxpool(luc_u_untracked);

    // Calculate NPP fluxes
    fluxpool npp_biome =
        npp_total * wt; // this is already adjusted for any NBP constraint
    final_npp[biome] = npp_biome;
    // Note that the following fluxes are weighted by 'yf' (year fraction)
    fluxpool npp_fav_biome_flux =
        yf * atmos_c.flux_from_fluxpool(npp_biome * f_nppv.at(biome));
    fluxpool npp_fad_biome_flux =
        yf * atmos_c.flux_from_fluxpool(npp_biome * f_nppd.at(biome));
    fluxpool npp_fas_biome_flux =
        yf * atmos_c.flux_from_fluxpool(
                 npp_biome * (1 - f_nppv.at(biome) - f_nppd.at(biome)));

    // Calculate and record the final RH values adjusted for any NBP constraint
    fluxpool rh_fda_adj = rh_fda(biome) * rh_nbp_constraint_adjust;
    fluxpool rh_fsa_adj = rh_fsa(biome) * rh_nbp_constraint_adjust;
    fluxpool rh_ftpa_co2_adj = rh_ftpa_co2(biome) * rh_nbp_constraint_adjust;
    fluxpool rh_ftpa_ch4_adj = rh_ftpa_ch4(biome) * rh_nbp_constraint_adjust;

    final_rh[biome] =
        rh_fda_adj + rh_fsa_adj + rh_ftpa_co2_adj + rh_ftpa_ch4_adj; // per year
    // Note that the following fluxes are weighted by 'yf' (year fraction)
    fluxpool rh_fda_flux =
        yf * detritus_c[biome].flux_from_fluxpool(rh_fda_adj);
    fluxpool rh_fsa_flux = yf * soil_c[biome].flux_from_fluxpool(rh_fsa_adj);
    fluxpool rh_fpa_co2_flux =
        yf * thawed_permafrost_c[biome].flux_from_fluxpool(rh_ftpa_co2_adj);
    fluxpool rh_fpa_ch4_flux =
        yf * thawed_permafrost_c[biome].flux_from_fluxpool(rh_ftpa_ch4_adj);
    RH_ch4[biome] = rh_fpa_ch4_flux;

    // Update soil, detritus, and atmosphere pools - luc fluxes
    atmos_c = atmos_c + luc_fva_biome_flux - luc_fav_biome_flux +
              luc_fda_biome_flux + luc_fsa_biome_flux;
    veg_c[biome] = veg_c[biome] + luc_fav_biome_flux - luc_fva_biome_flux;
    detritus_c[biome] - luc_fda_biome_flux;
    soil_c[biome] = soil_c[biome] - luc_fsa_biome_flux;

    // Update soil, detritus, and atmosphere pools - npp fluxes
    veg_c[biome] = veg_c[biome] + npp_fav_biome_flux;
    detritus_c[biome] = detritus_c[biome] + npp_fad_biome_flux;
    soil_c[biome] = soil_c[biome] + npp_fas_biome_flux;
    atmos_c =
        atmos_c - npp_fav_biome_flux - npp_fad_biome_flux - npp_fas_biome_flux;

    // Update soil, detritus, and atmosphere pools - rh fluxes
    atmos_c =
        atmos_c + rh_fda_flux + rh_fsa_flux + rh_fpa_co2_flux + rh_fpa_ch4_flux;
    detritus_c[biome] = detritus_c[biome] - rh_fda_flux;
    soil_c[biome] = soil_c[biome] - rh_fsa_flux;
    thawed_permafrost_c[biome] =
        thawed_permafrost_c[biome] - rh_fpa_co2_flux - rh_fpa_ch4_flux;

    // Permafrost thaw and refreeze
    if (!in_spinup) {
      // We pass in the annual fluxes here, because want annual thaw and
      // refreeze
      auto [x, y, z] =
          compute_pf_thaw_refreeze(biome, rh_ftpa_co2_adj, rh_ftpa_ch4_adj);
      // Construct fluxes...
      fluxpool pf_thaw =
          yf * permafrost_c[biome].flux_from_fluxpool(fluxpool(x, U_PGC_YR));
      fluxpool pf_refreeze_tp =
          yf *
          thawed_permafrost_c[biome].flux_from_fluxpool(fluxpool(y, U_PGC_YR));
      fluxpool pf_refreeze_soil =
          yf * soil_c[biome].flux_from_fluxpool(fluxpool(z, U_PGC_YR));
      // ...and update pools
      permafrost_c[biome] =
          permafrost_c[biome] - pf_thaw + pf_refreeze_tp + pf_refreeze_soil;
      thawed_permafrost_c[biome] =
          thawed_permafrost_c[biome] + pf_thaw - pf_refreeze_tp;
      soil_c[biome] = soil_c[biome] - pf_refreeze_soil;
    }

    // Update litter from veg to soil and detritus
    fluxpool litter_flux = veg_c[biome] * (0.035 * yf);
    fluxpool litter_fvd_flux = litter_flux * f_litterd.at(biome);
    fluxpool litter_fvs_flux = litter_flux * (1 - f_litterd.at(biome));
    detritus_c[biome] = detritus_c[biome] + litter_fvd_flux;
    soil_c[biome] = soil_c[biome] + litter_fvs_flux;
    veg_c[biome] = veg_c[biome] - litter_flux;

    // Update detritus and soil with detsoil flux
    fluxpool detsoil_flux = detritus_c[biome] * (0.6 * yf);
    soil_c[biome] = soil_c[biome] + detsoil_flux;
    // Detritus is a small pool that turns over very quickly (i.e. has large
    // fluxes in and out). As a result calculating it this way produces lots of
    // instability. Luckily we have the solver's final value to adjust to,
    // below; what we really want is to pass the carbon-tracking information
    // around if it's being used.
    detritus_c[biome] = detritus_c[biome] - detsoil_flux;

    // Adjust biome pools to final values from calcDerivs
    veg_c[biome].adjust_pool_to_val(newveg.value(U_PGC) * wt, false);
    detritus_c[biome].adjust_pool_to_val(newdet.value(U_PGC) * wt, false);
    soil_c[biome].adjust_pool_to_val(newsoil.value(U_PGC) * wt, false);
    permafrost_c[biome].adjust_pool_to_val(newpermafrost.value(U_PGC) * wt_pf,
                                           false);
    thawed_permafrost_c[biome].adjust_pool_to_val(newthawedpf.value(U_PGC) * wt_pf,
                                                  false);
  }

  // Update earth_c and atmos_c with fossil fuel and ocean fluxes
  earth_c = (earth_c - ffi_flux) + ccs_flux;
  atmos_c = (atmos_c + ffi_flux) - ccs_flux;
  atmos_c = atmos_c + oa_flux - ao_flux;

  // adjust non-biome pools to output from calcderivs (accounting for any NBP
  // constraint)
  earth_c.adjust_pool_to_val(c[SNBOX_EARTH], false);
  atmos_c.adjust_pool_to_val(newatmos.value(U_PGC), false);

  log_pools(t, "AFTER update");

  // Each time the model pools are updated, check that mass has been conserved
  double sum = 0.0;
  for (int i = 0; i < ncpool(); i++) {
    sum += c[i];
  }

  const double diff = fabs(sum - masstot);
  H_LOG(logger, Logger::DEBUG) << "masstot = " << masstot << ", sum = " << sum
                               << ", diff = " << diff << std::endl;
  if (masstot > 0.0 && diff > MB_EPSILON) {
    H_LOG(logger, Logger::SEVERE)
        << "Mass not conserved in " << getComponentName() << std::endl;
    H_LOG(logger, Logger::SEVERE)
        << "masstot = " << masstot << ", sum = " << sum << ", diff = " << diff
        << std::endl;
    H_THROW("Mass not conserved! (See log.)");
  }
  masstot = sum;

  // If user has supplied [CO2] values, adjust atmospheric C to match
  if (core->inSpinup() || (CO2_constrain.size() && CO2_constrain.exists(t))) {

    fluxpool atmos_cpool_to_match;
    fluxpool atmppmv;
    if (core->inSpinup()) {
      atmos_cpool_to_match.set(C0.value(U_PPMV_CO2) / PGC_TO_PPMVCO2, U_PGC);
      atmppmv.set(C0.value(U_PPMV_CO2), U_PPMV_CO2);
    } else {
      H_LOG(logger, Logger::NOTICE)
          << "** Constraining atmospheric CO2 to user-supplied value"
          << std::endl;
      atmos_cpool_to_match.set(
          CO2_constrain.get(t).value(U_PPMV_CO2) / PGC_TO_PPMVCO2, U_PGC);
      atmppmv.set(CO2_constrain.get(t).value(U_PPMV_CO2), U_PPMV_CO2);
    }

    // Ugly: residual is a unitval, but calculated by subtracting two fluxpools,
    // so extract value
    Ca_residual.set(atmos_c.value(U_PGC) - atmos_cpool_to_match.value(U_PGC),
                    U_PGC);

    H_LOG(logger, Logger::DEBUG) << t << "- have " << CO2_conc() << " want "
                                 << atmppmv.value(U_PPMV_CO2) << std::endl;
    H_LOG(logger, Logger::DEBUG)
        << t << "- have " << atmos_c << " want " << atmos_cpool_to_match
        << "; residual = " << Ca_residual << std::endl;

    // Transfer C from atmosphere to deep ocean and update our C and [CO2]
    // variables
    H_LOG(logger, Logger::DEBUG) << "Sending residual of " << Ca_residual
                                 << " to deep ocean" << std::endl;
    core->sendMessage(M_DUMP_TO_DEEP_OCEAN, D_OCEAN_C,
                      message_data(Ca_residual));
    atmos_c = atmos_c - Ca_residual;
  } else {
    Ca_residual.set(0.0, U_PGC);
  }

  log_pools(t, "FINAL");

  // All good! t will be the start of the next timestep, so
  ODEstartdate = t;
}

// A series of small functions to calculate variables that will appear in the
// output stream

double SimpleNbox::calc_co2fert(std::string biome, double time) const {
  return 1 + beta.at(biome) * log(CO2_conc(time) / C0);
}

//------------------------------------------------------------------------------
/*! \brief      Compute annual net primary production
 *  \returns    current annual NPP
 */
fluxpool SimpleNbox::npp(std::string biome, double time) const {
  fluxpool npp(npp_flux0.at(biome).value(U_PGC_YR),
               U_PGC_YR); // 'at' throws exception if not found
  if (time == Core::undefinedIndex()) {
    npp = npp * co2fert.at(biome); // that's why used here instead of []
  } else {
    npp = npp * calc_co2fert(biome, time);
  }

  // LUC causes loss (or gains) to vegetation; account for this
  npp = npp * npp_luc_adjust;

  return npp;
}

//------------------------------------------------------------------------------
/*! \brief      Compute global net primary production
 *  \returns    Annual NPP summed across all biomes
 */
fluxpool SimpleNbox::sum_npp(double time) const {
  fluxpool total(0.0, U_PGC_YR);
  for (auto biome : biome_list) {
    total = total + npp(biome, time);
  }
  return total;
}

//------------------------------------------------------------------------------
/*! \brief      Compute detritus component of annual heterotrophic respiration
 *  \returns    current detritus component of annual heterotrophic respiration
 */
fluxpool SimpleNbox::rh_fda(std::string biome, double time) const {
  unitval det_t;
  double tfd;
  if (time == Core::undefinedIndex()) {
    det_t = detritus_c.at(biome);
    tfd = tempfertd.at(biome);
  } else {
    det_t = detritus_c_tv.get(time).at(biome);
    tfd = tempfertd_tv.get(time).at(biome);
  }
  fluxpool dflux(det_t.value(U_PGC) * 0.25, U_PGC_YR);
  return dflux * tfd;
}

//------------------------------------------------------------------------------
/*! \brief      Compute soil component of annual heterotrophic respiration
 *  \returns    current soil component of annual heterotrophic respiration
 */
fluxpool SimpleNbox::rh_fsa(std::string biome, double time) const {
  unitval soil_t;
  double tfs;
  if (time == Core::undefinedIndex()) {
    soil_t = soil_c.at(biome);
    tfs = tempferts.at(biome);
  } else {
    soil_t = soil_c_tv.get(time).at(biome);
    tfs = tempferts_tv.get(time).at(biome);
  }
  fluxpool soilflux(soil_t.value(U_PGC) * 0.02, U_PGC_YR);
  return soilflux * tfs;
}

//------------------------------------------------------------------------------
/*! \brief      Compute CO2 flux from thawed permafrost
 *  \returns    CO2 flux from thawed permafrost, Pg C/yr
 */
fluxpool SimpleNbox::rh_ftpa_co2(std::string biome, double time) const {
  double tfs;
  fluxpool tpfc;
  if (time == Core::undefinedIndex()) {
    tfs = tempferts.at(biome);
    tpfc = thawed_permafrost_c.at(biome) * fpf_static.at(biome);
  } else {
    tfs = tempferts_tv.get(time).at(biome);
    tpfc = thawed_permafrost_c_tv.get(time).at(biome) * fpf_static.at(biome);
  }
  fluxpool tpflux(tpfc.value(U_PGC) * 0.02, U_PGC_YR);
  return tpflux * tfs * (1.0 - rh_ch4_frac.at(biome));
}

//------------------------------------------------------------------------------
/*! \brief      Compute CH4 flux from thawed permafrost
 *  \returns    CH4 flux from thawed permafrost, Pg C/yr
 */
fluxpool SimpleNbox::rh_ftpa_ch4(std::string biome, double time) const {
  // Calculate the CO2 flux, then calculate CH4 component of total
  return rh_ftpa_co2(biome, time) / (1.0 - rh_ch4_frac.at(biome)) *
         rh_ch4_frac.at(biome);
}

//------------------------------------------------------------------------------
/*! \brief      Compute total annual heterotrophic respiration
 *  \returns    current annual heterotrophic respiration
 */
fluxpool SimpleNbox::rh(std::string biome, double time) const {
  // Heterotrophic respiration is the sum of CO2 fluxes from detritus, soil, and
  // thawed permafrost
  return rh_fda(biome, time) + rh_fsa(biome, time) + rh_ftpa_co2(biome, time);
}

//------------------------------------------------------------------------------
/*! \brief      Compute global heterotrophic respiration
 *  \returns    Annual RH summed across all biomes
 */
fluxpool SimpleNbox::sum_rh(double time) const {
  fluxpool total(0.0, U_PGC_YR);
  for (auto biome : biome_list) {
    total = total + rh(biome, time);
  }
  return total;
}

//------------------------------------------------------------------------------
/*! \brief      Compute permafrost thaw and refreeze fluxes
 *  \param biome Name of the biome, a string
 *  \param rh_co2 Flux of CO2-C from thawed permafrost
 *  \param rh_ch4 Flux of CH4-C from thawed permafrost
 *  \returns    A tuple of pf_thaw_c, pf_refreeze_tp, pf_refreeze_soil (Pg C,
 * but all doubles for speed) \note This logic follows Woodard et al. 2021
 * https://gmd.copernicus.org/articles/14/4751/2021/
 */
tuple<double, double, double>
SimpleNbox::compute_pf_thaw_refreeze(string biome, fluxpool rh_co2,
                                     fluxpool rh_ch4) const {

  H_ASSERT(!in_spinup, "We should not be here!");
  
  double biome_c_thaw =
      permafrost_c.at(biome).value(U_PGC) * new_thaw.at(biome);
  double pf_refreeze_tp = 0.0;
  double pf_refreeze_soil = 0.0;

  if (biome_c_thaw < 0) {
    // If the permafrost thaw is negative, that means refreezing.
    // This occurs preferentially from the thawed permafrost pool,
    // and secondarily from the soil pool
    const double pf_refreeze = -biome_c_thaw;
    biome_c_thaw = 0.0;
    const double thawed_remaining = thawed_permafrost_c.at(biome).value(U_PGC) -
                                    rh_co2.value(U_PGC_YR) -
                                    rh_ch4.value(U_PGC_YR);
    pf_refreeze_tp = std::min(pf_refreeze, thawed_remaining);
    pf_refreeze_soil = pf_refreeze - pf_refreeze_tp;
  }

  return {biome_c_thaw, pf_refreeze_tp, pf_refreeze_soil};
}

///------------------------------------------------------------------------------
/*! \brief              Compute model fluxes for a time step
 *  \param[in]  t       time
 *  \param[in]  c       carbon pools (no units)
 *  \param[out] dcdt    carbon fluxes
 *  \returns            code indicating success or failure
 */
int SimpleNbox::calcderivs(double t, const double c[], double dcdt[]) const {
  // Solver is attempting to go from ODEstartdate to t
  // Atmosphere-ocean flux is calculated by ocean_component
  const int omodel_err = omodel->calcderivs(t, c, dcdt);
  const double ao_exchange = dcdt[SNBOX_OCEAN];
  fluxpool ocean_uptake(0.0, U_PGC_YR);
  fluxpool ocean_release(0.0, U_PGC_YR);
  if (ao_exchange >= 0.0) {
    ocean_uptake.set(ao_exchange, U_PGC_YR);
  } else {
    ocean_release.set(-ao_exchange, U_PGC_YR);
  }

  // NPP: Net primary productivity
  fluxpool npp_biome(0.0, U_PGC_YR);
  fluxpool npp_current(0.0, U_PGC_YR);
  fluxpool npp_fav(0.0, U_PGC_YR);
  fluxpool npp_fad(0.0, U_PGC_YR);
  fluxpool npp_fas(0.0, U_PGC_YR);

  // RH: heterotrophic respiration from detritus and soil
  // Permafrost HR is handled below
  fluxpool rh_fda_current(0.0, U_PGC_YR);
  fluxpool rh_fsa_current(0.0, U_PGC_YR);
  // Heterotrophic respiration (CO2 and CH4) from thawed permafrost
  fluxpool rh_ftpa_co2_current(0.0, U_PGC_YR);
  fluxpool rh_ftpa_ch4_current(0.0, U_PGC_YR);

  for (auto biome : biome_list) {
    // NPP is scaled by CO2 from preindustrial value
    npp_biome = npp(biome);
    npp_current = npp_current + npp_biome;
    npp_fav = npp_fav + npp_biome * f_nppv.at(biome);
    npp_fad = npp_fad + npp_biome * f_nppd.at(biome);
    npp_fas = npp_fas + npp_biome * (1 - f_nppv.at(biome) - f_nppd.at(biome));
    rh_fda_current = rh_fda_current + rh_fda(biome);
    rh_fsa_current = rh_fsa_current + rh_fsa(biome);
    rh_ftpa_co2_current = rh_ftpa_co2_current + rh_ftpa_co2(biome);
    rh_ftpa_ch4_current = rh_ftpa_ch4_current + rh_ftpa_ch4(biome);
  }
  fluxpool rh_current = rh_fda_current + rh_fsa_current + rh_ftpa_co2_current;
  fluxpool rh_ch4_current = rh_ftpa_ch4_current;

  // Detritus flux comes from the vegetation pool
  fluxpool litter_flux(0.0, U_PGC_YR);
  fluxpool litter_fvd(0.0, U_PGC_YR);
  fluxpool litter_fvs(0.0, U_PGC_YR);
  for (auto biome : biome_list) {
    fluxpool v = fluxpool(veg_c.at(biome).value(U_PGC) * 0.035, U_PGC_YR);
    litter_flux = litter_flux + v;
    litter_fvd = litter_fvd + v * f_litterd.at(biome);
    litter_fvs = litter_fvs + v * (1 - f_litterd.at(biome));
  }

  // Some detritus goes to soil
  fluxpool detsoil_flux(0.0, U_PGC_YR);
  for (auto biome : biome_list) {
    detsoil_flux = detsoil_flux +
                   fluxpool(detritus_c.at(biome).value(U_PGC) * 0.6, U_PGC_YR);
  }

  // Land-use change emissions come from veg, detritus, and soil proportionately
  const double total = c[SNBOX_VEG] + c[SNBOX_DET] + c[SNBOX_SOIL];
  fluxpool luc_fva = current_luc_e * c[SNBOX_VEG] / total;
  fluxpool luc_fda = current_luc_e * c[SNBOX_DET] / total;
  fluxpool luc_fsa = current_luc_e * c[SNBOX_SOIL] / total;
  // ...whereas uptake goes entirely to vegetation
  fluxpool luc_fav = current_luc_u;

  // Oxidized methane of fossil fuel origin
  fluxpool ch4ox_current(0.0, U_PGC_YR); // TODO: implement this

  // As permafrost thaws, the C is mobilized into the thawed permafrost pool.
  fluxpool pf_thaw_c(0.0, U_PGC_YR);
  fluxpool pf_refreeze_tp(0.0, U_PGC_YR);
  fluxpool pf_refreeze_soil(0.0, U_PGC_YR);
  if (!in_spinup) { // No permafrost dynamics during spinup
    // Sum permafrost thaw and refreeze across all biomes
    for (auto biome : biome_list) {
      auto [biome_c_thaw, biome_pf_refreeze_tp, biome_pf_refreeze_soil] =
          compute_pf_thaw_refreeze(biome, rh_ftpa_co2(biome),
                                   rh_ftpa_ch4(biome));
      pf_thaw_c = pf_thaw_c + fluxpool(biome_c_thaw, U_PGC_YR);
      pf_refreeze_tp =
          pf_refreeze_tp + fluxpool(biome_pf_refreeze_tp, U_PGC_YR);
      pf_refreeze_soil =
          pf_refreeze_tp + fluxpool(biome_pf_refreeze_soil, U_PGC_YR);
    }
  }

  // If user has supplied NBP (net biome production) values,
  // adjust NPP and RH to match
  const int rounded_t = round(t);
  if (!in_spinup && NBP_constrain.size() && NBP_constrain.exists(rounded_t)) {
    // Compute how different we are from the user-specified constraint
    const double nbp =
        npp_current.value(U_PGC_YR) - rh_current.value(U_PGC_YR) -
        current_luc_e.value(U_PGC_YR) + current_luc_u.value(U_PGC_YR);
    const unitval diff = NBP_constrain.get(rounded_t) - unitval(nbp, U_PGC_YR);

    // Adjust total NPP and total RH equally (but not LUC, which is an input)
    // so that their net total will match the NBP constraint
    fluxpool npp_current_old = npp_current;
    npp_current = npp_current + diff / 2.0;
    // ...also need to adjust their sub-components
    const double npp_ratio = npp_current / npp_current_old;
    npp_fav = npp_fav * npp_ratio;
    npp_fad = npp_fad * npp_ratio;
    npp_fas = npp_fas * npp_ratio;

    // Do same thing for the RH sub-components
    fluxpool rh_current_old = rh_current;
    rh_current = rh_current - diff / 2.0;
    const double rh_ratio = rh_current / rh_current_old;
    rh_fda_current = rh_fda_current * rh_ratio;
    rh_fsa_current = rh_fsa_current * rh_ratio;
    rh_ftpa_co2_current = rh_ftpa_co2_current * rh_ratio;
  }

  // Compute fluxes
  dcdt[SNBOX_ATMOS] = // change in atmosphere pool
      current_ffi_e.value(U_PGC_YR) - current_daccs_u.value(U_PGC_YR) +
      current_luc_e.value(U_PGC_YR) - current_luc_u.value(U_PGC_YR) +
      ch4ox_current.value(U_PGC_YR) - ocean_uptake.value(U_PGC_YR) +
      ocean_release.value(U_PGC_YR) -
      npp_current.value(U_PGC_YR)
      // HACK: For mass balance purposes, dump both RH{CO2} and RH{CH4} into
      // the atmosphere. Effectively, this means that CH4 is emitted on top of
      // existing CO2 -- i.e., more CH4 emissions does not mean less CO2
      // emissions from RH
      + rh_ch4_current.value(U_PGC_YR) + rh_current.value(U_PGC_YR);
  dcdt[SNBOX_VEG] = // change in vegetation pool
      npp_fav.value(U_PGC_YR) - litter_flux.value(U_PGC_YR) -
      luc_fva.value(U_PGC_YR) + luc_fav.value(U_PGC_YR);
  dcdt[SNBOX_DET] = // change in detritus pool
      npp_fad.value(U_PGC_YR) + litter_fvd.value(U_PGC_YR) -
      detsoil_flux.value(U_PGC_YR) - rh_fda_current.value(U_PGC_YR) -
      luc_fda.value(U_PGC_YR);
  dcdt[SNBOX_SOIL] = // change in soil pool
      npp_fas.value(U_PGC_YR) + litter_fvs.value(U_PGC_YR) +
      detsoil_flux.value(U_PGC_YR) - rh_fsa_current.value(U_PGC_YR) -
      pf_refreeze_soil.value(U_PGC_YR) - luc_fsa.value(U_PGC_YR);
  dcdt[SNBOX_PERMAFROST] = // change in permafrost pool
      -pf_thaw_c.value(U_PGC_YR) + pf_refreeze_soil.value(U_PGC_YR) +
      pf_refreeze_tp.value(U_PGC_YR);
  dcdt[SNBOX_THAWEDP] = // change in thawed permafrost pool
      pf_thaw_c.value(U_PGC_YR) - pf_refreeze_tp.value(U_PGC_YR) -
      rh_ftpa_ch4_current.value(U_PGC_YR) - rh_ftpa_co2_current.value(U_PGC_YR);
  dcdt[SNBOX_OCEAN] = // change in ocean pool
      ocean_uptake.value(U_PGC_YR) - ocean_release.value(U_PGC_YR);
  dcdt[SNBOX_EARTH] = // change in earth pool
      -current_ffi_e.value(U_PGC_YR) + current_daccs_u.value(U_PGC_YR);

  return omodel_err;
}

//------------------------------------------------------------------------------
/*! \brief              Compute 'slowly varying' fluxes
 *  \param[in]  t       time (at the *beginning* of the current time step.
 *  \param[in]  c       carbon pools (no units)
 *
 *  Compute 'slowly varying' fertilization and anthropogenic fluxes.
 *  Treat the fertilization factors as slowly varying for illustrative purposes
 *  (in fact we could calculate it at each integration step if we wanted to).
 */
void SimpleNbox::slowparameval(double t, const double c[]) {
  omodel->slowparameval(t, c); // pass msg on to ocean model

  // Set this year's LUC and FFI/DACCS emissions and uptake
  // We do this here, and not allow interpolation of their time series,
  // so that pulse tests work correctly (see #643)
  fluxpool zero_flux(0.0, U_PGC_YR);
  current_luc_e = in_spinup ? zero_flux : lucEmissions.get(t);
  current_luc_u = in_spinup ? zero_flux : lucUptake.get(t);
  current_ffi_e = in_spinup ? zero_flux : ffiEmissions.get(t);
  current_daccs_u = in_spinup ? zero_flux : daccsUptake.get(t);

  // Compute loss (or gain) of vegetation to LUC
  npp_luc_adjust = (end_of_spinup_vegc - cum_luc_va) / end_of_spinup_vegc;
  H_LOG(logger, Logger::DEBUG)
      << "slowparameval: npp_luc_adjust = " << npp_luc_adjust << std::endl;
  //  cout << "xxx," << t << "," << cum_luc_va << "," << end_of_spinup_vegc <<
  //  "," << npp_luc_adjust << endl;

  // Compute CO2 fertilization factor globally (and for each biome specified)
  for (auto biome : biome_list) {
    if (in_spinup) {
      co2fert[biome] = 1.0; // no perturbation allowed if in spinup
    } else {
      co2fert[biome] = calc_co2fert(biome);
    }
    H_LOG(logger, Logger::DEBUG)
        << "co2fert[ " << biome << " ] at " << CO2_conc() << " = "
        << co2fert.at(biome) << std::endl;
  }

  // Compute temperature factor globally (and for each biome specified)
  // Heterotrophic respiration depends on the pool sizes (detritus and soil) and
  // Q10 values The soil pool uses a lagged land air/surface temperature, i.e.
  // we assume it takes time for heat to diffuse into soil
  const double Tland = core->sendMessage(M_GETDATA, D_LAND_TAS);

  /* set tempferts (soil) and tempfertd (detritus) for each biome */

  // Need the previous time step values of tempferts.  Since t is
  // the time at the beginning of the current time step (== the end
  // of the previous time step), we can use t as the index to look
  // up the previous value.
  double_stringmap
      tfs_last; // Previous time step values of tempferts; initialized empty
  if (t != Core::undefinedIndex() && t > core->getStartDate()) {
    tfs_last = tempferts_tv[t];
  }

  // Loop over biomes
  for (auto biome : biome_list) {
    if (in_spinup) {
      tempfertd[biome] = 1.0; // no perturbation allowed in spinup
      tempferts[biome] = 1.0; // no perturbation allowed in spinup
      f_frozen[biome] = 1.0;  // no perturbation allowed in spinup
      new_thaw[biome] = 0.0;  // no perturbation allowed in spinup
    } else {
      double wf;
      if (warmingfactor.count(biome)) {
        wf = warmingfactor.at(biome); // biome-specific warming
      } else if (warmingfactor.count(SNBOX_DEFAULT_BIOME)) {
        wf = warmingfactor.at(SNBOX_DEFAULT_BIOME);
      } else {
        wf = 1.0;
      }

      const double Tland_biome = Tland * wf; // biome-specific temperature

      tempfertd[biome] = pow(q10_rh.at(biome),
                             (Tland_biome / 10.0)); // detritus warms with air

      // Permafrost thaw, as a fraction
      // Currently, these are calibrated to produce a 0.172 / year slope from
      // 0.8 to 4 degrees C, which was the linear form of this in Kessler 2017
      // https://doi.org/10.1142/s2010007817500087 (referenced in Woodard 2021).
      new_thaw[biome] = 0.0;
      if (permafrost_c[biome].value(U_PGC)) {
        // This uses the biome's lognormal distribution, based on its mu and
        // sigma, which is precomputed in prepareToRun(). Replicating the
        // behavior of R's plnorm(), we use a value of one (=exp(0)) if
        // Tland_biome <= 0
        double f_frozen_current = 1.0;
        if (Tland_biome > 0) {
          f_frozen_current = 1 - cdf(pf_s[biome], Tland_biome);
          H_LOG(logger, Logger::DEBUG)
              << "slowparameval: f_frozen_current = " << f_frozen_current
              << std::endl;
        }

        new_thaw[biome] = f_frozen[biome] - f_frozen_current;
        f_frozen[biome] = f_frozen_current;
      }

// Soil warm very slowly relative to the atmosphere
// We use a mean temperature of a window (size Q10_TEMPN) of temperatures to
// scale Q10
#define Q10_TEMPLAG 0 // 125         // TODO: put lag in input files 150, 25
#define Q10_TEMPN 200 // 25
      double Tland_rm = 0.0; /* window mean of Tland */
      if (t > core->getStartDate() + Q10_TEMPLAG) {
        for (int i = t - Q10_TEMPLAG - Q10_TEMPN; i < t - Q10_TEMPLAG; i++) {
          Tland_rm += Tland_record.get(i) * wf;
        }

        Tland_rm /= Q10_TEMPN;
      }

      tempferts[biome] = pow(q10_rh.at(biome), (Tland_rm / 10.0));

      // The soil Q10 effect is 'sticky' and can only increase, not decline
      double tempferts_last =
          tfs_last[biome]; // If tfs_last is empty, this will produce 0.0
      if (tempferts[biome] < tempferts_last) {
        tempferts[biome] = tempferts_last;
      }

      H_LOG(logger, Logger::DEBUG)
          << biome << " Tland=" << Tland << ", Tland_biome=" << Tland_biome
          << ", tempfertd=" << tempfertd[biome]
          << ", tempferts=" << tempferts[biome] << std::endl;
    }
  } // loop over biomes

  H_LOG(logger, Logger::DEBUG)
      << "slowparameval: would have recorded tempferts = "
      << tempferts[SNBOX_DEFAULT_BIOME] << " at time= " << tcurrent
      << std::endl;
}

} // namespace Hector
