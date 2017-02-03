/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  ocean_component.cpp
 *  hector
 *
 *  Created by Corinne Hartin on 1/3/13.
 *
 */

#include <cmath>
#include <limits>

#include "components/ocean_component.hpp"
#include "core/core.hpp"
#include "h_util.hpp"
#include "models/simpleNbox.hpp"
#include "visitors/avisitor.hpp"

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
    logger.open( getComponentName(), false, Logger::DEBUG );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;
	
    max_timestep = OCEAN_MAX_TIMESTEP;
    reduced_timestep_timeout = 0;
    
	surfaceHL.logger = &logger;
	surfaceLL.logger = &logger;
	inter.logger = &logger;
	deep.logger = &logger;
    
    core = coreptr;
    
	oceanflux_constrain.allowInterp( true );
    oceanflux_constrain.name = "atm_ocean_constrain";
	
    Tgav.set( 0.0, U_DEGC );
	heatflux.set( 0.0, U_W_M2 );
        k_max.set( 0.8, U_W_M2_K );    // default
        t_mid.set( 2.75, U_K );        // default
        k_min.set( 0.0, U_W_M2_K );   // default
        slope.set( -3.0, U_1_K );      // default
        
        m_min_k_so_far = std::numeric_limits<double>::max();

	lastflux_annualized.set( 0.0, U_PGC );
        
    // Register the data we can provide
    core->registerCapability( D_OCEAN_CFLUX, getComponentName() );
	core->registerCapability( D_OCEAN_C, getComponentName() );
    core->registerCapability( D_HEAT_FLUX, getComponentName() );
    core->registerCapability( D_CARBON_HL, getComponentName() );
    core->registerCapability( D_CARBON_LL, getComponentName() );
    core->registerCapability( D_CARBON_IO, getComponentName() );
    core->registerCapability( D_CARBON_DO, getComponentName() );
    core->registerCapability( D_TT, getComponentName() );
    core->registerCapability( D_TU, getComponentName() );
    core->registerCapability( D_TWI, getComponentName() );
    core->registerCapability( D_TID, getComponentName() );
    core->registerCapability( D_HEAT_UPTAKE_EFF, getComponentName() );
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval OceanComponent::sendMessage( const std::string& message,
                                    const std::string& datum,
                                    const message_data info ) throw ( h_exception )
{
    unitval returnval;
    
    if( message == M_GETDATA ) {          //! Caller is requesting data
        return getData( datum, info.date );
        
    } else if( message == M_SETDATA ) {   //! Caller is requesting to set data
        H_THROW("Ocean sendMessage not yet implemented for message=M_SETDATA.");
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
    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << data.date << "]=" << data.value_str << std::endl;
    
    try {
        if( varName == D_CARBON_HL ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            surfaceHL.set_carbon( data.getUnitval( U_PGC ) );
          //} else if( varName == D_CIRC_TOPT ) {
          //  H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
          // circ_Topt = data.getUnitval( U_DEGC );
		//} else if( varName == D_CIRC_T50_HIGH ) {
          //  H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
           // circ_T50_high = data.getUnitval( U_DEGC );
		//} else if( varName == D_CIRC_T50_LOW ) {
          //  H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            //circ_T50_low = data.getUnitval( U_DEGC );
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
        } else if( varName == D_ATM_OCEAN_CONSTRAIN ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            oceanflux_constrain.set( data.date, data.getUnitval( U_PGC_YR ) );
            // Heat uptake-related parameters
        } else if( varName == D_MAX_HEAT_UPTAKE_EFF ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            k_max.set( data.getUnitval(U_W_M2_K), U_W_M2_K );
        } else if( varName == D_KAPPA50_TEMP ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            t_mid.set( data.getUnitval(U_K), U_K );
        } else if( varName == D_MIN_HEAT_UPTAKE_EFF ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            k_min.set( data.getUnitval(U_W_M2_K), U_W_M2_K );
        } else if( varName == D_SLOPE_HEAT_UPTAKE_EFF ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            slope.set( data.getUnitval(U_1_K), U_1_K );
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
// TO DO: should we put these in the ini file instead?
void OceanComponent::prepareToRun() throw ( h_exception ) {
    
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
    
    // Set up our ocean box model. Carbon values here can be overridden by user input
    H_LOG( logger, Logger::DEBUG ) << "Setting up ocean box model" << std::endl;
    surfaceHL.initbox( unitval( 140, U_PGC ), "HL" );
//  surfaceHL.warmingfactor = 1.5;    // HL box warm 50% faster than world
    surfaceHL.surfacebox = true;
    surfaceHL.preindustrial_flux.set( 1.000, U_PGC_YR );         // used if no spinup chemistry
	surfaceHL.active_chemistry = spinup_chem;
    
	surfaceLL.initbox( unitval( 770, U_PGC ),  "LL" );
    surfaceLL.surfacebox = true;
    surfaceLL.preindustrial_flux.set( -1.000, U_PGC_YR );        // used if no spinup chemistry
	surfaceLL.active_chemistry = spinup_chem;
    
	inter.initbox( unitval( 8400, U_PGC ),  "intermediate" );
	deep.initbox( unitval( 26000, U_PGC ),  "deep" );
    
	double time = 60*60*24*365.25;  // seconds per year
    	
	// ocean_volume = 1.36e18 m3
	double thick_LL = 100;
	double thick_HL = 100;
	double thick_inter = 1000-thick_LL;
	double thick_deep = 3777-thick_inter-thick_LL; // 3777m - 1000m - 100m
    
    //	const double ocean_sarea = 5.101e14; // surface area m2
	const double ocean_area = 3.6e14; // m2;
	const double part_high = 0.15;
	const double part_low = 1-part_high;
	const double LL_volume = ocean_area * part_low * thick_LL;
	const double HL_volume = ocean_area * part_high * thick_HL;
	const double I_volume = ocean_area* thick_inter;
	const double D_volume = ocean_area* thick_deep;
	
    // transport * seconds / volume of box
	// Advection --> transport of carbon from one box to the next (k values, fraction/yr )
	double LL_HL = ( tt.value( U_M3_S ) * time ) / LL_volume;
	double HL_DO = ( ( tt + tu).value( U_M3_S ) * time ) / HL_volume;
	double DO_IO = ( ( tt + tu).value( U_M3_S ) * time ) / D_volume;
	double IO_HL = ( tu.value( U_M3_S) * time )  / I_volume;
	double IO_LL = ( tt.value( U_M3_S) * time )  / I_volume;
    
    // Exchange parameters --> not explicitly modeling diffusion
	double IO_LLex = ( twi.value( U_M3_S) *time ) / I_volume;
	double LL_IOex = ( twi.value( U_M3_S) * time ) / LL_volume;
	double DO_IOex = ( tid.value( U_M3_S) *time ) / D_volume;
	double IO_DOex = ( tid.value( U_M3_S) * time ) / I_volume;
    
    // make_connection( box to connect to, k value, window size (0=present only) )
	surfaceLL.make_connection( &surfaceHL, LL_HL, 1 );
	surfaceLL.make_connection( &inter, LL_IOex, 1 );
	surfaceHL.make_connection( &deep, HL_DO, 1 );
	inter.make_connection( &surfaceLL, IO_LL + IO_LLex, 1 );
	inter.make_connection( &surfaceHL, IO_HL, 1 );
	inter.make_connection( &deep, IO_DOex, 1 );
	deep.make_connection( &inter, DO_IO + DO_IOex, 1 );
	
	//inputs for surface chemistry boxes
	//surfaceHL.mychemistry.alk = // mol/kg
        surfaceHL.deltaT.set( -13.0, U_DEGC );  // delta T is added 288.15 to return the initial temperature value of the surface box
	surfaceHL.mychemistry.S             = 34.5; // Salinity
	surfaceHL.mychemistry.volumeofbox   = HL_volume; //5.4e15; //m3
	surfaceHL.mychemistry.As            = ocean_area * part_high ; // surface area m2
	surfaceHL.mychemistry.U             = 6.7; // average wind speed m/s
		
	//surfaceLL.mychemistry.alk = // mol/kg
        surfaceLL.deltaT.set( 7.0, U_DEGC );    // delta T is added to 288.15 to return the initial temperature value of the surface box
	surfaceLL.mychemistry.S             = 34.5; // Salinity
	surfaceLL.mychemistry.volumeofbox   = LL_volume; //3.06e16; //m3
	surfaceLL.mychemistry.As            = ocean_area * part_low; // surface area m2
	surfaceLL.mychemistry.U             = 6.7; // average wind speed m/s
    
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
/*! \brief                  Internal function to calculate atmosphere-ocean C flux
 *  \param[in] date         double, date of calculation (in case constraint used)
 *  \param[in] Ca           unitval, atmospheric CO2
 *  \param[in] cpoolscale   double, how much to scale surface C pools by
 *  \returns                unitval, annual atmosphere-ocean C flux
 */
unitval OceanComponent::annual_totalcflux( const double date, const unitval& Ca, const double cpoolscale ) const {
	
    unitval flux( 0.0, U_PGC_YR );
    
    if( in_spinup && !spinup_chem ) {
        flux = surfaceHL.preindustrial_flux + surfaceLL.preindustrial_flux;
    } else {
        flux = surfaceHL.mychemistry.calc_annual_surface_flux( Ca, cpoolscale )
                            + surfaceLL.mychemistry.calc_annual_surface_flux( Ca, cpoolscale );
    }

        if( !in_spinup && oceanflux_constrain.size() && date <= oceanflux_constrain.lastdate() ) {
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
	annualflux_sum.set( 0.0, U_PGC );
	annualflux_sumHL.set( 0.0, U_PGC );
	annualflux_sumLL.set( 0.0, U_PGC );
    timesteps = 0;
    
    // Initialize ocean box boundary conditions and inform them new year starting
    surfaceHL.new_year( Tgav );
    surfaceLL.new_year( Tgav );
    inter.new_year( Tgav );
    deep.new_year( Tgav );
    
    H_LOG( logger, Logger::DEBUG ) << "----------------------------------------------------" << std::endl;
    H_LOG( logger, Logger::DEBUG ) << "runToDate=" << runToDate << ", spinup=" << in_spinup << std::endl;
   H_LOG( logger, Logger::DEBUG ) << "runToDate=" << runToDate << ", Ca=" << Ca << ", spinup=" << in_spinup << std::endl;
    
    // If chemistry models weren't turned on during spinup, do so now
    if( !spinup_chem && !in_spinup && !surfaceHL.active_chemistry ) {
        H_LOG( logger, Logger::DEBUG ) << "*** Turning on chemistry models ***" << std::endl;
        surfaceHL.active_chemistry = true;
        surfaceLL.active_chemistry = true;
        surfaceHL.chem_equilibrate( Ca );
        surfaceLL.chem_equilibrate( Ca );

        // Warn if the user has supplied an atmosphere-ocean C flux constraint
        if( oceanflux_constrain.size() ) {
            unitval constrained_flux = oceanflux_constrain.get( runToDate );
            H_LOG( logger, Logger::WARNING ) << "Atm-ocean C fluxes will be constrained to " << constrained_flux << std::endl;
        }
   }

    // Call compute_fluxes with do_boxfluxes=false to run just chemistry
	surfaceHL.compute_fluxes( Ca, 1.0, false );
	surfaceLL.compute_fluxes( Ca, 1.0, false );
        
    calcHeatflux( runToDate );
    
    // Now wait for the solver to call us
}

//------------------------------------------------------------------------------
    /*! \brief Calculate ocean heat flux
     */
    void OceanComponent::calcHeatflux( const double runToDate ) {
        // k (kappa) is the ocean heat uptake efficiency, after Raper et al. 2002
        
        if( in_spinup ) {
            heatflux.set( 0.0, U_W_M2 );  // during spinup ocean is neutral
        } else {
            unitval tgaveq = core->sendMessage( M_GETDATA, D_GLOBAL_TEMPEQ );
            
            /* First assumption: the ocean's heat uptake efficiency `k` (kappa) is a function
            // of global temperature and declines following a sigmoid model:
            //   ^
            //   |   ____
            //   |       \
            //   k        \____
            //   |
            //    -- Tgav -------->
            // In this model `kmax` is the upper asymptote (maximum k heat uptake), `sl` a
            // slope parameter (here negative -> declining), tmid is the midpoint of the
            // transition, and `kmin` the minimum heat uptake.
            */
            const double t = Tgav.value( U_DEGC );
            const double sl = slope.value( U_1_K );
            const double tmid = t_mid.value( U_K );
            const double kmin = k_min.value( U_W_M2_K );               // W/m2/K
            const double kmax = k_max.value( U_W_M2_K );

            double k = ( sl == 0 ?
                        kmax :
                        kmin + ( kmax - kmin ) / ( 1 + exp( sl * ( t - tmid ) ) ) );  // W/m2/K
            
            H_ASSERT( k >= kmin && k <= kmax, "kappa out of range");
            
            // Second assumption: there's a 'ratchet' effect, such that once k starts to decline,
            // it's not allowed to come back up. (This would not be true at longer timescales.)
            if( k > m_min_k_so_far ) {
                k = m_min_k_so_far;
            }
            m_min_k_so_far = k;
            
            heatflux.set( k * (tgaveq.value( U_DEGC )), U_W_M2 );
            kappa.set( k, U_W_M2_K );
            
            H_LOG( logger, Logger::DEBUG ) << "heatflux = " << heatflux << ", kappa = " << kappa << std::endl;
        }
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
		returnval = unitval( annualflux_sumHL.value( U_PGC ), U_PGC_YR );
    } else if( varName == D_ATM_OCEAN_FLUX_LL ) {
		returnval = unitval( annualflux_sumLL.value( U_PGC ), U_PGC_YR );
	} else if( varName == D_PCO2_HL ) {
        returnval = surfaceHL.mychemistry.PCO2o;
	} else if( varName == D_PCO2_LL ) {
		returnval = surfaceLL.mychemistry.PCO2o;
    } else if( varName == D_DIC_HL ) {
        returnval = surfaceHL.mychemistry.convertToDIC( surfaceHL.get_carbon() );
	} else if( varName == D_DIC_LL ) {
        returnval = surfaceLL.mychemistry.convertToDIC( surfaceLL.get_carbon() );
	} else if( varName == D_CARBON_HL ) {
        returnval = surfaceHL.get_carbon();
	} else if( varName == D_CARBON_LL ) {
        returnval = surfaceLL.get_carbon();
	} else if( varName == D_CARBON_IO ) {
		returnval = inter.get_carbon();
    } else if( varName == D_CARBON_DO ) {
        returnval = deep.get_carbon();
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
    } else if( varName == D_TEMP_HL ) {
		returnval = surfaceHL.get_Tbox();
	} else if( varName == D_TEMP_LL ) {
		returnval = surfaceLL.get_Tbox();
	} else if( varName == D_CO3_LL ) {
		returnval = surfaceLL.mychemistry.CO3;
	} else if( varName == D_CO3_HL ) {
		returnval = surfaceHL.mychemistry.CO3;
    } else if( varName == D_HEAT_UPTAKE_EFF ) {
            returnval =  kappa;
    } else if( varName == D_TIMESTEPS ) {
        returnval = unitval( timesteps, U_UNITLESS );
    } else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
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
 *
     *  \details "This function should store the vector elements f_i(t,y,params)
 *  in the array dydt, for arguments (t,y) and parameters params." -GSL docs
 *  Compute the air-ocean flux (Pg C/yr) at time t and for pools c[]
 */
int  OceanComponent::calcderivs( double t, const double c[], double dcdt[] ) const {

    const double yearfraction = ( t - ODEstartdate );

    // If the solver has adjusted the ocean and/or atmosphere pools,
    // need to be take into account in the flux computation
    const unitval cpooldiff = unitval( c[ SNBOX_OCEAN ], U_PGC ) - totalcpool();
    const unitval surfacepools = surfaceLL.get_carbon() + surfaceHL. get_carbon();
    const double cpoolscale = ( surfacepools + cpooldiff ) / surfacepools;
    unitval Ca( c[ SNBOX_ATMOS ] * PGC_TO_PPMVCO2, U_PPMV_CO2 );
    
    const double cflux = annual_totalcflux( t, Ca, cpoolscale ).value( U_PGC_YR );
    dcdt[ SNBOX_OCEAN ] = cflux;
    
//    std::cout << t << " cflux=" << cflux << " lastflux_annualized=" << lastflux_annualized.value( U_PGC ) << " Ca=" << Ca << std::endl;

    // If too big a timestep--i.e., stashCvalues below has signalled a reduced step
    // that we're exceeding--signal to the solver that this won't work for us.
    if( yearfraction > max_timestep ) {
//        std::cout << "Exceeded max_timestep of " << max_timestep << " (timeout=" << reduced_timestep_timeout << ")" << std::endl;
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
// documentation is inherited
void OceanComponent::stashCValues( double t, const double c[] ) {

    H_LOG( logger,Logger::DEBUG ) << "Stashing at t=" << t << ", model pools at " << t << ": " << c[ 0 ] << " "
    << c[ 1 ] << " " << c[ 2 ] << " " << c[ 3 ] << " " << c[ 4 ] << " " << c[ 5 ] << std::endl;
//    std::cout << "Stashing at t=" << t << ", model pools at " << t << ": " << c[ 0 ] << " "
//    << c[ 1 ] << " " << c[ 2 ] << " " << c[ 3 ] << " " << c[ 4 ] << " " << c[ 5 ] << std::endl;

	// At this point the solver has converged, going from ODEstartdate to t
    // Now we finalize calculations: circulate ocean, update carbon states, heat flux, etc.
    const double yearfraction = ( t - ODEstartdate );
    H_LOG( logger, Logger::NOTICE ) << "Solver has finished. Yearfraction = " << yearfraction << std::endl;
    H_ASSERT( yearfraction >= 0 && yearfraction <= 1, "yearfraction out of bounds" );

    timesteps++;
    const bool in_partial_year = ( t != int( t ) );
    
    unitval Ca( c[ SNBOX_ATMOS ] * PGC_TO_PPMVCO2, U_PPMV_CO2 );
  
    // Compute fluxes between the boxes (advection of carbon)
    surfaceHL.compute_fluxes( Ca, yearfraction );
	surfaceLL.compute_fluxes( Ca, yearfraction );
	inter.compute_fluxes( Ca, yearfraction );
	deep.compute_fluxes( Ca, yearfraction );
    
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
    
    // This (along with carbon-cycle-solver obviously) is the heart of the reduced-timestep code.
    // If carbon flux has exceeded some critical value, need to reduce timestep for the future.
    unitval cflux_annualdiff = solver_flux/yearfraction - lastflux_annualized;
    
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
//            max_timestep = OCEAN_MAX_TIMESTEP;
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
