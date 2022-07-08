/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
 *  ocean_component.cpp
 *  hector
 *
 *  Created by Corinne Hartin on 1/3/13.
 *
 *  Structure of the ocean component (high latitude, low latitude, intermediate & deep ocean) is
 *  based on Knox & McElroy.
 *      Knox, F. and McElroy, M. B.: Changes in Atmospheric CO2: Influence
 *          of the Marine Biota at High Latitude, J. Geophys. Res., 89,
 *          4629–4637, doi:10.1029/JD089iD03p04629, 1984.
 *
 *  Other References
 *
 *  Hartin, C. A., Bond-Lamberty, B., Patel, P., and Mundra, A.: Ocean acidification over
 *      the next three centuries using a simple global climate carbon-cycle model:
 *      projections and sensitivities, Biogeosciences, 13, 4329–4342,
 *      https://doi.org/10.5194/bg-13-4329-2016, 2016.
 *
 *  Riley, J. P. and Tongudai, M.: The major cation/chlorinity ratios
 *      in sea water, Chem. Geol., 2, 263–269, doi:10.1016/0009-
 *      2541(67)90026-5, 1967.
 */

#include <cmath>
#include <limits>

#include "ocean_component.hpp"
#include "core.hpp"
#include "h_util.hpp"
#include "simpleNbox.hpp"
#include "avisitor.hpp"

namespace Hector {

using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 */

OceanComponent::OceanComponent() {
    spinup_chem = true;
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
OceanComponent::~OceanComponent() {
}

//------------------------------------------------------------------------------
// documentation is inherited
string OceanComponent::getComponentName() const {
    const string name = OCEAN_COMPONENT_NAME;

    return name;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OceanComponent::init( Core* coreptr ) {
    logger.open( getComponentName(), false, coreptr->getGlobalLogger().getEchoToFile(), coreptr->getGlobalLogger().getMinLogLevel() );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;

    max_timestep = OCEAN_MAX_TIMESTEP;
    reduced_timestep_timeout = 0;

	surfaceHL.logger = &logger;
	surfaceLL.logger = &logger;
	inter.logger = &logger;
	deep.logger = &logger;

    core = coreptr;

    SST.set( 0.0, U_DEGC );

	lastflux_annualized.set( 0.0, U_PGC );

    // Register the data we can provide
    core->registerCapability( D_OCEAN_C_UPTAKE, getComponentName() );
    core->registerCapability( D_OCEAN_C, getComponentName() );
    core->registerCapability( D_CARBON_HL, getComponentName() );
    core->registerCapability( D_CARBON_LL, getComponentName() );
    core->registerCapability( D_CARBON_IO, getComponentName() );
    core->registerCapability( D_CARBON_DO, getComponentName() );
    core->registerCapability( D_CARBON_ML, getComponentName() );
    core->registerCapability( D_TT, getComponentName() );
    core->registerCapability( D_TU, getComponentName() );
    core->registerCapability( D_TWI, getComponentName() );
    core->registerCapability( D_TID, getComponentName() );
    core->registerCapability( D_PH_HL, getComponentName() );
    core->registerCapability( D_PH_LL, getComponentName() );
    core->registerCapability( D_PH, getComponentName() );
    core->registerCapability( D_ATM_OCEAN_FLUX_HL, getComponentName() );
    core->registerCapability( D_ATM_OCEAN_FLUX_LL, getComponentName() );
    core->registerCapability( D_PCO2_HL, getComponentName() );
    core->registerCapability( D_PCO2_LL, getComponentName() );
    core->registerCapability( D_PCO2, getComponentName() );
    core->registerCapability( D_DIC_HL, getComponentName() );
    core->registerCapability( D_DIC_LL, getComponentName() );
    core->registerCapability( D_DIC, getComponentName() );
    core->registerCapability( D_TEMP_HL, getComponentName() );
    core->registerCapability( D_TEMP_LL, getComponentName() );
    core->registerCapability( D_CO3_HL, getComponentName() );
    core->registerCapability( D_CO3_LL, getComponentName() );
    core->registerCapability( D_CO3, getComponentName() );


    // Register the inputs we can receive from outside
    core->registerInput(D_TT, getComponentName());
    core->registerInput(D_TU, getComponentName());
    core->registerInput(D_TWI, getComponentName());
    core->registerInput(D_TID, getComponentName());
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval OceanComponent::sendMessage( const std::string& message,
                                    const std::string& datum,
                                    const message_data info )
{
    unitval returnval;

    if( message == M_GETDATA ) {          //! Caller is requesting data
        return getData( datum, info.date );

    } else if( message == M_SETDATA ) {   //! Caller is requesting to set data
        setData(datum, info);
        //TODO: change core so that parsing is routed through sendMessage
        //TODO: make setData private

	} else if( message == M_DUMP_TO_DEEP_OCEAN ) {
        // info struct holds the amount being dumped/extracted from deep ocean
        unitval carbon = info.value_unitval;
        H_LOG( logger, Logger::DEBUG ) << "Atmosphere dumping " << carbon << " Pg C to deep ocean" << std::endl;

        // We don't want this to be tracked, so just overwrite the deep total
        carbon = carbon + unitval(deep.get_carbon().value( U_PGC ), U_PGC);
        deep.set_carbon( carbon );

    } else { //! We don't handle any other messages
        H_THROW( "Caller sent unknown message: "+message );
    }

    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OceanComponent::setData( const string& varName,
                              const message_data& data )
{
    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << data.date << "]=" << data.value_str << std::endl;

    try {
        if( varName == D_CARBON_HL ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            surfaceHL.set_carbon( data.getUnitval( U_PGC ) );
        } else if( varName == D_CARBON_LL ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            surfaceLL.set_carbon( data.getUnitval( U_PGC ) );
        } else if( varName == D_CARBON_IO ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            inter.set_carbon( data.getUnitval( U_PGC ) );
        } else if( varName == D_CARBON_DO ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            deep.set_carbon( data.getUnitval( U_PGC ) );
        } else if( varName == D_TT ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            tt.set( data.getUnitval(U_M3_S), U_M3_S );
        } else if( varName == D_TU ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            tu.set( data.getUnitval(U_M3_S), U_M3_S );
       } else if( varName == D_TWI ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            twi.set( data.getUnitval(U_M3_S), U_M3_S) ;
      } else if( varName == D_TID ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            tid.set( data.getUnitval(U_M3_S), U_M3_S) ;
		} else if( varName == D_SPINUP_CHEM ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            spinup_chem = (data.getUnitval(U_UNDEFINED) > 0);
        } else {
            H_THROW( "Unknown variable name while parsing " + getComponentName() + ": "
                    + varName );
        }
    } catch( h_exception& parseException ) {
        H_RETHROW( parseException, "Could not parse var: "+varName );
    }
}

//------------------------------------------------------------------------------
// documentation is inherited
void OceanComponent::prepareToRun() {

    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;

    // Define constants used in ocean box set up.
    const double spy = 60 * 60 * 24 * 365.25;  // seconds per year

    // ocean depth
    double thick_LL = 100;         // (m) Thickness of surface ocean from Knox and McElroy (1984)
    double thick_HL = 100;         // (m) Thickness of surface ocean from Knox and McElroy (1984)
    double thick_inter = 1000-thick_LL;  // (m) Thickness of of intermediate ocean from Knox and McElroy (1984)
    double thick_deep = 3777-thick_inter-thick_LL; // (m) Thickness of deep ocean from Knox and McElroy (1984)

    // ocean area
    const double ocean_area = 3.6e14; // (m2) Knox and McElroy (1984);

    // Define high and low latitude
    // The cold high-latitude surface box makes up 15% of the total ocean surface area and has latitude > 55
    // The warm low-latitude surface box makes up the rest.
    //const double part_high = 0.15;
    //const double part_low = 1-part_high;

    // ocean box volumes (m3)
    const double LL_volume = ocean_area * part_low * thick_LL;
    const double HL_volume = ocean_area * part_high * thick_HL;
    const double I_volume = ocean_area * thick_inter;
    const double D_volume = ocean_area * thick_deep;

    // Calculate the fraction of volume of the two surface boxes, the deep, and
    // intermediate ocean boxes. The fraction of the volumes will be used to determine the
    // inital size of the preindustrial carbon pools for Hector's ocean boxes.
    const double LL_vol_frac = LL_volume / (LL_volume + HL_volume);
    const double HL_vol_frac = 1 - LL_vol_frac;
    const double I_vol_frac = I_volume / (I_volume + D_volume);
    const double D_vol_frac = 1 - I_vol_frac;

    // Define the size of the preindustrial ocean surface and intermediate-deep ocean carbon pools
    // from IPCC AR6 Figure 5.12.
    const double preind_C_surface = 900;   // (Pg C) IPCC AR6 Figure 5.12
    const double preind_C_ID = 37100;      // (Pg C) IPCC AR6 Figure 5.12

    // Partition the preindustrial ocean carbon pools by volume.
    const double LL_preind_C = LL_vol_frac * preind_C_surface;
    const double HL_preind_C = HL_vol_frac * preind_C_surface;
    const double I_preind_C = I_vol_frac * preind_C_ID;
    const double D_preind_C = D_vol_frac * preind_C_ID;

    // Set up our ocean box model.
    H_LOG( logger, Logger::DEBUG ) << "Setting up ocean box model" << std::endl;
    surfaceHL.initbox( HL_preind_C, "HL" );
    surfaceHL.surfacebox = true;
    surfaceHL.preindustrial_flux.set( 1.000, U_PGC_YR );         // used if no spinup chemistry
    surfaceHL.active_chemistry = spinup_chem;

    surfaceLL.initbox( LL_preind_C, "LL" );
    surfaceLL.surfacebox = true;
    surfaceLL.preindustrial_flux.set( -1.000, U_PGC_YR );        // used if no spinup chemistry
    surfaceLL.active_chemistry = spinup_chem;

    inter.initbox( I_preind_C, "intermediate" );
    deep.initbox( D_preind_C, "deep" );

    // transport * seconds / volume of box
    // Advection --> transport of carbon from one box to the next (k values, fraction/yr )
    double LL_HL = ( tt.value( U_M3_S ) * spy ) / LL_volume;
    double HL_DO = ( ( tt + tu).value( U_M3_S ) * spy ) / HL_volume;
    double DO_IO = ( ( tt + tu).value( U_M3_S ) * spy ) / D_volume;
    double IO_HL = ( tu.value( U_M3_S) * spy )  / I_volume;
    double IO_LL = ( tt.value( U_M3_S) * spy )  / I_volume;

    // Exchange parameters --> not explicitly modeling diffusion
    double IO_LLex = ( twi.value( U_M3_S) * spy ) / I_volume;
    double LL_IOex = ( twi.value( U_M3_S) * spy ) / LL_volume;
    double DO_IOex = ( tid.value( U_M3_S) * spy ) / D_volume;
    double IO_DOex = ( tid.value( U_M3_S) * spy ) / I_volume;

    // Make_connection( box to connect to, k value, window size (0=present only) )
    surfaceLL.make_connection( &surfaceHL, LL_HL, 1 );
    surfaceLL.make_connection( &inter, LL_IOex, 1 );
    surfaceHL.make_connection( &deep, HL_DO, 1 );
    inter.make_connection( &surfaceLL, IO_LL + IO_LLex, 1 );
    inter.make_connection( &surfaceHL, IO_HL, 1 );
    inter.make_connection( &deep, IO_DOex, 1 );
    deep.make_connection( &inter, DO_IO + DO_IOex, 1 );

    // Inputs for surface chemistry boxes
    surfaceHL.deltaT.set( -16.4, U_DEGC );  // delta T to the absolute mean ocean tos to return the initial temperature value of the HL surface. See hector_cmip6data for details.
    surfaceHL.mychemistry.S             = 34.5; // Salinity Riley and Tongudai (1967)
    surfaceHL.mychemistry.volumeofbox   = HL_volume; // m3
    surfaceHL.mychemistry.As            = ocean_area * part_high ; // surface area m2
    surfaceHL.mychemistry.U             = 6.7; // average wind speed m/s Hartin et al. 2016

    surfaceLL.deltaT.set( 2.9, U_DEGC );  // delta T to the absolute mean ocean tos to return the initial temperature value of the LL surface. See hector_cmip6data for details.
    surfaceLL.mychemistry.S             = 34.5;  // Salinity Riley and Tongudai (1967)
    surfaceLL.mychemistry.volumeofbox   = LL_volume; //m3
    surfaceLL.mychemistry.As            = ocean_area * part_low; // surface area m2
    surfaceLL.mychemistry.U             = 6.7; // average wind speed m/s Hartin et al. 2016

    // Initialize surface flux tracking variables
    annualflux_sum.set( 0.0, U_PGC );
    annualflux_sumHL.set( 0.0, U_PGC );
    annualflux_sumLL.set( 0.0, U_PGC );

    // Log the state of all our boxes, so we know things are as they should be
    surfaceLL.log_state();
    surfaceHL.log_state();
    inter.log_state();
    deep.log_state();
}

//------------------------------------------------------------------------------
/*! \brief      Internal function to add up all model C pools
 *  \returns    unitval, total carbon in the ocean
 */
fluxpool OceanComponent::totalcpool() const {
	return deep.get_carbon() + inter.get_carbon() + surfaceLL.get_carbon() + surfaceHL.get_carbon();
}

//------------------------------------------------------------------------------
/*! \brief                  Internal function to calculate atmosphere-ocean C flux
 *  \param[in] date         double, date of calculation (in case constraint used)
 *  \param[in] CO2_conc     unitval, atmospheric CO2 concentration
 *  \param[in] cpoolscale   double, how much to scale surface C pools by
 *  \returns                unitval, annual atmosphere-ocean C flux
 */
unitval OceanComponent::annual_totalcflux( const double date, const unitval& CO2_conc, const double cpoolscale ) const {

    unitval flux( 0.0, U_PGC_YR );

    if( in_spinup && !spinup_chem ) {
        flux = surfaceHL.preindustrial_flux + surfaceLL.preindustrial_flux;
    } else {
        flux = surfaceHL.mychemistry.calc_annual_surface_flux( CO2_conc, cpoolscale )
                            + surfaceLL.mychemistry.calc_annual_surface_flux( CO2_conc, cpoolscale );
    }

    return flux;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OceanComponent::run( const double runToDate ) {

    // If we've hit the tracking start year, enagage!
    const double tdate = core->getTrackingDate();
    if(!in_spinup && runToDate == tdate){
        H_LOG( logger, Logger::NOTICE ) << "Tracking start" << std::endl;
        surfaceHL.start_tracking();
        surfaceLL.start_tracking();
        inter.start_tracking();
        deep.start_tracking();
    }

    CO2_conc = core->sendMessage( M_GETDATA, D_CO2_CONC, message_data( runToDate ) );
    SST.set(core->sendMessage( M_GETDATA, D_SST ), U_DEGC);

    in_spinup = core->inSpinup();

	annualflux_sum.set( 0.0, U_PGC );
	annualflux_sumHL.set( 0.0, U_PGC );
	annualflux_sumLL.set( 0.0, U_PGC );
    timesteps = 0;

    // Initialize ocean box boundary conditions and inform them new year starting
    H_LOG(logger, Logger::DEBUG) << "Starting new year: SST= " << SST << std::endl;
    surfaceHL.new_year( SST );
    surfaceLL.new_year( SST );
    inter.new_year( SST );
    deep.new_year( SST );
    H_LOG( logger, Logger::DEBUG ) << "----------------------------------------------------" << std::endl;
    H_LOG( logger, Logger::DEBUG ) << "runToDate=" << runToDate << ", [CO2]=" << CO2_conc << ", spinup=" << in_spinup << std::endl;

    // If chemistry models weren't turned on during spinup, do so now
    if( !spinup_chem && !in_spinup && !surfaceHL.active_chemistry ) {

        H_LOG( logger, Logger::DEBUG ) << "*** Turning on chemistry models ***" << std::endl;
        surfaceHL.active_chemistry = true;
        surfaceLL.active_chemistry = true;
        surfaceHL.chem_equilibrate( CO2_conc );
        surfaceLL.chem_equilibrate( CO2_conc );

   }

    // Call compute_fluxes with do_boxfluxes=false to run just chemistry
	surfaceHL.compute_fluxes( CO2_conc, atmosphere_cpool, 1.0, false );
	surfaceLL.compute_fluxes( CO2_conc, atmosphere_cpool, 1.0, false );

    // Now wait for the solver to call us
}

//------------------------------------------------------------------------------
// documentation is inherited
bool OceanComponent::run_spinup( const int step ) {
    run( step );
    return true;        // solver will be the one signalling
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval OceanComponent::getData( const std::string& varName,
                                const double date ) {

    unitval returnval;

    if(date == Core::undefinedIndex() ){
        // If no date, we're in spinup; just return the current value

        if( varName == D_OCEAN_C_UPTAKE ){
            returnval = annualflux_sum;
        } else if( varName == D_TT ) {
            returnval = tt;
        } else if( varName == D_TU ) {
            returnval = tu;
         } else if( varName == D_TID ) {
            returnval = tid;
         } else if( varName == D_TWI ) {
            returnval = twi;
        } else if( varName == D_OMEGACA_HL ) {
            returnval = surfaceHL.mychemistry.OmegaCa;
        } else if( varName == D_OMEGACA_LL ) {
            returnval = surfaceLL.mychemistry.OmegaCa;
        } else if( varName == D_OMEGAAR_HL ) {
            returnval = surfaceHL.mychemistry.OmegaAr;
        } else if( varName == D_OMEGAAR_LL ) {
            returnval = surfaceLL.mychemistry.OmegaAr;
        } else if( varName == D_REVELLE_HL ) {
            returnval = surfaceHL.calc_revelle();
        } else if( varName == D_REVELLE_LL ) {
            returnval = surfaceLL.calc_revelle();
        } else if( varName == D_ATM_OCEAN_FLUX_HL ) {
            returnval = unitval( annualflux_sumHL.value( U_PGC ), U_PGC_YR );
        } else if( varName == D_ATM_OCEAN_FLUX_LL ) {
            returnval = unitval( annualflux_sumLL.value( U_PGC ), U_PGC_YR );
        } else if( varName == D_CARBON_DO ) {
            returnval = deep.get_carbon();
        } else if( varName == D_CARBON_HL ) {
            returnval = surfaceHL.get_carbon();
        } else if( varName == D_CARBON_LL ) {
            returnval = surfaceLL.get_carbon();
        } else if( varName == D_CARBON_ML ) {
            returnval = surfaceLL.get_carbon() + surfaceHL.get_carbon();
        } else if( varName == D_CARBON_IO ) {
            returnval = inter.get_carbon();
        } else if( varName == D_DIC_HL ) {
            returnval = surfaceHL.mychemistry.convertToDIC( surfaceHL.get_carbon() );
        } else if( varName == D_DIC_LL ) {
            returnval = surfaceLL.mychemistry.convertToDIC( surfaceLL.get_carbon() );
        } else if( varName == D_DIC ) {
            double value = part_low * surfaceLL.mychemistry.convertToDIC( surfaceLL.get_carbon() ) + part_high *  surfaceHL.mychemistry.convertToDIC( surfaceHL.get_carbon() ) ;
            returnval = unitval(value, U_UMOL_KG);
        } else if( varName == D_HL_DO ) {
            returnval = surfaceHL.annual_box_fluxes[ &deep ] ;
        } else if( varName == D_PCO2_HL ) {
            returnval = surfaceHL.mychemistry.PCO2o;
        } else if( varName == D_PCO2_LL ) {
            returnval = surfaceLL.mychemistry.PCO2o;
        } else if( varName == D_PCO2 ) {
            double value = part_low * surfaceLL.mychemistry.PCO2o + part_high * surfaceHL.mychemistry.PCO2o;
            returnval = unitval(value, U_UATM);
        } else if( varName == D_PH_HL ) {
            returnval = surfaceHL.mychemistry.pH;
        } else if( varName == D_PH_LL ) {
            returnval = surfaceLL.mychemistry.pH;
        } else if( varName == D_PH ) {
            double value = part_low * surfaceLL.mychemistry.pH +  part_high * surfaceHL.mychemistry.pH;
            returnval = unitval(value, U_PH);
        } else if( varName == D_TEMP_HL ) {
            returnval = surfaceHL.get_Tbox();
        } else if( varName == D_TEMP_LL ) {
            returnval = surfaceLL.get_Tbox();
        } else if( varName == D_OCEAN_C ) {
            returnval = totalcpool();
        } else if( varName == D_CO3_HL ) {
            returnval = surfaceHL.mychemistry.CO3;
        } else if( varName == D_CO3_LL ) {
            returnval = surfaceLL.mychemistry.CO3;
        } else if( varName == D_CO3 ) {
            double value = part_low * surfaceLL.mychemistry.CO3 + part_high * surfaceHL.mychemistry.CO3;
            returnval = unitval(value, U_UMOL_KG);
        } else if( varName == D_TIMESTEPS ) {
            returnval = unitval( timesteps, U_UNITLESS );
        } else {
            H_THROW( "Problem with user request for constant data: " + varName );
        }

    } else if(date != Core::undefinedIndex() ){
        if( varName == D_OCEAN_C_UPTAKE ){
            returnval = annualflux_sum_ts.get(date);
        } else if( varName == D_OCEAN_C ) {
            returnval = C_DO_ts.get( date ) +  C_IO_ts.get(date) + Ca_LL_ts.get(date) + Ca_HL_ts.get(date);
        } else if( varName == D_HL_DO ) {
            returnval = C_DO_ts.get( date );
        } else if( varName == D_PH_HL ) {
            returnval = PH_HL_ts.get( date );
        } else if( varName == D_PH_LL ) {
            returnval = PH_LL_ts.get( date );
        } else if( varName == D_PH ) {
            double value = part_low * PH_LL_ts.get( date ) + part_high * PH_HL_ts.get( date ) ;
            returnval = unitval(value, U_PH);
        } else if( varName == D_ATM_OCEAN_FLUX_HL ) {
            returnval = annualflux_sumHL_ts.get(date);
        } else if( varName == D_ATM_OCEAN_FLUX_LL ) {
            returnval = annualflux_sumLL_ts.get(date);
        } else if( varName == D_PCO2_HL ) {
            returnval = pco2_HL_ts.get( date );
        } else if( varName == D_PCO2_LL ) {
            returnval = pco2_LL_ts.get( date );
        } else if( varName == D_PCO2 ) {
            double value = part_low * pco2_LL_ts.get( date ) + part_high * pco2_HL_ts.get( date );
            returnval = unitval(value, U_UATM);
        } else if( varName == D_DIC_HL ) {
            returnval = dic_HL_ts.get( date );
        } else if( varName == D_DIC_LL ) {
            returnval = dic_LL_ts.get( date );
        } else if( varName == D_DIC ) {
            double value = part_low * dic_LL_ts.get( date ) + part_high * dic_HL_ts.get( date );
            returnval = unitval(value, U_UMOL_KG);
        } else if( varName == D_CARBON_HL ) {
            returnval = Ca_HL_ts.get(date);
        } else if( varName == D_CARBON_LL ) {
            returnval = Ca_LL_ts.get(date);
        } else if( varName == D_CARBON_ML ) {
            returnval = Ca_LL_ts.get(date) +  Ca_HL_ts.get(date);
        } else if( varName == D_CARBON_IO ) {
            returnval = C_IO_ts.get(date);
        } else if( varName == D_CARBON_DO ) {
            returnval = C_DO_ts.get(date);
        } else if( varName == D_TEMP_HL ) {
            returnval = temp_HL_ts.get(date);
        } else if( varName == D_TEMP_LL ) {
            returnval = temp_LL_ts.get(date);
        } else if( varName == D_CO3_LL ) {
            returnval = co3_LL_ts.get(date);
        } else if( varName == D_CO3_HL ) {
            returnval = co3_HL_ts.get(date);
        } else if( varName == D_CO3 ) {
            double value = part_high * co3_HL_ts.get(date) + part_low * co3_LL_ts.get(date);
            returnval = unitval(value, U_UMOL_KG);
        } else {
            H_THROW( "Problem with user request for time series: " + varName );
        }

    } else {
        H_ASSERT( date == Core::undefinedIndex(), "Date data not available for " + varName + " in OceanComponent::getData()" );
    }

    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OceanComponent::getCValues( double t, double c[] ) {
    c[ SNBOX_OCEAN ] = totalcpool().value( U_PGC );

    ODEstartdate = t;
}

//------------------------------------------------------------------------------
/*! \brief              Compute atmosphere-ocean flux for a time step
 *  \param[in]  t       time
 *  \param[in]  c       carbon pools (no units)
 *  \param[out] dcdt    carbon deltas - we fill in only ocean change
 *  \returns            code indicating success or failure
 *  \details "This function should store the vector elements f_i(t,y,params)
 *  in the array dydt, for arguments (t,y) and parameters params." -GSL docs
 *  Compute the air-ocean flux (Pg C/yr) at time t and for pools c[]
 */
int OceanComponent::calcderivs( double t, const double c[], double dcdt[] ) const {

    const double yearfraction = ( t - ODEstartdate );

    // If the solver has adjusted the ocean and/or atmosphere pools,
    // need to be take into account in the flux computation
    const unitval cpooldiff = unitval( c[ SNBOX_OCEAN ], U_PGC ) - totalcpool();
    const unitval surfacepools = surfaceLL.get_carbon() + surfaceHL.get_carbon();
    const double cpoolscale = ( surfacepools + cpooldiff ) / surfacepools;
    unitval CO2_conc( c[ SNBOX_ATMOS ] * PGC_TO_PPMVCO2, U_PPMV_CO2 );

    dcdt[ SNBOX_OCEAN ] = annual_totalcflux( t, CO2_conc, cpoolscale ).value( U_PGC_YR );

    // If too big a timestep--i.e., stashCvalues below has signalled a reduced step
    // that we're exceeding--signal to the solver that this won't work for us.
    if( yearfraction > max_timestep ) {
        return CARBON_CYCLE_RETRY;
    } else {
        return ODE_SUCCESS;
    }
}

//------------------------------------------------------------------------------
// documentation is inherited
void OceanComponent::slowparameval( double t, const double c[] ) {

    in_spinup = core->inSpinup();
}

//------------------------------------------------------------------------------
/*! \brief   Return the ocean-to-atmosphere flux to simpleNbox
*  \returns           The two ocean-atmosphere fluxpools added together
*/
fluxpool OceanComponent::get_oaflux() const {
    return surfaceLL.get_oa_flux() + surfaceHL.get_oa_flux();
}

//------------------------------------------------------------------------------
/*! \brief   Return the atmosphere-to-ocean flux to simpleNbox
*  \returns           The two atmosphere-ocean fluxpools added together
*/
fluxpool OceanComponent::get_aoflux() const {
    return surfaceLL.get_ao_flux() + surfaceHL.get_ao_flux();
}

//------------------------------------------------------------------------------
// documentation is inherited
void OceanComponent::stashCValues( double t, const double c[] ) {

    H_LOG( logger, Logger::DEBUG ) << "Stashing at t=" << t << ", model pools at " << t << ": " <<
    c[ 0 ] << " " << c[ 1 ] << " " << c[ 2 ] << " " << c[ 3 ] << " " << c[ 4 ] << " " <<
    c[ 5 ] << std::endl;

	// At this point the solver has converged, going from ODEstartdate to t
    // Now we finalize calculations: circulate ocean, update carbon states, etc.
    const double yearfraction = ( t - ODEstartdate );
    //H_LOG( logger, Logger::NOTICE ) << "Solver has finished. Yearfraction = " << yearfraction << std::endl;
    H_ASSERT( yearfraction >= 0 && yearfraction <= 1, "yearfraction out of bounds" );

    timesteps++;
    const bool in_partial_year = ( t != int( t ) );

    unitval CO2_conc( c[ SNBOX_ATMOS ] * PGC_TO_PPMVCO2, U_PPMV_CO2 );

    // Compute fluxes between the boxes (advection of carbon)
    surfaceHL.compute_fluxes( CO2_conc, atmosphere_cpool, yearfraction );
	surfaceLL.compute_fluxes( CO2_conc, atmosphere_cpool, yearfraction );
	inter.compute_fluxes( CO2_conc, atmosphere_cpool, yearfraction );
	deep.compute_fluxes( CO2_conc, atmosphere_cpool, yearfraction );

    // At this point, compute_fluxes has (by calling the chemistry model) computed atmosphere-
    // ocean fluxes for the surface boxes. But these are end-of-timestep values, and we need to
    // overwrite them with what the solver has sent us (~mid-timestep values), so that everything
    // stays consistent.
    unitval currentflux = surfaceHL.atmosphere_flux + surfaceLL.atmosphere_flux;
    unitval solver_flux = unitval( c[ SNBOX_OCEAN ], U_PGC ) - totalcpool();
    unitval adjustment( 0.0, U_PGC );
    if( currentflux.value( U_PGC ) ) adjustment = ( solver_flux - currentflux ) / 2.0;
	H_LOG( logger, Logger::DEBUG) << "Solver flux = " << solver_flux << ", currentflux = " << currentflux << ", adjust = " << adjustment << std::endl;
    surfaceHL.atmosphere_flux = surfaceHL.atmosphere_flux + adjustment;
    surfaceLL.atmosphere_flux = surfaceLL.atmosphere_flux + adjustment;

    // Separate the one net flux (can be positive or negative) into the two fluxpool fluxes (always positive)
    // This updates oa_flux and ao_flux within the two ocean boxes
    surfaceHL.separate_surface_fluxes(atmosphere_cpool);
    surfaceLL.separate_surface_fluxes(atmosphere_cpool);

    // This (along with carbon-cycle-solver obviously) is the heart of the
    // reduced-timestep code. If carbon flux has exceeded some critical value,
    // we need to reduce timestep for the future.
    unitval cflux_annualdiff = solver_flux / yearfraction - lastflux_annualized;

    if( cflux_annualdiff.value( U_PGC ) > OCEAN_TSR_TRIGGER1 ) {
        // Annual fluxes are changing rapidly. Reduce the max timestep allowed.
        max_timestep = max( OCEAN_MIN_TIMESTEP, max_timestep * OCEAN_TSR_FACTOR );
        H_LOG( logger, Logger::DEBUG ) << "Reducing timestep to " << max_timestep << ": t=" << t << " yearfraction=" << yearfraction << std::endl;
        H_LOG( logger, Logger::DEBUG ) << " solver_flux=" << solver_flux << " lastflux_annualized=" << lastflux_annualized;
        H_LOG( logger, Logger::DEBUG ) << " cflux_annualdiff=" << cflux_annualdiff << std::endl;
        reduced_timestep_timeout = OCEAN_TSR_TIMEOUT;

    } else if( !in_partial_year && reduced_timestep_timeout ) {
        // Things look OK, so decrement the timeout counter if it's active
        reduced_timestep_timeout = max<int>( 0, reduced_timestep_timeout-1 );
        H_LOG( logger, Logger::DEBUG ) << "OK, reduced_timestep_timeout =" << reduced_timestep_timeout << std::endl;
        if( !reduced_timestep_timeout ) {
            H_LOG( logger, Logger::DEBUG ) << "Reduced ts timeout done; raising" << std::endl;
            max_timestep = min( OCEAN_MAX_TIMESTEP, max_timestep / OCEAN_TSR_FACTOR );
            if( max_timestep < OCEAN_MAX_TIMESTEP ) {
                reduced_timestep_timeout = OCEAN_TSR_TIMEOUT; // set timer for another raise attempt
            }
        }
    }

    // Update lastflux and add it to annual sum
    unitval lastflux = surfaceLL.atmosphere_flux + surfaceHL.atmosphere_flux;
    annualflux_sumHL = annualflux_sumHL + surfaceHL.atmosphere_flux;
    annualflux_sumLL = annualflux_sumLL + surfaceLL.atmosphere_flux;
    annualflux_sum = annualflux_sum + lastflux;

    // lastflux_annualized is our basis of comparison for variable timestep
    lastflux_annualized = lastflux / yearfraction;

    H_LOG( logger, Logger::DEBUG ) << "lastflux_annualized=" << lastflux_annualized << std::endl;
    H_LOG( logger, Logger::DEBUG ) << "annualflux_sum=" << annualflux_sum << std::endl;

    // Log the state of all our boxes
    surfaceLL.log_state();
    surfaceHL.log_state();
    inter.log_state();
    deep.log_state();

	// Update box states
	surfaceHL.update_state();
	surfaceLL.update_state();
	inter.update_state();
	deep.update_state();

    // All good! t will be the start of the next timestep, so
    ODEstartdate = t;
   }

//------------------------------------------------------------------------------
// documentation is inherited
void OceanComponent::reset(double time)
{
    // Reset state variables to their values at the reset time
    surfaceHL = surfaceHL_tv.get(time);
    surfaceLL = surfaceLL_tv.get(time);
    inter = inter_tv.get(time);
    deep = deep_tv.get(time);

    SST = SST_ts.get(time);
    CO2_conc = Ca_ts.get(time);

    annualflux_sum = annualflux_sum_ts.get(time);
    annualflux_sumHL = annualflux_sumHL_ts.get(time);
    annualflux_sumLL = annualflux_sumLL_ts.get(time);
    lastflux_annualized = lastflux_annualized_ts.get(time);

    max_timestep = max_timestep_ts.get(time);
    reduced_timestep_timeout = reduced_timestep_timeout_ts.get(time);
    timesteps = 0;

    // truncate all the time series beyond the reset time
    surfaceHL_tv.truncate(time);
    surfaceLL_tv.truncate(time);
    inter_tv.truncate(time);
    deep_tv.truncate(time);

    SST_ts.truncate(time);
    Ca_ts.truncate(time);

    annualflux_sum_ts.truncate(time);
    annualflux_sumHL_ts.truncate(time);
    annualflux_sumLL_ts.truncate(time);
    lastflux_annualized_ts.truncate(time);

    max_timestep_ts.truncate(time);
    reduced_timestep_timeout_ts.truncate(time);

    H_LOG(logger, Logger::NOTICE)
        << getComponentName() << " reset to time= " << time << "\n";
}

//------------------------------------------------------------------------------
// documentation is inherited
void OceanComponent::record_state(double time)
{
    H_LOG(logger, Logger::DEBUG) << "Recording component state at t= " << time << endl;
    surfaceHL_tv.set(time, surfaceHL);
    surfaceLL_tv.set(time, surfaceLL);
    inter_tv.set(time, inter);
    deep_tv.set(time, deep);

    // Record the state of the various ocean boxes and variables at each time step
    // in a unitval time series so that the output can be output by the
    // R wrapper.
    SST_ts.set(time, SST );
    Ca_ts.set(time, CO2_conc);
    annualflux_sum_ts.set(time, annualflux_sum);
    annualflux_sumHL_ts.set(time, annualflux_sumHL);
    annualflux_sumLL_ts.set(time, annualflux_sumLL);
    lastflux_annualized_ts.set(time, lastflux_annualized);
    C_IO_ts.set(time, inter.get_carbon());
    Ca_HL_ts.set(time, surfaceHL.get_carbon());
    C_DO_ts.set(time, surfaceHL.annual_box_fluxes[ &deep ]);
    PH_HL_ts.set(time, surfaceHL.mychemistry.pH);
    PH_LL_ts.set(time, surfaceLL.mychemistry.pH);
    pco2_HL_ts.set(time, surfaceHL.mychemistry.PCO2o);
    pco2_LL_ts.set(time, surfaceLL.mychemistry.PCO2o);
    dic_HL_ts.set(time, surfaceHL.mychemistry.convertToDIC( surfaceHL.get_carbon() ));
    dic_LL_ts.set(time, surfaceLL.mychemistry.convertToDIC( surfaceLL.get_carbon() ));
    Ca_LL_ts.set(time, surfaceLL.get_carbon());
    C_DO_ts.set(time, deep.get_carbon());
    temp_HL_ts.set(time, surfaceHL.get_Tbox());
    temp_LL_ts.set(time, surfaceLL.get_Tbox());
    co3_HL_ts.set(time, surfaceHL.mychemistry.CO3);
    co3_LL_ts.set(time, surfaceLL.mychemistry.CO3);

    max_timestep_ts.set(time, max_timestep);
    reduced_timestep_timeout_ts.set(time, reduced_timestep_timeout);
}

//------------------------------------------------------------------------------
// documentation is inherited
void OceanComponent::shutDown() {
	H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void OceanComponent::accept( AVisitor* visitor ) {
    visitor->visit( this );
}

}
