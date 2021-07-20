/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  simpleNbox-runtime.cpp
 *  The old simpleNbox.cpp file was getting very long; this file now holds the functions
 *  focusing on the model runtime: prepareToRun() and run(), solver-related funcs such
 *  as calcDerivs() and StashCValues(), etc.
 *  hector
 *
 *  Created by Ben on 2020-02-06.
 *
 */

#include "boost/algorithm/string.hpp"

#include "dependency_finder.hpp"
#include "simpleNbox.hpp"
#include "avisitor.hpp"

#include <algorithm>

namespace Hector {

using namespace boost;

//------------------------------------------------------------------------------
/*! \brief      Sanity checks
 *  \exception  If any of the sanity checks fails
 *
 *  This is called internally throughout the model run and performs sanity checks.
 *  For example, the main carbon pools (except earth) should always be positive;
 *  partitioning coefficients should not exceed 1; etc.
 */
void SimpleNbox::sanitychecks()
{
    // A few sanity checks
    // Note that with the addition of the fluxpool class (which guarantees
    // non-negative numbers) many of these checks went away
    for ( auto it = biome_list.begin(); it != biome_list.end(); it++ ) {
        std::string biome = *it;
        H_ASSERT( f_nppv.at(biome) >= 0.0, "f_nppv <0" );
        H_ASSERT( f_nppd.at(biome) >= 0.0, "f_nppd <0" );
        H_ASSERT( f_nppv.at(biome) + f_nppd.at(biome) <= 1.0, "f_nppv + f_nppd >1" );
        H_ASSERT( f_litterd.at(biome) >= 0.0 && f_litterd.at(biome) <= 1.0, "f_litterd <0 or >1" );
    }

    H_ASSERT( f_lucv >= 0.0, "f_lucv <0" );
    H_ASSERT( f_lucd >= 0.0, "f_lucd <0" );
    H_ASSERT( f_lucv + f_lucd <= 1.0, "f_lucv + f_lucd >1" );
}

//------------------------------------------------------------------------------
/*! \brief      Log pool states
 *  \param      current date
 */
void SimpleNbox::log_pools( const double t )
{
    // Log pool states
    H_LOG( logger,Logger::DEBUG ) << "---- simpleNbox pool states at t=" << t << " ----" << std::endl;
    H_LOG( logger,Logger::DEBUG ) << "Atmos = " << atmos_c << std::endl;
    H_LOG( logger,Logger::DEBUG ) << "Biome \tveg_c \t\tdetritus_c \tsoil_c" << std::endl;
    for ( auto it = biome_list.begin(); it != biome_list.end(); it++ ) {
        std::string biome = *it;
        H_LOG( logger,Logger::DEBUG ) << biome << "\t" << veg_c[ biome ] << "\t" <<
        detritus_c[ biome ] << "\t\t" << soil_c[ biome ] << std::endl;
    }
    H_LOG( logger,Logger::DEBUG ) << "Earth = " << earth_c << std::endl;
}

//------------------------------------------------------------------------------
// documentation is inherited
void SimpleNbox::prepareToRun()
{
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;

    // If any 'global' settings, there shouldn't also be regional
    if ( (has_biome( SNBOX_DEFAULT_BIOME )) & (biome_list.size() > 1) ) {
        H_THROW( "Cannot have both global and biome-specific data! "
                 "Did you forget to rename the default ('global') biome?")
    }

    // Ensure consistency between biome_list and all pools and fluxes
    H_ASSERT( biome_list.size() == veg_c.size(), "veg_c and biome_list data not same size" );
    H_ASSERT( biome_list.size() == detritus_c.size(), "detritus_c and biome_list not same size" );
    H_ASSERT( biome_list.size() == soil_c.size(), "soil_c and biome_list not same size" );
    H_ASSERT( biome_list.size() == npp_flux0.size(), "npp_flux0 and biome_list not same size" );

    for ( auto it = biome_list.begin(); it != biome_list.end(); it++ ) {
        std::string biome = *it;
        H_LOG( logger, Logger::DEBUG ) << "Checking that data for biome '" << biome << "' is complete" << std::endl;
        H_ASSERT( detritus_c.count( biome ), "no biome data for detritus_c" );
        H_ASSERT( soil_c.count( biome ), "no biome data for soil_c" );
        H_ASSERT( npp_flux0.count( biome ), "no biome data for npp_flux0" );

        H_ASSERT( beta.count( biome ), "no biome value for beta" );

        if ( !warmingfactor.count( biome )) {
            H_LOG( logger, Logger::NOTICE ) << "No warmingfactor set for biome '" << biome << "'. " <<
                "Setting to default value = 1.0" << std::endl;
            warmingfactor[ biome ] = 1.0;
        }
    }

    // Save a pointer to the ocean model in use
    omodel = dynamic_cast<CarbonCycleModel*>( core->getComponentByCapability( D_OCEAN_C ) );

    if( !Ftalbedo.size() ) {          // if no albedo data, assume constant
        unitval alb( -0.2, U_W_M2 ); // default is MAGICC value
        Ftalbedo.set( core->getStartDate(), alb );
        Ftalbedo.set( core->getEndDate(), alb );
    }

    double c0init = C0.value(U_PPMV_CO2);
    Ca.set(c0init, U_PPMV_CO2);
    atmos_c.set(c0init * PPMVCO2_TO_PGC, U_PGC, atmos_c.tracking, atmos_c.name);

    if( CO2_constrain.size() ) {
        Logger& glog = core->getGlobalLogger();
        H_LOG( glog, Logger::WARNING ) << "Atmospheric CO2 will be constrained to user-supplied values!" << std::endl;
    }

    // One-time checks
    for( auto it = biome_list.begin(); it != biome_list.end(); it++ ) {
        H_ASSERT( beta.at( *it ) >= 0.0, "beta < 0" );
        H_ASSERT( q10_rh.at( *it )>0.0, "q10_rh <= 0.0" );
    }
    sanitychecks();
}

//------------------------------------------------------------------------------
/*! \brief                  Run code, called from core
 *  \param[in] runToDate    Date to which to run to, double
 *
 *  This run method doesn't do much, because it's the carbon-cycle-solver
 *  run that does all the work.
 */
void SimpleNbox::run( const double runToDate )
{
    in_spinup = core->inSpinup();
    sanitychecks();
    if(!in_spinup && tcurrent == trackingYear){
        startTracking();
    }
    Tgav_record.set( runToDate, core->sendMessage( M_GETDATA, D_GLOBAL_TEMP ).value( U_DEGC ) );
}

//------------------------------------------------------------------------------
/*! \brief                  Spinup run code, called from core
 *  \param[in] step         Spinup step number
 *
 *  This run_spinup method doesn't do much, because it's the carbon-cycle-solver
 *  run that does all the work.
 */
bool SimpleNbox::run_spinup( const int step )
{
    sanitychecks();
    in_spinup = true;
    return true;        // solver will really be the one signalling
}

//------------------------------------------------------------------------------
/*! \brief            transfer model pools to flat array (for ODE solver)
 *  \param[in] t  time, double, the date from which ODE solver is starting
 *  \param[in] c  flat array of carbon pools (no units)
 */
void SimpleNbox::getCValues( double t, double c[] )
{
    c[ SNBOX_ATMOS ] = atmos_c.value( U_PGC );
    c[ SNBOX_VEG ] = sum_map( veg_c ).value( U_PGC );
    c[ SNBOX_DET ] = sum_map( detritus_c ).value( U_PGC );
    c[ SNBOX_SOIL ] = sum_map( soil_c ).value( U_PGC );
    omodel->getCValues( t, c );
    c[ SNBOX_EARTH ] = earth_c.value( U_PGC );

    ODEstartdate = t;
}

//------------------------------------------------------------------------------
/*! \brief                  Transfer new model pools from ODE solver array back to model pools
 *  \param[in] t            Time, double, the ending date of the solver
 *  \param[in] double       Flat array of carbon pools (no units)
 *  \exception h_exception  If ocean model diverges from our pool tracking all ocean C
 *  \exception h_exception  If mass is not conserved
 *
 *  \details Transfer solver pools (no units) back to our pools (with units), and run
 *  a sanity check to make sure mass has been conserved.
 */
void SimpleNbox::stashCValues( double t, const double c[] )
{
    // Solver has gone from ODEstartdate to t
    const double yf = ( t - ODEstartdate );
    H_ASSERT( yf >= 0 && yf <= 1, "yearfraction out of bounds" );
    H_LOG( logger,Logger::DEBUG ) << "Stashing at t=" << t << ", solver pools at " << t << ": " <<
        "  atm = " << c[ SNBOX_ATMOS ] <<
        "  veg = " << c[ SNBOX_VEG ] <<
        "  det = " << c[ SNBOX_DET ] <<
        "  soil = " << c[ SNBOX_SOIL ] <<
        "  ocean = " << c[ SNBOX_OCEAN ] <<
        "  earth = " << c[ SNBOX_EARTH ] << std::endl;

    log_pools( t );
    // get the UNTRACKED earth emissions (ffi) and uptake (ccs)
    // We immediately adjust them for the year fraction (as the solver
    // may call stashCValues multiple times within a given year)
    fluxpool ffi_untracked = ffi(t, in_spinup) * yf;  // function also used by calcDerivs()
    fluxpool ccs_untracked = ccs(t, in_spinup) * yf;  // function also used by calcDerivs()

    // now construct the TRACKED versions
    // because earth_c is tracked, ffi and ccs automatically become tracked as well
    fluxpool ffi_flux = earth_c.flux_from_fluxpool(ffi_untracked);
    fluxpool ccs_flux = atmos_c.flux_from_fluxpool(ccs_untracked);

    // current ocean fluxes
    // TODO: Add all other ocean pools and get fluxes from DOECLIM and ocean box of Hector
    unitval ocean_atmos = unitval(c[SNBOX_OCEAN] - ocean_model_c.value(U_PGC), U_PGC);
    fluxpool oa_flux(0.0, U_PGC);
    fluxpool ao_flux(0.0, U_PGC);
    if(ocean_atmos > 0){
        ao_flux = atmos_c.flux_from_unitval(ocean_atmos);
        oa_flux = ocean_model_c.flux_from_fluxpool(oa_flux);
    } else {
        oa_flux = ocean_model_c.flux_from_unitval(-ocean_atmos);
        ao_flux = atmos_c.flux_from_fluxpool(ao_flux);
    }

    fluxpool luc_e_untracked = luc_emission(t, in_spinup) * yf;
    fluxpool luc_u_untracked = luc_uptake(t, in_spinup) * yf;

    // Land-use change uptake from atmosphere to veg, detritus, and soil
    fluxpool luc_fav_flux = atmos_c.flux_from_fluxpool(luc_u_untracked * f_lucv);
    fluxpool luc_fad_flux = atmos_c.flux_from_fluxpool(luc_u_untracked * f_lucd);
    fluxpool luc_fas_flux = atmos_c.flux_from_fluxpool(luc_u_untracked * ( 1 - f_lucv - f_lucd ));

    // Record the land C flux
    const fluxpool npp_total = sum_npp();
    const fluxpool rh_total = sum_rh();

    // TODO: If/when we implement fire, update this calculation to include it
    // (as a negative term).
    // BBL-TODO this is really "exchange" not a flux
    // pull these values into doubles as we're constructing a unitval exchange from positive-only fluxpools
    const double alf = npp_total.value(U_PGC_YR) - rh_total.value(U_PGC_YR) - lucEmissions.get( t ).value(U_PGC_YR);
    atmosland_flux.set(alf, U_PGC_YR);
    atmosland_flux_ts.set(t, atmosland_flux);

    // The solver just knows about one vegetation box, one detritus, and one
    // soil. So we need to apportion new veg C pool (set by the solver) to
    // as many biomes as we have. This is not ideal.
    // TODO: Solver actually solves all boxes in multi-biome system

    // Apportioning is done by NPP and RH
    // i.e., biomes with higher values get more of any C change
    fluxpool npp_rh_total = npp_total + rh_total; // these are both positive
    const unitval newveg( c[ SNBOX_VEG ], U_PGC );
    const unitval newdet( c[ SNBOX_DET ], U_PGC );
    const unitval newsoil( c[ SNBOX_SOIL ], U_PGC );

    for( auto it = biome_list.begin(); it != biome_list.end(); it++ ) {
        std::string biome = *it;
        const double wt = (npp(biome) + rh( biome ) ) / npp_rh_total;

        fluxpool npp_biome = yf * npp(biome);

        // Update atmosphere with luc emissons from all land pools and biomes
        fluxpool luc_fva_biome_flux = veg_c[ biome ].flux_from_fluxpool((luc_e_untracked*f_lucv)*wt);
        fluxpool luc_fda_biome_flux = detritus_c[biome].flux_from_fluxpool((luc_e_untracked*f_lucd)*wt);
        fluxpool luc_fsa_biome_flux = soil_c[biome].flux_from_fluxpool((luc_e_untracked*( 1 - f_lucv - f_lucd ))*wt);
        atmos_c = atmos_c + luc_fva_biome_flux - luc_fav_flux*wt;
        atmos_c = atmos_c + luc_fda_biome_flux - luc_fad_flux*wt;
        atmos_c = atmos_c + luc_fsa_biome_flux - luc_fas_flux*wt;

        // Update veg_c, detritus_c, and soil_c with luc uptake from atmos per biome
        veg_c[ biome ] = veg_c[ biome ] + luc_fav_flux*wt - luc_fva_biome_flux;
        detritus_c[ biome ] = detritus_c[ biome ] + luc_fad_flux*wt - luc_fda_biome_flux;
        soil_c[ biome ] = soil_c[ biome ] + luc_fas_flux*wt - luc_fsa_biome_flux;

        // Update all pools for NPP
        fluxpool npp_fav_biome_flux = atmos_c.flux_from_fluxpool(npp_biome * f_nppv.at(biome));
        fluxpool npp_fad_biome_flux = atmos_c.flux_from_fluxpool(npp_biome* f_nppd.at(biome));
        fluxpool npp_fas_biome_flux = atmos_c.flux_from_fluxpool(npp_biome * ( 1 - f_nppv.at(biome) - f_nppd.at(biome)));
        veg_c[ biome ] = veg_c[ biome ] + npp_fav_biome_flux;
        detritus_c[ biome ] = detritus_c[ biome ] + npp_fad_biome_flux;
        soil_c[ biome ] = soil_c[ biome ] + npp_fas_biome_flux;
        atmos_c = atmos_c - npp_fav_biome_flux - npp_fad_biome_flux - npp_fas_biome_flux;

        // Update soil, detritus, and atmosphere with RH fluxes
        fluxpool rh_fda_flux = detritus_c[ biome ].flux_from_fluxpool(yf * rh_fda(biome));
        fluxpool rh_fsa_flux = soil_c[ biome ].flux_from_fluxpool(yf * rh_fsa(biome));
        atmos_c = atmos_c + rh_fda_flux + rh_fsa_flux;
        detritus_c[ biome ] = detritus_c[ biome ] - rh_fda_flux;
        soil_c[ biome ] = soil_c[ biome ] - rh_fsa_flux;

        // Update litter from veg to soil and detritus
        fluxpool litter_flux = veg_c[ biome ] * (0.035 * yf);
        fluxpool litter_fvd_flux = litter_flux * f_litterd.at(biome);
        fluxpool litter_fvs_flux = litter_flux * (1 - f_litterd.at(biome));
        detritus_c[ biome ] = detritus_c[ biome ] + litter_fvd_flux;
        soil_c[ biome ] = soil_c[ biome ] + litter_fvs_flux;
        veg_c[ biome ] = veg_c[ biome ] - litter_flux;

        // Update detritus and soil with detsoil flux
        fluxpool detsoil_flux = detritus_c[ biome ] * (0.6 * yf);
        soil_c[ biome ] = soil_c[ biome ] + detsoil_flux;
        // THIS IS THE ONE THAT CAUSES THE TRACKING TO GET MESSED UP
        detritus_c[ biome ] = detritus_c[ biome ] - detsoil_flux;

        // TEMPORARY TO INVESTIGATE FLUXES
        veg_diff[biome] = veg_c[biome].value(U_PGC) - c[SNBOX_VEG]*wt;
        soil_diff[biome] = soil_c[biome].value(U_PGC) - c[SNBOX_SOIL]*wt;
        det_diff[ biome ] = detritus_c[biome].value(U_PGC) - c[SNBOX_DET]*wt;
        // Adjust biome pools to final values from calcDerives
        veg_c[ biome ].adjust_pool_to_val(newveg.value(U_PGC) * wt, false);
        detritus_c[ biome ].adjust_pool_to_val(newdet.value(U_PGC) * wt, false);
        soil_c[ biome ].adjust_pool_to_val(newsoil.value(U_PGC) * wt, false);

        H_LOG( logger,Logger::DEBUG ) << "Biome " << biome << " weight = " << wt << std::endl;
    }

    // Update earth_c and atmos_c with fossil fuel related fluxes
    earth_c = (earth_c - ffi_flux) + ccs_flux;
    atmos_c = (atmos_c + ffi_flux) - ccs_flux;

    // ocean-atmosphere flux adjustment
    ocean_model_c = ocean_model_c - oa_flux + ao_flux;
    atmos_c = atmos_c + oa_flux - ao_flux;

    // TEMPORARY TO INVESTIGATE FLUXES
    earth_diff = earth_c - c[SNBOX_EARTH];
    atmos_diff = atmos_c - c[SNBOX_ATMOS];
    // adjusts non-biome pools to output from calcderivs
    earth_c.adjust_pool_to_val(c[SNBOX_EARTH], false);
    atmos_c.adjust_pool_to_val(c[SNBOX_ATMOS], false);

    omodel->stashCValues( t, c );   // tell ocean model to store new C values

    log_pools( t );

    // Each time the model pools are updated, check that mass has been conserved
    double sum=0.0;
    for( int i=0; i<ncpool(); i++ ) {
        sum += c[ i ];
    }

    const double diff = fabs( sum - masstot );
    H_LOG( logger,Logger::DEBUG ) << "masstot = " << masstot << ", sum = " << sum << ", diff = " << diff << std::endl;
    if(masstot > 0.0 && diff > MB_EPSILON) {
        H_LOG( logger,Logger::SEVERE ) << "Mass not conserved in " << getComponentName() << std::endl;
        H_LOG( logger,Logger::SEVERE ) << "masstot = " << masstot << ", sum = " << sum << ", diff = " << diff << std::endl;
        H_THROW( "Mass not conserved! (See log.)" );
    }
    masstot = sum;

    // If user has supplied Ca values, adjust atmospheric C to match
    if(core->inSpinup() ||
       ( CO2_constrain.size() && CO2_constrain.exists(t) )) {

        fluxpool atmos_cpool_to_match;
        fluxpool atmppmv;
        if(core->inSpinup()) {
            atmos_cpool_to_match.set(C0.value(U_PPMV_CO2) / PGC_TO_PPMVCO2, U_PGC);
            atmppmv.set(C0.value(U_PPMV_CO2), U_PPMV_CO2);
        }
        else {
            H_LOG( logger, Logger::NOTICE ) << "** Constraining atmospheric CO2 to user-supplied value" << std::endl;
            atmos_cpool_to_match.set(CO2_constrain.get(t).value( U_PPMV_CO2 ) /
                                     PGC_TO_PPMVCO2, U_PGC);
            atmppmv.set(CO2_constrain.get(t).value(U_PPMV_CO2), U_PPMV_CO2);
        }

        // Ugly: residual is a unitval, but calculated by subtracting two fluxpools, so extract value
        residual.set(atmos_c.value(U_PGC) - atmos_cpool_to_match.value(U_PGC), U_PGC);

        H_LOG( logger,Logger::DEBUG ) << t << "- have " << Ca << " want " <<  atmppmv.value( U_PPMV_CO2 ) << std::endl;
        H_LOG( logger,Logger::DEBUG ) << t << "- have " << atmos_c << " want " << atmos_cpool_to_match << "; residual = " << residual << std::endl;

        // Transfer C from atmosphere to deep ocean and update our C and Ca variables
        H_LOG( logger,Logger::DEBUG ) << "Sending residual of " << residual << " to deep ocean" << std::endl;
        core->sendMessage( M_DUMP_TO_DEEP_OCEAN, D_OCEAN_C, message_data( residual ) );
        atmos_c = atmos_c - residual;
        Ca.set( atmos_c.value( U_PGC ) * PGC_TO_PPMVCO2, U_PPMV_CO2 );
    } else {
        residual.set( 0.0, U_PGC );
    }

    // All good! t will be the start of the next timestep, so
    ODEstartdate = t;
}

// A series of small functions to calculate variables that will appear in the output stream

double SimpleNbox::calc_co2fert(std::string biome, double time) const
{
    unitval Ca_t;
    if(time == Core::undefinedIndex()) {
        Ca_t = Ca;
    } else {
        Ca_t = Ca_ts.get(time);
    }
    return 1 + beta.at(biome) * log(Ca_t/C0);
}

//------------------------------------------------------------------------------
/*! \brief      Compute annual net primary production
 *  \returns    current annual NPP
 */
fluxpool SimpleNbox::npp(std::string biome, double time) const
{
    fluxpool npp(npp_flux0.at( biome ).value(U_PGC_YR), U_PGC_YR);    // 'at' throws exception if not found
    if(time == Core::undefinedIndex()) {
        npp = npp * co2fert.at( biome );        // that's why used here instead of []
    }
    else {
        npp = npp * calc_co2fert(biome, time);
    }
    return npp;
}

//------------------------------------------------------------------------------
/*! \brief      Compute global net primary production
 *  \returns    Annual NPP summed across all biomes
 */
fluxpool SimpleNbox::sum_npp(double time) const
{
    fluxpool total( 0.0, U_PGC_YR );
    for( auto it = biome_list.begin(); it != biome_list.end(); it++ ) {
        total = total + npp( *it, time );}
    return total;
}

//------------------------------------------------------------------------------
/*! \brief      Compute detritus component of annual heterotrophic respiration
 *  \returns    current detritus component of annual heterotrophic respiration
 */
fluxpool SimpleNbox::rh_fda( std::string biome, double time ) const
{
    unitval det_t;
    double tfd;
    if(time == Core::undefinedIndex()) {
        det_t = detritus_c.at( biome );
        tfd = tempfertd.at( biome );
    } else {
        det_t = detritus_c_tv.get( time ).at( biome );
        tfd = tempfertd_tv.get( time ).at( biome );
        //double_stringmap tfd = tempfertd_tv[ time ];
    }
    fluxpool dflux( det_t.value( U_PGC ) * 0.25, U_PGC_YR );
    //return dflux * tempfertd.at( biome, time );
    return dflux * tfd;
}

//------------------------------------------------------------------------------
/*! \brief      Compute soil component of annual heterotrophic respiration
 *  \returns    current soil component of annual heterotrophic respiration
 */
fluxpool SimpleNbox::rh_fsa( std::string biome, double time ) const
{
    unitval soil_t;
    double tfs;
    if(time == Core::undefinedIndex()) {
        soil_t = soil_c.at( biome );
        tfs = tempferts.at( biome );
    } else {
        soil_t = soil_c_tv.get( time ).at( biome );
        tfs = tempferts_tv.get( time ).at( biome );
    }
    fluxpool soilflux( soil_t.value( U_PGC ) * 0.02, U_PGC_YR );
    //return soilflux * tempferts.at( biome, time );
    return soilflux * tfs;
}

//------------------------------------------------------------------------------
/*! \brief      Compute total annual heterotrophic respiration
 *  \returns    current annual heterotrophic respiration
 */
fluxpool SimpleNbox::rh( std::string biome, double time ) const
{
    // Heterotrophic respiration is the sum of fluxes from detritus and soil
    return rh_fda( biome, time ) + rh_fsa( biome, time );
}

//------------------------------------------------------------------------------
/*! \brief      Compute global heterotrophic respiration
 *  \returns    Annual RH summed across all biomes
 */
fluxpool SimpleNbox::sum_rh( double time ) const
{
    fluxpool total( 0.0, U_PGC_YR);
    for( auto it = biome_list.begin(); it != biome_list.end(); it++ ) {
        total = total + rh( *it, time );
    }
    return total;
}

//------------------------------------------------------------------------------
/*! \brief      Compute fossil fuel industrial (FFI) emissions (when input emissions > 0)
 *  \returns    FFI flux from earth to atmosphere
 */
fluxpool SimpleNbox::ffi(double t, bool in_spinup) const
{
    if( !in_spinup ) {   // no perturbation allowed if in spinup
        double totflux = ffiEmissions.get( t ).value(U_PGC_YR);
        if(totflux >= 0.0) {
            return fluxpool(totflux, U_PGC_YR);
        }
    }
    return fluxpool(0.0, U_PGC_YR);
}

//------------------------------------------------------------------------------
/*! \brief      Compute carbon capture storage (CCS) flux (when input emissions < 0)
 *  \returns    CCS flux from atmosphere to earth
 */
fluxpool SimpleNbox::ccs(double t, bool in_spinup) const
{
    if( !in_spinup ) {   // no perturbation allowed if in spinup
        double totflux = ffiEmissions.get( t ).value(U_PGC_YR);
        if(totflux < 0.0) {
            return fluxpool(-totflux, U_PGC_YR);
        }
    }
    return fluxpool(0.0, U_PGC_YR);
}

//------------------------------------------------------------------------------
/*! \brief      Compute land use change emissions (when input emissions > 0)
 *  \returns    luc flux from land to atmosphere
 */
fluxpool SimpleNbox::luc_emission(double t, bool in_spinup) const
{
    if( !in_spinup ) {   // no perturbation allowed if in spinup
        double totflux = lucEmissions.get( t ).value(U_PGC_YR);
        if(totflux >= 0.0) {
            return fluxpool(totflux, U_PGC_YR);
        }
    }
    return fluxpool(0.0, U_PGC_YR);
}

//------------------------------------------------------------------------------
/*! \brief      Compute land use change uptake (when input emissions < 0)
 *  \returns    luc flux from atmosphere to land
 */
fluxpool SimpleNbox::luc_uptake(double t, bool in_spinup) const
{
    if( !in_spinup ) {   // no perturbation allowed if in spinup
        double totflux = lucEmissions.get( t ).value(U_PGC_YR);
        if(totflux < 0.0) {
            return fluxpool(-totflux, U_PGC_YR);
        }
    }
    return fluxpool(0.0, U_PGC_YR);
}

//------------------------------------------------------------------------------
/*! \brief              Compute model fluxes for a time step
 *  \param[in]  t       time
 *  \param[in]  c       carbon pools (no units)
 *  \param[out] dcdt    carbon fluxes
 *  \returns            code indicating success or failure
 */
int SimpleNbox::calcderivs( double t, const double c[], double dcdt[] ) const
{
    // Solver is attempting to go from ODEstartdate to t
    // Atmosphere-ocean flux is calculated by ocean_component
    const int omodel_err = omodel->calcderivs( t, c, dcdt );
    const double ao_exchange = dcdt[ SNBOX_OCEAN ];
    fluxpool ocean_uptake(0.0, U_PGC_YR);
    fluxpool ocean_release(0.0, U_PGC_YR);
    if(ao_exchange >= 0.0) {
        ocean_uptake.set(ao_exchange, U_PGC_YR);
    } else {
        ocean_release.set(-ao_exchange, U_PGC_YR);
    }

    // NPP: Net primary productivity
    fluxpool npp_biome( 0.0, U_PGC_YR);
    fluxpool npp_current( 0.0, U_PGC_YR );
    fluxpool npp_fav( 0.0, U_PGC_YR );
    fluxpool npp_fad( 0.0, U_PGC_YR );
    fluxpool npp_fas( 0.0, U_PGC_YR );

    // RH: heterotrophic respiration
    fluxpool rh_fda_current( 0.0, U_PGC_YR );
    fluxpool rh_fsa_current( 0.0, U_PGC_YR );

    for( auto it = biome_list.begin(); it != biome_list.end(); it++ ) {
        std::string biome = *it;
        // NPP is scaled by CO2 from preindustrial value
        npp_biome = npp( biome );
        npp_current = npp_current + npp_biome;
        npp_fav = npp_fav + npp_biome * f_nppv.at( biome );
        npp_fad = npp_fad + npp_biome * f_nppd.at( biome );
        npp_fas = npp_fas + npp_biome * (1 - f_nppv.at( biome ) - f_nppd.at( biome ));
        rh_fda_current = rh_fda_current + rh_fda( biome );
        rh_fsa_current = rh_fsa_current + rh_fsa( biome );
    }
    fluxpool rh_current = rh_fda_current + rh_fsa_current;

    // Detritus flux comes from the vegetation pool
    // TODO: these values should use the c[] pools passed in by solver!
    fluxpool litter_flux( 0.0, U_PGC_YR );
    fluxpool litter_fvd( 0.0, U_PGC_YR );
    fluxpool litter_fvs( 0.0, U_PGC_YR );
    for( auto it = biome_list.begin(); it != biome_list.end(); it++ ) {
        std::string biome = *it;
        fluxpool v = fluxpool( veg_c.at( biome ).value( U_PGC ) * 0.035, U_PGC_YR );
        litter_flux = litter_flux + v;
        litter_fvd = litter_fvd + v * f_litterd.at( biome );
        litter_fvs = litter_fvs + v * ( 1 - f_litterd.at( biome ) );
    }

    // Some detritus goes to soil
    fluxpool detsoil_flux( 0.0, U_PGC_YR );
    for( auto it = biome_list.begin(); it != biome_list.end(); it++ ) {
        std::string biome = *it;
        detsoil_flux = detsoil_flux + fluxpool( detritus_c.at( biome ).value( U_PGC ) * 0.6, U_PGC_YR );
    }

    // Annual fossil fuels and industry emissions and atmosphere CO2 capture (CCS or whatever)
    fluxpool ffi_flux_current = ffi(t, in_spinup);
    fluxpool ccs_flux_current = ccs(t, in_spinup);

    // Annual land use change emissions
    fluxpool luc_emission_current = luc_emission(t, in_spinup);
    fluxpool luc_uptake_current = luc_uptake(t, in_spinup);

    // Land-use change emissions come from veg, detritus, and soil
    fluxpool luc_fva = luc_emission_current * f_lucv;
    fluxpool luc_fda = luc_emission_current * f_lucd;
    fluxpool luc_fsa = luc_emission_current * ( 1 - f_lucv - f_lucd );
    // ...treat uptake the same way
    fluxpool luc_fav = luc_uptake_current * f_lucv;
    fluxpool luc_fad = luc_uptake_current * f_lucd;
    fluxpool luc_fas = luc_uptake_current * ( 1 - f_lucv - f_lucd );

    // Oxidized methane of fossil fuel origin
    fluxpool ch4ox_current( 0.0, U_PGC_YR );     //TODO: implement this

    // Compute fluxes
    dcdt[ SNBOX_ATMOS ] = // change in atmosphere pool
        ffi_flux_current.value( U_PGC_YR )
        - ccs_flux_current.value( U_PGC_YR )
        + luc_emission_current.value( U_PGC_YR )
        - luc_uptake_current.value( U_PGC_YR )
        + ch4ox_current.value( U_PGC_YR )
        - ocean_uptake.value( U_PGC_YR )
        + ocean_release.value( U_PGC_YR )
        - npp_current.value( U_PGC_YR )
        + rh_current.value( U_PGC_YR );
    dcdt[ SNBOX_VEG ] = // change in vegetation pool
        npp_fav.value( U_PGC_YR )
        - litter_flux.value( U_PGC_YR )
        - luc_fva.value( U_PGC_YR )
        + luc_fav.value( U_PGC_YR );
    dcdt[ SNBOX_DET ] = // change in detritus pool
        npp_fad.value( U_PGC_YR )
        + litter_fvd.value( U_PGC_YR )
        - detsoil_flux.value( U_PGC_YR )
        - rh_fda_current.value( U_PGC_YR )
        - luc_fda.value( U_PGC_YR )
        + luc_fad.value( U_PGC_YR );
    dcdt[ SNBOX_SOIL ] = // change in soil pool
        npp_fas.value( U_PGC_YR )
        + litter_fvs.value( U_PGC_YR )
        + detsoil_flux.value( U_PGC_YR )
        - rh_fsa_current.value( U_PGC_YR )
        - luc_fsa.value( U_PGC_YR )
        + luc_fas.value( U_PGC_YR );
    dcdt[ SNBOX_OCEAN ] = // change in ocean pool
        ocean_uptake.value( U_PGC_YR )
        - ocean_release.value( U_PGC_YR );
    dcdt[ SNBOX_EARTH ] = // change in earth pool
        - ffi_flux_current.value( U_PGC_YR )
        + ccs_flux_current.value( U_PGC_YR );

/*    printf( "%6.3f%8.3f%8.2f%8.2f%8.2f%8.2f%8.2f\n", t, dcdt[ SNBOX_ATMOS ],
            dcdt[ SNBOX_VEG ], dcdt[ SNBOX_DET ], dcdt[ SNBOX_SOIL ], dcdt[ SNBOX_OCEAN ], dcdt[ SNBOX_EARTH ] );
*/
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
void SimpleNbox::slowparameval( double t, const double c[] )
{
    omodel->slowparameval( t, c );      // pass msg on to ocean model

    // CO2 fertilization
    Ca.set( c[ SNBOX_ATMOS ] * PGC_TO_PPMVCO2, U_PPMV_CO2 );

    // Compute CO2 fertilization factor globally (and for each biome specified)
    for( auto it = biome_list.begin(); it != biome_list.end(); it++ ) {
        std::string biome = *it;
        if( in_spinup ) {
            co2fert[ biome ] = 1.0;  // no perturbation allowed if in spinup
        } else {
            co2fert[ biome ] = calc_co2fert( biome );
        }
        H_LOG( logger,Logger::DEBUG ) << "co2fert[ " << biome << " ] at " << Ca << " = " << co2fert.at( biome ) << std::endl;
    }

    // Compute temperature factor globally (and for each biome specified)
    // Heterotrophic respiration depends on the pool sizes (detritus and soil) and Q10 values
    // The soil pool uses a lagged Tgav, i.e. we assume it takes time for heat to diffuse into soil
    const double Tgav = core->sendMessage( M_GETDATA, D_GLOBAL_TEMP ).value( U_DEGC );

    /* set tempferts (soil) and tempfertd (detritus) for each biome */

    // Need the previous time step values of tempferts.  Since t is
    // the time at the beginning of the current time step (== the end
    // of the previous time step), we can use t as the index to look
    // up the previous value.
    double_stringmap tfs_last;  // Previous time step values of tempferts; initialized empty
    if(t != Core::undefinedIndex() && t > core->getStartDate()) {
        tfs_last = tempferts_tv[t];
    }

    // Loop over biomes
    for( auto it = biome_list.begin(); it != biome_list.end(); it++ ) {
        std::string biome = *it;
        if( in_spinup ) {
            tempfertd[ biome ] = 1.0;  // no perturbation allowed in spinup
            tempferts[ biome ] = 1.0;  // no perturbation allowed in spinup
        } else {
            double wf;
            if( warmingfactor.count( biome ) ) {
                wf = warmingfactor.at( biome );   // biome-specific warming
            } else if ( warmingfactor.count( SNBOX_DEFAULT_BIOME ) ) {
                wf = warmingfactor.at( SNBOX_DEFAULT_BIOME );
            } else {
                wf = 1.0;
            }

            const double Tgav_biome = Tgav * wf;    // biome-specific temperature

            tempfertd[ biome ] = pow( q10_rh.at( biome ), ( Tgav_biome / 10.0 ) ); // detritus warms with air


            // Soil warm very slowly relative to the atmosphere
            // We use a mean temperature of a window (size Q10_TEMPN) of temperatures to scale Q10
            #define Q10_TEMPLAG 0 //125         // TODO: put lag in input files 150, 25
            #define Q10_TEMPN 200 //25
            double Tgav_rm = 0.0;       /* window mean of Tgav */
            if( t > core->getStartDate() + Q10_TEMPLAG ) {
                for( int i=t-Q10_TEMPLAG-Q10_TEMPN; i<t-Q10_TEMPLAG; i++ ) {
                    Tgav_rm += Tgav_record.get( i ) * wf;
                }
                Tgav_rm /= Q10_TEMPN;
            }

            tempferts[ biome ] = pow( q10_rh.at( biome ), ( Tgav_rm / 10.0 ) );

            // The soil Q10 effect is 'sticky' and can only increase, not decline
            double tempferts_last = tfs_last[ biome ]; // If tfs_last is empty, this will produce 0.0
            if(tempferts[ biome ] < tempferts_last) {
                tempferts[ biome ] = tempferts_last;
            }

            H_LOG( logger,Logger::DEBUG ) << biome << " Tgav=" << Tgav << ", Tgav_biome=" <<
                Tgav_biome << ", tempfertd=" << tempfertd[ biome ] << ", tempferts=" <<
                tempferts[ biome ] << std::endl;
        }
    } // loop over biomes

    H_LOG(logger, Logger::DEBUG) << "slowparameval: would have recorded tempferts = " <<
        tempferts[SNBOX_DEFAULT_BIOME] << " at time= " << tcurrent << std::endl;
}

}

