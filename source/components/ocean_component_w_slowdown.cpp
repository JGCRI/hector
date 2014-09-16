/*
 *  ocean_component.cpp
 *  hector
 *
 *  Created by Corinne Hartin on 1/3/13.
 *
 */

#include <boost/lexical_cast.hpp>
#include <cmath>

#include "core/core.hpp"
#include "core/ocean_component.hpp"
#include "models/simple6box.hpp"

#include "core/avisitor.hpp"
#include "h_util.hpp"


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
    logger.open( getComponentName(), false, Logger::DEBUG );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;
	
    max_timestep = OCEAN_DEFAULT_TIMESTEP;
    reduced_timestep_timeout = 0.0;
    
	surfaceHL.logger = &logger;
	surfaceLL.logger = &logger;
	inter.logger = &logger;
	deep.logger = &logger;
    
    core = coreptr;
    
	oceanflux_constrain.allowInterp( true );
    oceanflux_constrain.name = "atm_ocean_constrain";
	
    Tgav.set( 0.0, U_DEGC );
	heatflux.set( 0.0, U_W_M2 );
	annualflux_sum.set( 0.0, U_PGC );
	lastflux_annualized.set( 0.0, U_PGC );
    
    // Slowdown parameter defaults
    circ_Topt.set( 1.0, U_DEGC );
    circ_T50_high.set( 4.0, U_DEGC );
    circ_T50_low.set( -5.0, U_DEGC );
    ocean_slowdown = 0.0;
    
    // Register the data we can provide
    core->registerCapability( D_OCEAN_CFLUX, getComponentName() );
	core->registerCapability( D_OCEAN_C, getComponentName() );
    core->registerCapability( D_HEAT_FLUX, getComponentName() );
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval OceanComponent::sendMessage( const std::string& message,
                                    const std::string& datum,
                                    const message_data info ) throw ( h_exception )
{
    unitval returnval;
    
    if( message==M_GETDATA ) {          //! Caller is requesting data
        return getData( datum, info.date );
        
    } else if( message==M_SETDATA ) {   //! Caller is requesting to set data
        //TODO: call setData below
        //TODO: change core so that parsing is routed through sendMessage
        //TODO: make setData private
        
	} else if( message == M_DUMP_TO_DEEP_OCEAN ) {
        // info struct holds the amount being dumped/extracted from deep ocean
        unitval carbon = info.value_unitval;
        H_LOG( logger, Logger::DEBUG ) << "Atmosphere dumping " << carbon << " Pg C to deep ocean" << std::endl;
        deep.set_carbon( deep.get_carbon() + carbon );
        
    } else {                        //! We don't handle any other messages
        H_THROW( "Caller sent unknown message: "+message );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OceanComponent::setData( const string& varName,
                              const message_data& data ) throw ( h_exception )
{
    using namespace boost;
    
    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << data.date << "]=" << data.value_str << std::endl;
    
    try {
        if( varName == D_CARBON_HL ) {
            H_ASSERT( data.date == Core::undefinedIndex() );
            surfaceHL.set_carbon( unitval::parse_unitval( data.value_str, data.units_str, U_PGC ) );
		} else if( varName == D_HEAT_UPTAKE_EFF ) {
            H_ASSERT( data.date == Core::undefinedIndex() );
            k = unitval::parse_unitval( data.value_str, data.units_str, U_W_M2_K );
		} else if( varName == D_CIRC_TOPT ) {
            H_ASSERT( data.date == Core::undefinedIndex() );
            circ_Topt = unitval::parse_unitval( data.value_str, data.units_str, U_DEGC );
		} else if( varName == D_CIRC_T50_HIGH ) {
            H_ASSERT( data.date == Core::undefinedIndex() );
            circ_T50_high = unitval::parse_unitval( data.value_str, data.units_str, U_DEGC );
		} else if( varName == D_CIRC_T50_LOW ) {
            H_ASSERT( data.date == Core::undefinedIndex() );
            circ_T50_low = unitval::parse_unitval( data.value_str, data.units_str, U_DEGC );
		} else if( varName == D_CARBON_LL ) {
            H_ASSERT( data.date == Core::undefinedIndex() );
            surfaceLL.set_carbon( unitval::parse_unitval( data.value_str, data.units_str, U_PGC ) );
        } else if( varName == D_CARBON_IO ) {
            H_ASSERT( data.date == Core::undefinedIndex() );
            inter.set_carbon( unitval::parse_unitval( data.value_str, data.units_str, U_PGC ) );
        } else if( varName == D_CARBON_DO ) {
            H_ASSERT( data.date == Core::undefinedIndex() );
            deep.set_carbon( unitval::parse_unitval( data.value_str, data.units_str, U_PGC ) );
		} else if( varName == D_SPINUP_CHEM ) {
            H_ASSERT( data.date == Core::undefinedIndex() );
            spinup_chem = lexical_cast<bool>( data.value_str );
        } else if( varName == D_ATM_OCEAN_CONSTRAIN ) {
            H_ASSERT( data.date != Core::undefinedIndex() );
            oceanflux_constrain.set( data.date, unitval::parse_unitval( data.value_str, data.units_str, U_PGC_YR ) );
            
        } else {
            H_THROW( "Unknown variable name while parsing " + getComponentName() + ": "
                    + varName );
        }
    } catch( bad_lexical_cast& castException ) {
        H_THROW( "Could not convert var: "+varName+", value: " + data.value_str + ", exception: "
                +castException.what() );
    } catch( h_exception& parseException ) {
        H_RETHROW( parseException, "Could not parse var: "+varName );
    }
}

//------------------------------------------------------------------------------
// documentation is inherited
// TO DO: should we put these in the ini file instead?
void OceanComponent::prepareToRun() throw ( h_exception ) {
    
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
    
    // Set up our ocean box model. Carbon values here can be overridden by user input
    H_LOG( logger, Logger::DEBUG ) << "Setting up ocean box model" << std::endl;
    surfaceHL.initbox( unitval( 140, U_PGC ), "HL" );
    surfaceHL.surfacebox = true;
    surfaceHL.preindustrial_flux.set( 1.000, U_PGC_YR );         // used if no spinup chemistry
	surfaceHL.active_chemistry = spinup_chem;
    
	surfaceLL.initbox( unitval( 770, U_PGC ),  "LL" );
    surfaceLL.surfacebox = true;
    surfaceLL.preindustrial_flux.set( -1.000, U_PGC_YR );        // used if no spinup chemistry
	surfaceLL.active_chemistry = spinup_chem;
    
	inter.initbox( unitval( 8400, U_PGC ),  "intermediate" );
	deep.initbox( unitval( 26000, U_PGC ),  "deep" );
    
	// transport values between boxes
	double tt = 6.4e7;              //6.40e7; //6.59e7; // m3/s thermohaline overtuning
	double tu = 4.87e7;             //5.30e7; //4.87e7; // m3/s high latitude overturning
	double const twi = 1.25e7;      // m3/s warm-intermediate exchange
	double const tid = 2.0e8;       // m3/s intermediate-deep exchange
	double time = 60*60*24*365.25;  // seconds per year
    
	// transport * seconds / volume of box
	// ocean_volume = 1.36e18 m3
	double thick_LL = 100;
	double thick_HL = 100;
	double thick_inter = 1000-thick_LL;
	double thick_deep = 3777-thick_inter-thick_LL; // 3777-1000-100
    
    //	const double ocean_sarea = 5.101e14; // surface area m2
	const double ocean_area = 3.6e14; // m2;
	const double part_high = 0.15;
	const double part_low = 1-part_high;
	const double LL_volume = ocean_area * part_low * thick_LL;
	const double HL_volume = ocean_area * part_high * thick_HL;
	const double I_volume = ocean_area* thick_inter;
	const double D_volume = ocean_area* thick_deep;
	
	// Advection - transport of carbon from one box to the next (k values, fraction/yr )
	double LL_HL = ( tt * time ) / LL_volume;
	double HL_DO = ( ( tt + tu ) * time ) / HL_volume;
	double DO_IO = ( ( tt + tu ) * time ) / D_volume;
	double IO_HL = ( tu *time )  / I_volume;
	double IO_LL = ( tt *time )  / I_volume;
    
    // Exchange parameters --> not explicitly modeling diffusion
	double IO_LLex = ( twi *time ) / I_volume;
	double LL_IOex = ( twi * time ) / LL_volume;
	double DO_IOex = ( tid *time ) / D_volume;
	double IO_DOex = ( tid * time ) / I_volume;
    
    // make_connection( box to connect to, k value, window size (0=present only) )
	surfaceLL.make_connection( &surfaceHL, LL_HL, 0 );
	surfaceLL.make_connection( &inter, LL_IOex, 0 );
	surfaceHL.make_connection( &deep, HL_DO, 0 );
	inter.make_connection( &surfaceLL, IO_LL + IO_LLex, 0 );
	inter.make_connection( &surfaceHL, IO_HL, 0 );
	inter.make_connection( &deep, IO_DOex, 0 );
	deep.make_connection( &inter, DO_IO + DO_IOex, 0 );
	
	//inputs for surface chemistry boxes
	//surfaceHL.mychemistry.alk = 2320.0*1e-6;//2335.62 * 1e-6; // units... mol/kg
	surfaceHL.deltaT.set( -13.5 * (1-ocean_slowdown)  , U_DEGC );  // 13.5 delta T is added 288.15 to return the initial temperature value of the surface box 
	surfaceHL.mychemistry.S             = 34.5; // Salinity
	surfaceHL.mychemistry.volumeofbox   = HL_volume; //5.4e15; //m3
	surfaceHL.mychemistry.As            = ocean_area * part_high ; // surface area m2
	surfaceHL.mychemistry.U             = 8.06; // average wind speed m/s
	// taken from Takahashi et al, 2009.
	// From 197902005 NCEP-DOE AMIP-II reanalysis 6-h wind speed data (Kanamitsu et al, 2002)
	
	//surfaceLL.mychemistry.alk = 2400.0*1e-6;//2460.98 * 1e-6; // units... mol/kg
	surfaceLL.deltaT.set( 6.0  * (1-ocean_slowdown), U_DEGC );    // delta T is added to 288.15 to return the initial temperature value of the surface box
	surfaceLL.mychemistry.S             = 34.5; // Salinity
	surfaceLL.mychemistry.volumeofbox   = LL_volume; //3.06e16; //m3
	surfaceLL.mychemistry.As            = ocean_area * part_low; // surface area m2
	surfaceLL.mychemistry.U             = 8.06; // average wind speed m/s
    
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
unitval OceanComponent::totalcpool() const {
	return deep.get_carbon() + inter.get_carbon() + surfaceLL.get_carbon() + surfaceHL.get_carbon();
}

//------------------------------------------------------------------------------
/*! \brief              Internal function to calculate atmosphere-ocean C flux
 *  \param[in] date     double, date of calculation (in case constraint used)
 *  \param[in] Ca       unitval, atmospheric CO2
 *  \param[in] cpoolscale       double, how much to scale surface C pools by
 *  \returns            unitval, annual atmosphere-ocean C flux
 */
unitval OceanComponent::annual_totalcflux( const double date, const unitval& Ca, const double cpoolscale ) const {
	
    unitval flux( 0.0, U_PGC_YR );
    
    if( in_spinup && !spinup_chem ) {
        flux = surfaceHL.preindustrial_flux + surfaceLL.preindustrial_flux;
    } else {
        flux = surfaceHL.mychemistry.calc_annual_surface_flux( Ca, cpoolscale )
                            + surfaceLL.mychemistry.calc_annual_surface_flux( Ca, cpoolscale );        
    }

    if( !in_spinup && oceanflux_constrain.size() ) {
        flux = oceanflux_constrain.get( date );
    }

    return flux;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OceanComponent::run( const double runToDate ) throw ( h_exception ) {

    Ca = core->sendMessage( M_GETDATA, D_ATMOSPHERIC_CO2 );
    Tgav = core->sendMessage( M_GETDATA, D_GLOBAL_TEMP );
    in_spinup = core->inSpinup();
    in_partial_year = false;
	annualflux_sum.set( 0.0, U_PGC );
    
    // Initialize ocean box boundary conditions and inform them new year starting
    surfaceHL.new_year( Tgav );
    surfaceLL.new_year( Tgav );
    inter.new_year( Tgav );
    deep.new_year( Tgav );
    
    H_LOG( logger, Logger::DEBUG ) << "----------------------------------------------------" << std::endl;
    H_LOG( logger, Logger::DEBUG ) << "runToDate=" << runToDate << ", spinup=" << in_spinup << std::endl;
    
    // If chemistry models weren't turned on during spinup, do so now
    if( !spinup_chem && !in_spinup && !surfaceHL.active_chemistry ) {
        H_LOG( logger, Logger::DEBUG ) << "*** Turning on chemistry models ***" << std::endl;
        surfaceHL.active_chemistry = true;
        surfaceLL.active_chemistry = true;
        surfaceHL.chem_equilibrate();
        surfaceLL.chem_equilibrate();

        // Warn if the user has supplied an atmosphere-ocean C flux constraint
        if( oceanflux_constrain.size() ) {
            unitval constrained_flux = oceanflux_constrain.get( runToDate );
            H_LOG( logger, Logger::WARNING ) << "Atm-ocean C fluxes will be constrained to " << constrained_flux << std::endl;
        }
   }

    // Call compute_fluxes with do_boxfluxes=false to run just chemistry
	surfaceHL.compute_fluxes( Ca, 1.0, false );
	surfaceLL.compute_fluxes( Ca, 1.0, false );
    
    // Now wait for the solver to call us
}

//------------------------------------------------------------------------------
// documentation is inherited
bool OceanComponent::run_spinup( const int step ) throw ( h_exception ) {
    run( step );
    return true;        // solver will be the one signalling
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval OceanComponent::getData( const std::string& varName,
                                const double date ) throw ( h_exception ) {
    
    unitval returnval;
    
    H_ASSERT( date == Core::undefinedIndex(), "Date data not available for ocean_component" );
    
    if( varName == D_OCEAN_CFLUX ) {
        returnval = unitval( annualflux_sum.value( U_PGC ), U_PGC_YR );
    } else if( varName == D_OCEAN_C ) {
        returnval = totalcpool();
	} else if( varName == D_HL_DO ) {
        returnval = surfaceHL.annual_box_fluxes[ &deep ] ;
	} else if( varName == D_HEAT_FLUX ) {
		returnval = heatflux;
    } else if( varName == D_PH_HL ) {
        returnval = surfaceHL.mychemistry.pH;
	} else if( varName == D_PH_LL ) {
        returnval = surfaceLL.mychemistry.pH;
	} else if( varName == D_ATM_OCEAN_FLUX_HL ) {
		returnval = unitval( surfaceHL.annual_atmosphere_flux.value( U_PGC ), U_PGC_YR );
    } else if( varName == D_ATM_OCEAN_FLUX_LL ) {
		returnval = unitval( surfaceLL.annual_atmosphere_flux.value( U_PGC ), U_PGC_YR );
	} else if( varName == D_PCO2_HL ) {
        returnval = surfaceHL.mychemistry.PCO2o;
	} else if( varName == D_PCO2_LL ) {
		returnval = surfaceLL.mychemistry.PCO2o;
    } else if( varName == D_DIC_HL ) {
        returnval = surfaceHL.mychemistry.DIC_output;
	} else if( varName == D_DIC_LL ) {
        returnval = surfaceLL.mychemistry.DIC_output;
	} else if( varName == D_P0_LL ) {
		returnval = surfaceLL.mychemistry.get_p0();
    } else if( varName == D_CARBON_HL ) {
        returnval = surfaceHL.get_carbon();
	} else if( varName == D_CARBON_LL ) {
        returnval = surfaceLL.get_carbon();
	} else if( varName == D_CARBON_IO ) {
		returnval = inter.get_carbon();
    } else if( varName == D_CARBON_DO ) {
        returnval = deep.get_carbon();
	} else if( varName == D_OMEGACA_HL ) {
        returnval = surfaceHL.mychemistry.OmegaCa;
	} else if( varName == D_OMEGACA_LL ) {
		returnval = surfaceLL.mychemistry.OmegaCa;
	} else if( varName == D_OMEGAAR_HL ) {
        returnval = surfaceHL.mychemistry.OmegaAr;
	} else if( varName == D_OMEGAAR_LL ) {
        returnval = surfaceLL.mychemistry.OmegaAr;
	} else if( varName == D_TEMP_HL ) {
		returnval = surfaceHL.mychemistry.T;
	} else if( varName == D_TEMP_LL ) {
		returnval = surfaceLL.mychemistry.T;
	} else if( varName == D_CO3_LL ) {
		returnval = surfaceLL.mychemistry.CO3;
	} else if( varName == D_CO3_HL ) {
		returnval = surfaceHL.mychemistry.CO3;
    } else if( varName == D_HEAT_UPTAKE_EFF ) {
        returnval = k;
    } else if( varName == D_OCEAN_SLOWDOWN ) {
        returnval = unitval( ocean_slowdown, U_UNITLESS );
    } else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void OceanComponent::getCValues( double t, double c[] ) {
    c[ S6B_N_OCEAN ] = totalcpool().value( U_PGC );
    
    ODEstartdate = t;
}

//------------------------------------------------------------------------------
/*! \brief              Compute atmosphere-ocean flux for a time step
 *  \param[in]  t       time
 *  \param[in]  c       carbon pools (no units)
 *  \param[out] dcdt    carbon deltas - we fill in only ocean change
 *  \returns            code indicating success or failure
 *
 *  \details "This function should store the vector elements f_i(t,y,params) 
 *  in the array dydt, for arguments (t,y) and parameters params." -GSL docs
 *  Compute the air-ocean flux (Pg C/yr) at time t and for pools c[]
 */
int  OceanComponent::calcderivs( double t, const double c[], double dcdt[] ) const {

    const double yearfraction = ( t - ODEstartdate );

    // If the solver has adjusted the ocean and/or atmosphere pools,
    // need to be take into account in the flux computation
    const unitval cpooldiff = unitval( c[ S6B_N_OCEAN ], U_PGC ) - totalcpool();
    const unitval surfacepools = surfaceLL.get_carbon() + surfaceHL. get_carbon();
    const double cpoolscale = ( surfacepools + cpooldiff ) / surfacepools;
    unitval Ca( c[ S6B_N_ATMOS ] * PGC_TO_PPMVCO2, U_PPMV_CO2 );
    
    const double cflux = annual_totalcflux( t, Ca, cpoolscale ).value( U_PGC_YR );
    dcdt[ S6B_N_OCEAN ] = cflux;
    
    std::cout << t << " cflux=" << cflux << " lastflux_annualized=" << lastflux_annualized.value( U_PGC ) << " Ca=" << Ca << std::endl;

    // If too big a timestep--i.e., stashCvalues below has signalled a reduced step
    // that we're exceeding--signal to the solver that this won't work for us.
    if( yearfraction > max_timestep ) {
        std::cout << "Exceeded max_timestep of " << max_timestep << " (timeout=" << reduced_timestep_timeout << ")" << std::endl;
        return CARBON_CYCLE_RETRY;
    } else {
        return GSL_SUCCESS;
    }
}

//------------------------------------------------------------------------------
// documentation is inherited
void OceanComponent::slowparameval( double t, const double c[] ) {

    in_spinup = core->inSpinup();
}

//------------------------------------------------------------------------------
// documentation is inherited
void OceanComponent::stashCValues( double t, const double c[] ) {

    H_LOG( logger,Logger::DEBUG ) << "Stashing at t=" << t << ", model pools at " << t << ": " << c[ 0 ] << " "
    << c[ 1 ] << " " << c[ 2 ] << " " << c[ 3 ] << " " << c[ 4 ] << " " << c[ 5 ] << std::endl;
    std::cout << "Stashing at t=" << t << ", model pools at " << t << ": " << c[ 0 ] << " "
    << c[ 1 ] << " " << c[ 2 ] << " " << c[ 3 ] << " " << c[ 4 ] << " " << c[ 5 ] << std::endl;

	// At this point the solver has converged, going from ODEstartdate to t
    // Now we finalize calculations: circulate ocean, update carbon states, heat flux, etc.
    const double yearfraction = ( t - ODEstartdate );
    H_LOG( logger, Logger::NOTICE ) << "Solver has finished. Yearfraction = " << yearfraction << std::endl;
    H_ASSERT( yearfraction >= 0 && yearfraction <= 1, "yearfraction out of bounds" );

    in_partial_year = yearfraction < 1.0;
    
    unitval Ca( c[ S6B_N_ATMOS ] * PGC_TO_PPMVCO2, U_PPMV_CO2 );
    
    // The ocean has on optimal circulation temperature (T_opt, by default 1C)
    // and slows down linearly on either side of this; the 50% slowdown temperatures
    // are defined by circ_T50_low and circ_T50_high.
    /*
    const double max_slowdown = 0.9;
    static double tgav_lastyear = 0.0;
    unitval tgav_smooth( ( Tgav.value( U_DEGC ) + tgav_lastyear ) / 2.0, U_DEGC );
    
    if( tgav_smooth.value( U_DEGC ) > circ_Topt.value( U_DEGC ) ) {
        ocean_slowdown = abs( ( tgav_smooth-circ_Topt ).value( U_DEGC ) ) / abs( ( circ_Topt-circ_T50_high ).value( U_DEGC ) ) / 2.0;
    } else {
        ocean_slowdown = abs( ( tgav_smooth-circ_Topt ).value( U_DEGC ) ) / abs( ( circ_Topt-circ_T50_low ).value( U_DEGC ) ) / 2.0;
    }
    ocean_slowdown = min( max_slowdown, ocean_slowdown );   // not allowed to drop below max_slowdown
    
    tgav_lastyear = Tgav.value( U_DEGC );
     */

    surfaceHL.slowdown = ocean_slowdown;
    surfaceLL.slowdown = ocean_slowdown;
    inter.slowdown = ocean_slowdown;
    deep.slowdown = ocean_slowdown;
    
    // Compute fluxes between the boxes (advection of carbon)
//	slowparameval( t, c );
    surfaceHL.compute_fluxes( Ca, yearfraction );
	surfaceLL.compute_fluxes( Ca, yearfraction );
	inter.compute_fluxes( Ca, yearfraction );
	deep.compute_fluxes( Ca, yearfraction );
    
    // At this point, compute_fluxes has (by calling the chemistry model) computed atmosphere-
    // ocean fluxes for the surface boxes. But these are end-of-timestep values, and we need to
    // overwrite them with what the solver has sent us (~mid-timestep values), so that everything
    // stays consistent.
    unitval currentflux = surfaceHL.atmosphere_flux + surfaceLL.atmosphere_flux;
    unitval solver_flux = unitval( c[ S6B_N_OCEAN ], U_PGC ) - totalcpool();
    unitval adjustment( 0.0, U_PGC );
    if( currentflux.value( U_PGC ) ) adjustment = ( solver_flux - currentflux ) / 2.0;
	H_LOG( logger, Logger::DEBUG) << "Solver flux = " << solver_flux << ", currentflux = " << currentflux << ", adjust = " << adjustment << std::endl;
    surfaceHL.atmosphere_flux = surfaceHL.atmosphere_flux + adjustment;
    surfaceLL.atmosphere_flux = surfaceLL.atmosphere_flux + adjustment;
    
    /* If carbon flux has exceeded some critical value, need to reduce timestep for the future. */
#define SOME_CRITICAL_VALUE 0.5     // Pg C
    unitval cflux_annualdiff = solver_flux/yearfraction - lastflux_annualized;
    if( t>1800 &&   // TODO: this is a hack t - instability at beginning of run
       cflux_annualdiff.value( U_PGC) > SOME_CRITICAL_VALUE ) {
        std::cout << "Reducing timestep: t=" << t << " yearfraction=" << yearfraction;
        std::cout << " solver_flux=" << solver_flux << " lastflux_annualized=" << lastflux_annualized;
        std::cout << " cflux_annualdiff=" << cflux_annualdiff << std::endl;
        max_timestep = max( OCEAN_MIN_TIMESTEP, max_timestep * 0.5 );
        reduced_timestep_timeout = OCEAN_TIMESTEP_TIMEOUT;
    }
   
    // Update lastflux and add it to annual sum
    unitval lastflux = surfaceLL.atmosphere_flux + surfaceHL.atmosphere_flux;
    lastflux_annualized = lastflux / yearfraction;
    annualflux_sum = annualflux_sum + lastflux;
    
    H_LOG( logger, Logger::DEBUG ) << "lastflux_annualized=" << lastflux_annualized << std::endl;
    H_LOG( logger, Logger::DEBUG ) << "annualflux_sum=" << annualflux_sum << std::endl;
 
    std::cout << "lastflux_annualized=" << lastflux_annualized << std::endl;
    std::cout << "annualflux_sum=" << annualflux_sum << std::endl;
    
    printf( "zzz%8.2f%8.3f%8.3f%8.3f%8.3f%8.3f\n", t, lastflux.value( U_PGC ), annualflux_sum.value( U_PGC ), lastflux_annualized.value( U_PGC ), max_timestep, reduced_timestep_timeout );

    // If not a partial year, decrement the timeout counter if we're in a timeout.
    if( !in_partial_year ) {
        std::cout << "END OF YEAR" << std::endl;
        if( reduced_timestep_timeout ) {
            if( --reduced_timestep_timeout==0 ) {
                std::cout << "Reduced timestep timeout done; resetting" << std::endl;
                max_timestep = OCEAN_DEFAULT_TIMESTEP;
            }
        }
    }
    
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
    
    
    //--------------------- Specific Heat Capacity of SeaWater -----------------------------------
    /*! \brief specific heat capacity
     * D. T. Jamieson, J. S. Tudhope, R. Morris, and G. Cartwright,
     * Physical properties of sea water solutions: heat capacity,
     * Desalination, 7(1), 23-30, 1969.
     * units J kg-1 K-1
     
     if (! in_spinup) {       // do not run during spinup
     // this will be the initial temperature of the surface boxes
     unitval tempHL = core->sendMessage( M_GETDATA, D_TEMP_HL );
     unitval tempLL = core->sendMessage( M_GETDATA, D_TEMP_LL );
     //unitval sal = core->sendMessage( M_GETDATA, D_SAL_HL );  TODO: vary salinity
     double sal = 34.5;
         
		 // BEN how can i do this once, so it will pick up both boxes?

     T = 1.00024*temp-273.15;      //convert from T_90 to T_68
     double S = sal / 1.00472;    //convert from S to S_P
     
     double A =  4206.8 - 6.6197*sal + 1.2288E-2*S*S;
     double B = -1.1262 + 5.4178E-2*sal - 2.2719E-4*S*S;
     double C =  1.2026E-2 - 5.3566E-4*sal + 1.8906E-6*S*S;
     double D =  6.8777E-7 + 1.517E-6 *sal - 4.4268E-9*S*S;
     
     cp.set ((A + B*T + C*T*T + D*T*T*T), U_J_KG_C);  //DOUBLE CHECK UNITS!
     
     H_LOG( logger, Logger::DEBUG ) << "specific heat is " << cp << std::endl;
     }
	 */
     
    
    //--------------------- Ocean Heat Flux -----------------------------------
    /*! \brief Ocean heat flux (W/m2)
     * taken from Raper etal 2002
     * deltaF = k * deltaT  
	 * deltaT should be the difference between the boxes (in this case, atmosphere and ocean)
     * k is the ocean heat uptake efficiency (W/m2/K)
     */
	if ( !in_spinup ) {       // do not run during spinup
		//unitval tgav = core->sendMessage( M_GETDATA, D_GLOBAL_TEMP );
		unitval k( 0.2, U_W_M2_K );
		unitval tgaveq = core->sendMessage( M_GETDATA, D_GLOBAL_TEMPEQ );
		//unitval tempHL = core->sendMessage( M_GETDATA, D_TEMP_HL );
        //unitval tempLL = core->sendMessage( M_GETDATA, D_TEMP_LL );
	    //heatflux_HL.set( k.value( U_W_M2_K ) * (tgaveq.value( U_DEGC ) - temp_HL(U_DEGC)) * ( 1.0-ocean_slowdown ), U_W_M2 );
        //heatflux_LL.set( k.value( U_W_M2_K ) * (tgaveq.value( U_DEGC ) - temp_LL(U_DEGC)) * ( 1.0-ocean_slowdown ), U_W_M2 );
		//heatflux.set(( heatflux_HL.value( U_W_M2 ) + heatflux_LL.value( U_W_M2 )), U_W_M2);
		heatflux.set( k.value( U_W_M2_K ) * (tgaveq.value( U_DEGC )) * ( 1.0-ocean_slowdown ), U_W_M2 );

		H_LOG( logger, Logger::DEBUG ) << "heatflux = " << heatflux << ", kappa = " << k << std::endl;
	}
    
    //---------------- Temperature Change from Heat Flux--------------------------
    /*! \brief Ocean temperature change from heat flux
     * Q = density * specific heat * deltaT
     * Q = heat/volume
     * specific heat = cp J/kgC
     * density = 1030 kg/m3
     * Q = heatflux * 1/depth
     

	// Promblem....heatflux is not calculated in each box....
     unitval cp = core->sendMessage( M_GETDATA, D_SPECIFIC_HEAT );
     unitval heatflux = core->sendMessage( M_GETDATA, D_HEAT_FLUX );
     
	 // for each box
     double tmp = (100 * 1030 * cp.value(U_J_KG_C));

     deltaT = heatflux.value(U_W_M2_K) * 3.15e7 / tmp;  
	 // DOUBLE CHECK units
	 // this goes above when we set the surface boxes
     */
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
