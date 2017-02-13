/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  temp_doeclim_component.cpp
 *  hector
 *
 *  Created by Ben Vega-Westhoff on 11/1/16.
 *
 */

#include <boost/lexical_cast.hpp>
#include <cmath>
#include <limits>
#define _USE_MATH_DEFINES

#include "components/temp_doeclim_component.hpp"
#include "core/core.hpp"
#include "h_util.hpp"
#include "models/simpleNbox.hpp"
#include "visitors/avisitor.hpp"

namespace Hector {
  
using namespace std;


//------------------------------------------------------------------------------
/*! \brief Constructor
 */
TempDOECLIMComponent::TempDOECLIMComponent() : internal_Ftot(0.0), last_Ftot(0.0) {
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
TempDOECLIMComponent::~TempDOECLIMComponent(){
}
    
//------------------------------------------------------------------------------
// documentation is inherited
string TempDOECLIMComponent::getComponentName() const {
    const string name = TEMP_DOECLIM_COMPONENT_NAME;
    
    return name;
}


//------------------------------------------------------------------------------
/*! \brief              Calculates inverse of x and stores in y
 *  \param[in] x        Assume x is setup like x = [a,b,c,d] -> x = |a, b|
 *                                                                  |c, d|
 *  \param[in] y        Inverted 1-d matrix
 *  \returns            void, inverse is stored in y
 */
void TempDOECLIMComponent::invert_1d_2x2_matrix(double * x, double * y) {
    double temp_d = (x[0]*x[3] - x[1]*x[2]);
        
    if(temp_d == 0) {
        H_THROW("TempDOECLIM: Matrix inversion divide by zero.");
    }
    double temp = 1/temp_d;
    y[0] = temp * x[3];
    y[1] = temp * -1 * x[1];
    y[2] = temp * -1 * x[2];
    y[3] = temp * x[0];
        
    return;
}
    
//------------------------------------------------------------------------------
/*! \brief              Calculates element-wise sum of matrix x and y, stores in z
 *  \param[in] x,y      Assume x,y setup like x = [a,b,c,d] -> x = |a, b|
 *                                                                 |c, d|
 *  \param[in] z        Summed 1-d matrix
 *  \returns            void, summed matrix is stored in z
 */
void TempDOECLIMComponent::sum_1d_2x2_matrix(double * x, double * y, double * z) {
    for(int i=0; i < 4; i++) {
        z[i] = x[i] + y[i];
    }
        
    return;
}
    
//------------------------------------------------------------------------------
// documentation is inherited
void TempDOECLIMComponent::init( Core* coreptr ) {
    logger.open( getComponentName(), false, Logger::DEBUG );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;
    
    tgaveq.set( 0.0, U_DEGC, 0.0 );
    tgav.set( 0.0, U_DEGC, 0.0 );
    flux_mixed.set( 0.0, U_W_M2, 0.0 );
    flux_interior.set( 0.0, U_W_M2, 0.0 );
    
    core = coreptr;
    
    tgav_constrain.allowInterp( true );
    tgav_constrain.name = D_TGAV_CONSTRAIN;
    
    // Define the doeclim parameters
    diff.set( 0.55, U_CM2_S );  // default ocean heat diffusivity, cm2/s. value is CDICE default (varname is kappa there).
    S.set( 3.0, U_DEGC );         // default climate sensitivity, K (varname is t2co in CDICE).
    alpha.set( 1.0, U_UNITLESS);  // default aerosol scaling, unitless (similar to alpha in CDICE).
    
    // Register the data we can provide
    core->registerCapability( D_GLOBAL_TEMP, getComponentName() );
    core->registerCapability( D_GLOBAL_TEMPEQ, getComponentName() );
    core->registerCapability( D_FLUX_MIXED, getComponentName() );
    core->registerCapability( D_FLUX_INTERIOR, getComponentName() );

    // Register our dependencies
    core->registerDependency( D_RF_TOTAL, getComponentName() );
    core->registerDependency( D_RF_BC, getComponentName() );
    core->registerDependency( D_RF_OC, getComponentName() );
    core->registerDependency( D_RF_SO2d, getComponentName() );
    core->registerDependency( D_RF_SO2i, getComponentName() );

}

//------------------------------------------------------------------------------
// documentation is inherited
unitval TempDOECLIMComponent::sendMessage( const std::string& message,
                                    const std::string& datum,
                                    const message_data info ) throw ( h_exception )
{
    unitval returnval;
    
    if( message==M_GETDATA ) {          //! Caller is requesting data
        return getData( datum, info.date );
        
    } else if( message==M_SETDATA ) {   //! Caller is requesting to set data
        H_THROW("TempDOECLIM: sendMessage not yet implemented for message=M_SETDATA.");
        //TODO: call setData below
        //TODO: change core so that parsing is routed through sendMessage
        //TODO: make setData private

        
    } else {                        //! We don't handle any other messages
        H_THROW( "Caller sent unknown message: "+message );
    }
    
    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void TempDOECLIMComponent::setData( const string& varName,
                              const message_data& data ) throw ( h_exception )
{
    using namespace boost;
    
    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << data.date << "]=" << data.value_str << std::endl;
    
    try {
        if( varName == D_ECS ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            S = unitval::parse_unitval( data.value_str, data.units_str, U_DEGC );
        } else if( varName == D_DIFFUSIVITY ) {
            H_ASSERT( data.date == Core::undefinedIndex(), "date not allowed" );
            diff = unitval::parse_unitval( data.value_str, data.units_str, U_CM2_S );
	} else if( varName == D_AERO_SCALE ) {
            H_ASSERT( data.date == Core::undefinedIndex(), "date not allowed" );
            alpha = unitval::parse_unitval( data.value_str, data.units_str, U_UNITLESS );
        } else if( varName == D_TGAV_CONSTRAIN ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            tgav_constrain.set( data.date, unitval::parse_unitval( data.value_str, data.units_str, U_DEGC ) );
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
void TempDOECLIMComponent::prepareToRun() throw ( h_exception ) {
    
    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;
    
    if( tgav_constrain.size() ) {
        Logger& glog = Logger::getGlobalLogger();
        H_LOG( glog, Logger::WARNING ) << "Temperature will be overwritten by user-supplied values!" << std::endl;
    }
    
    dt = 1;         // years per timestep (this is hard-coded into Hector
    ak = 0.31;      // slope in climate feedback - land-sea heat exchange linear relationship
    bk = 1.59;      // offset in climate feedback - land-sea heat exchange linear relationship, W/m2/K
    csw = 0.13;     // specific heat capacity of seawater W*yr/m3/K
    earth_area = 5100656 * pow(10.0, 8);
    kcon = 3155.0;  // conversion from cm2/s to m2/yr
    q2co = 3.7;     // radiative forcing for atmospheric CO2 doubling, W/m2
    rlam = 1.43;    // factor between land clim. sens. and sea surface clim. sens. T_L2x = rlam*T_S2x
    secs_per_Year = 31556926.0;
    zbot = 4000.0;  // bottom depth of diffusive ocean, m
    bsi = 1.3;                  // warming factor for marine surface air over SST (due to retreating sea ice)
    cal = 0.52;                 // heat capacity of land-troposphere system, W*yr/m2/K
    cas = 7.80;                 // heat capacity of mixed layer-troposphere system, W*yr/m2/K
    flnd = 0.29;                // fractional land area
    fso = 0.95;                 // ocean fractional area below 60m
    
    
    // Initializing all model components that depend on the number of timesteps (ns)
    ns = core->getEndDate() - core->getStartDate() + 1;
    
    KT0.resize(ns);
    KTA1.resize(ns);
    KTB1.resize(ns);
    KTA2.resize(ns);
    KTB2.resize(ns);
    KTA3.resize(ns);
    KTB3.resize(ns);
    
    Ker.resize(ns);
    
    temp.resize(ns);
    temp_landair.resize(ns);
    temp_sst.resize(ns);
    heatflux_mixed.resize(ns);
    heatflux_interior.resize(ns);
    heat_mixed.resize(ns);
    heat_interior.resize(ns);
    forcing.resize(ns);
    
    for(int i=0; i<3; i++) {
        B[i] = 0.0;
        C[i] = 0.0;
    }
    
    for(int i=0; i<ns; i++) {
        KT0[i] = 0.0;
        KTA1[i] = 0.0;
        KTB1[i] = 0.0;
        KTA2[i] = 0.0;
        KTB2[i] = 0.0;
        KTA3[i] = 0.0;
        KTB3[i] = 0.0;
    }
    
    // Dependent Model Parameters
    ocean_area = (1.0 - flnd) * earth_area;
    cnum = rlam * flnd + bsi * (1.0 - flnd);
    cden = rlam * flnd - ak * (rlam - bsi);
    cfl = flnd * cnum / cden * q2co / S - bk * (rlam - bsi) / cden;
    cfs = (rlam * flnd - ak / (1.0 - flnd) * (rlam - bsi)) * cnum / cden * q2co / S + rlam * flnd / (1.0 - flnd) * bk * (rlam - bsi) / cden;
    kls = bk * rlam * flnd / cden - ak * flnd * cnum / cden * q2co / S;
    keff = kcon * alpha;
    taubot = pow(zbot,2) / keff;
    powtoheat = ocean_area * secs_per_Year / pow(10.0,22);
    taucfs = cas / cfs;
    taucfl = cal / cfl;
    taudif = pow(cas,2) / pow(csw,2) * M_PI / keff;
    tauksl  = (1.0 - flnd) * cas / kls;
    taukls  = flnd * cal / kls;
    
    // First order
    KT0[ns-1] = 4.0 - 2.0 * pow(2.0, 0.5);
    KTA1[ns-1] = -8.0 * exp(-taubot / double(dt)) + 4.0 * pow(2.0, 0.5) * exp(-0.5 * taubot / double(dt));
    KTB1[ns-1] = 4.0 * pow((M_PI * taubot / double(dt)), 0.5) * (1.0 + erf(pow(0.5 * taubot / double(dt), 0.5)) - 2.0 * erf(pow(taubot / double(dt), 0.5)));
    
    // Second order
    KTA2[ns-1] =  8.0 * exp(-4.0 * taubot / double(dt)) - 4.0 * pow(2.0, 0.5) * exp(-2.0 * taubot / double(dt));
    KTB2[ns-1] = -8.0 * pow((M_PI * taubot / double(dt)), 0.5) * (1.0 + erf(pow((2.0 * taubot / double(dt)), 0.5)) - 2.0 * erf(2.0 * pow((taubot / double(dt)), 0.5)) );
    
    // Third order
    KTA3[ns-1] = -8.0 * exp(-9.0 * taubot / double(dt)) + 4.0 * pow(2.0, 0.5) * exp(-4.5 * taubot / double(dt));
    KTB3[ns-1] = 12.0 * pow((M_PI * taubot / double(dt)), 0.5) * (1.0 + erf(pow((4.5 * taubot / double(dt)), 0.5)) - 2.0 * erf(3.0 * pow((taubot / double(dt)), 0.5)) );
    
    // Calculate the kernel component vectors
    for(int i=0; i<(ns-1); i++) {
        
        // First order
        KT0[i] = 4.0 * pow((double(ns-i)), 0.5) - 2.0 * pow((double(ns+1-i)), 0.5) - 2.0 * pow(double(ns-1-i), 0.5);
        KTA1[i] = -8.0 * pow(double(ns-i), 0.5) * exp(-taubot / double(dt) / double(ns-i)) + 4.0 * pow(double(ns+1-i), 0.5) * exp(-taubot / double(dt) / double(ns+1-i)) + 4.0 * pow(double(ns-1-i), 0.5) * exp(-taubot/double(dt) / double(ns-1-i));
        KTB1[i] =  4.0 * pow((M_PI * taubot / double(dt)), 0.5) * ( erf(pow((taubot / double(dt) / double(ns-1-i)), 0.5)) + erf(pow((taubot / double(dt) / double(ns+1-i)), 0.5)) - 2.0 * erf(pow((taubot / double(dt) / double(ns-i)), 0.5)) );
        
        // Second order
        KTA2[i] =  8.0 * pow(double(ns-i), 0.5) * exp(-4.0 * taubot / double(dt) / double(ns-i)) - 4.0 * pow(double(ns+1-i), 0.5) * exp(-4.0 * taubot / double(dt) / double(ns+1-i)) - 4.0 * pow(double(ns-1-i), 0.5) * exp(-4.0 * taubot / double(dt) / double(ns-1-i));
        KTB2[i] = -8.0 * pow((M_PI * taubot / double(dt)), 0.5) * ( erf(2.0 * pow((taubot / double(dt) / double(ns-1-i)), 0.5)) + erf(2.0 * pow((taubot / double(dt) / double(ns+1-i)), 0.5)) - 2.0 * erf(2.0 * pow((taubot / double(dt) / double(ns-i)), 0.5)) );
        
        // Third order
        KTA3[i] = -8.0 * pow(double(ns-i), 0.5) * exp(-9.0 * taubot / double(dt) / double(ns-i)) + 4.0 * pow(double(ns+1-i), 0.5) * exp(-9.0 * taubot / double(dt) / double(ns+1-i)) + 4.0 * pow(double(ns-1-i), 0.5) * exp(-9.0 * taubot / double(dt) / double(ns-1-i));
        KTB3[i] = 12.0 * pow((M_PI * taubot / double(dt)), 0.5) * ( erf(3.0 * pow((taubot / double(dt) / double(ns-1-i)), 0.5)) + erf(3.0 * pow((taubot / double(dt) / double(ns+1-i)), 0.5)) - 2.0 * erf(3.0 * pow((taubot / double(dt) / double(ns-i)), 0.5)) );
    }
    
    // Sum up the kernel components
    for(int i=0; i<ns; i++) {
        
        Ker[i] = KT0[i] + KTA1[i] + KTB1[i] + KTA2[i] + KTB2[i] + KTA3[i] + KTB3[i];
        
    }
    
    // Switched on (To switch off, comment out lines below)
    C[0] = 1.0 / pow(taucfl, 2.0) + 1.0 / pow(taukls, 2.0) + 2.0 / taucfl / taukls + bsi / taukls / tauksl;
    C[1] = -1 * bsi / pow(taukls, 2.0) - bsi / taucfl / taukls - bsi / taucfs / taukls - pow(bsi, 2.0) / taukls / tauksl;
    C[2] = -1 * bsi / pow(tauksl, 2.0) - 1.0 / taucfs / tauksl - 1.0 / taucfl / tauksl -1.0 / taukls / tauksl;
    C[3] = 1.0 / pow(taucfs, 2.0) + pow(bsi, 2.0) / pow(tauksl, 2.0) + 2.0 * bsi / taucfs / tauksl + bsi / taukls / tauksl;
    for(int i=0; i<4; i++) {
        C[i] = C[i] * (pow(double(dt), 2.0) / 12.0);
    }
    
    //------------------------------------------------------------------
    // Matrices of difference equation system B*T(i+1) = Q(i) + A*T(i)
    // T = (TL,TO)
    // (Equation A.27, EK05, or Equations 2.3.24 and 2.3.27, TK07)
    B[0] = 1.0 + double(dt) / (2.0 * taucfl) + double(dt) / (2.0 * taukls);
    B[1] = double(-dt) / (2.0 * taukls) * bsi;
    B[2] = double(-dt) / (2.0 * tauksl);
    B[3] = 1.0 + double(dt) / (2.0 * taucfs) + double(dt) / (2.0 * tauksl) * bsi + 2.0 * fso * pow((double(dt) / taudif), 0.5);
    
    A[0] = 1.0 - double(dt) / (2.0 * taucfl) - double(dt) / (2.0 * taukls);
    A[1] = double(dt) / (2.0 * taukls) * bsi;
    A[2] = double(dt) / (2.0 * tauksl);
    A[3] = 1.0 - double(dt) / (2.0 * taucfs) - double(dt) / (2.0 * tauksl) * bsi + Ker[ns-1] * fso * pow((double(dt) / taudif), 0.5);
    for (int i=0; i<4; i++) {
        B[i] = B[i] + C[i];
        A[i] = A[i] + C[i];
    }
    
    // Calculate the inverse of B
    invert_1d_2x2_matrix(B, IB);
}


//------------------------------------------------------------------------------
// documentation is inherited
void TempDOECLIMComponent::run( const double runToDate ) throw ( h_exception ) {
    H_LOG( logger, Logger::DEBUG ) << "DOECLIM run " << runToDate << std::endl;
    
    // Commented section below is for case of user-specified temperature record.
    // TempDOECLIM component can't handle that at the moment!
    
    //// We track total radiative forcing using internal variable `internal_Ftot`
    //// Need to do this because if we're subject to a user constraint (being forced
    //// to match a temperature record), need to track the Ftot that *would* have
    //// produced the observed temperature record. This way there's a smooth
    //// transition when we exit the constraint period, after which internal_Ftot
    //// will rise in parallel with the value reported by ForcingComponent.
    
    // If we never had any temperature constraint, `internal_Ftot` will match `Ftot`.
    //
    // If the user has supplied temperature data, use that (except if past its end)
    if( tgav_constrain.size() && runToDate <= tgav_constrain.lastdate() ) {
    //    H_LOG( logger, Logger::NOTICE ) << "** Using user-supplied temperature" << std::endl;
        H_LOG( logger, Logger::NOTICE ) << "** ERROR - DOECLIM can't currently handle user-supplied temperature" << std::endl;
    }
    //  tgav = tgav_constrain.get( runToDate );
    //    tgaveq = tgav;
    //
    //    // Update last_Ftot and internal_Ftot for when we exit constraint
    //    last_Ftot = core->sendMessage( M_GETDATA, D_RF_TOTAL ).value( U_W_M2 );
    //    const double heatflux = core->sendMessage( M_GETDATA, D_HEAT_FLUX ).value( U_W_M2 );
    //    internal_Ftot = 3.7 * tgav / S + heatflux;  // inverting normal T=S/3.7*F eqn
    //} else {  // not constrained
    //    // Not subject to a constraint. Get total forcing and update our
    //    // internal tracking variables `internal_Ftot` (parallels Ftot, may
    //    // be identical if never had a constraint) and `last_Ftot`.
    
    
    
    // Some needed inputs
    int tstep = runToDate - core->getStartDate();
    double aero_forcing = double(core->sendMessage( M_GETDATA, D_RF_BC ).value( U_W_M2 )) + double(core->sendMessage( M_GETDATA, D_RF_OC ).value( U_W_M2 )) + double(core->sendMessage( M_GETDATA, D_RF_SO2d ).value( U_W_M2 )) + double(core->sendMessage( M_GETDATA, D_RF_SO2i ).value( U_W_M2 ));
    forcing[tstep] = double(core->sendMessage( M_GETDATA, D_RF_TOTAL ).value( U_W_M2 )) - ( 1.0 - alpha ) * aero_forcing;
			    
    // Initialize variables for time-stepping through the model
    double DQ1 = 0.0;
    double DQ2 = 0.0;
    double QC1 = 0.0;
    double QC2 = 0.0;
    double DelQL = 0.0;
    double DelQO = 0.0;
    double DPAST1 = 0.0;
    double DPAST2 = 0.0;
    double DTEAUX1 = 0.0;
    double DTEAUX2 = 0.0;
    
    // Reset the endogenous varibales for this time step
    temp[tstep] = 0.0;
    temp_landair[tstep] = 0.0;
    temp_sst[tstep] = 0.0;
    heat_mixed[tstep] = 0.0;
    heat_interior[tstep] = 0.0;
    heatflux_mixed[tstep] = 0.0;
    heatflux_interior[tstep] = 0.0;
    
    // Assume land and ocean forcings are equal to global forcing
    std::vector<double> QL = forcing;
    std::vector<double> QO = forcing;
    
    if (tstep > 0) {
        
        DelQL = QL[tstep] - QL[tstep - 1];
        DelQO = QO[tstep] - QO[tstep - 1];
        
        // Assume linear forcing change between tstep and tstep+1
        QC1 = (DelQL/cal*(1.0/taucfl+1.0/taukls)-bsi*DelQO/cas/taukls);
        QC2 = (DelQO/cas*(1.0/taucfs+bsi/tauksl)-DelQL/cal/tauksl);
        QC1 = QC1 * pow(double(dt), 2.0)/12.0;
        QC2 = QC2 * pow(double(dt), 2.0)/12.0;
        
        // ----------------- Initial Conditions --------------------
        // Initialization of temperature and forcing vector:
        // Factor 1/2 in front of Q in Equation A.27, EK05, and Equation 2.3.27, TK07 is a typo!
        // Assumption: linear forcing change between n and n+1
        DQ1 = 0.5*double(dt)/cal*(QL[tstep]+QL[tstep-1]);
        DQ2 = 0.5*double(dt)/cas*(QO[tstep]+QO[tstep-1]);
        DQ1 = DQ1 + QC1;
        DQ2 = DQ2 + QC2;
        
        // ---------- SOLVE MODEL ------------------
        // Calculate temperatures
        for(int i = 0; i <= tstep; i++) {
            DPAST2 = DPAST2 + temp_sst[i] * Ker[ns-tstep+i-1];
        }
        DPAST2 = DPAST2 * fso * pow((double(dt)/taudif), 0.5);
        
        DTEAUX1 = A[0] * temp_landair[tstep-1] + A[1] * temp_sst[tstep-1];
        DTEAUX2 = A[2] * temp_landair[tstep-1] + A[3] * temp_sst[tstep-1];
        
        temp_landair[tstep] = IB[0] * (DQ1 + DPAST1 + DTEAUX1) + IB[1] * (DQ2 + DPAST2 + DTEAUX2);
        temp_sst[tstep] = IB[2] * (DQ1 + DPAST1 + DTEAUX1) + IB[3] * (DQ2 + DPAST2 + DTEAUX2);
    }
    else {  // Handle the initial conditions
        temp_landair[0] = 0.0;
        temp_sst[0] = 0.0;
    }
    temp[tstep] = flnd * temp_landair[tstep] + (1.0 - flnd) * bsi * temp_sst[tstep];
    
    // Calculate ocean heat uptake [W/m^2]
    // heatflux[tstep] captures in the heat flux in the period between tstep-1 and tstep.
    // Numerical implementation of Equation 2.7, EK05, or Equation 2.3.13, TK07)
    // ------------------------------------------------------------------------
    if (tstep > 0) {
        heatflux_mixed[tstep] = cas*(temp_sst[tstep] - temp_sst[tstep-1]);
        for (int i=0; i < tstep; i++) {
            heatflux_interior[tstep] = heatflux_interior[tstep] + temp_sst[i]*Ker[ns-tstep+i];
        }
        heatflux_interior[tstep] = cas*fso/pow((taudif*dt), 0.5)*(2.0*temp_sst[tstep] - heatflux_interior[tstep]);
        heat_mixed[tstep] = heat_mixed[tstep-1] + heatflux_mixed[tstep] * (powtoheat*dt);
        heat_interior[tstep] = heat_interior[tstep-1] + heatflux_interior[tstep] * (fso*powtoheat*dt);
    }
    
    else {   // Handle the initial conditions
        heatflux_mixed[0] = 0.0;
        heatflux_interior[0] = 0.0;
        heat_mixed[0] = 0.0;
        heat_interior[0] = 0.0;
    }

    flux_mixed.set( heatflux_mixed[tstep], U_W_M2, 0.0 );
    flux_interior.set( heatflux_interior[tstep], U_W_M2, 0.0 );			  
    tgav.set( temp[tstep], U_DEGC, 0.0 );
    tgaveq.set( temp[tstep], U_DEGC, 0.0 );
    H_LOG( logger, Logger::DEBUG ) << " tgav=" << tgav << " in " << runToDate << std::endl;
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval TempDOECLIMComponent::getData( const std::string& varName,
                                const double date ) throw ( h_exception ) {
    
    unitval returnval;
    
    H_ASSERT( date == Core::undefinedIndex(), "Only current temperatures provided" );
    
    if( varName == D_GLOBAL_TEMP ) {
        returnval = tgav;
    } else if( varName == D_GLOBAL_TEMPEQ ) {
        returnval = tgaveq;
    } else if( varName == D_DIFFUSIVITY ) {
        returnval = diff;
    } else if( varName == D_AERO_SCALE ) {
	returnval = alpha;
    } else if( varName == D_FLUX_MIXED ) {
	returnval = flux_mixed;
    } else if( varName == D_FLUX_INTERIOR ) {
	returnval = flux_interior;
    } else if( varName == D_ECS ) {
        returnval = S;
    } else {
        H_THROW( "Caller is requesting unknown variable: " + varName );
    }
    
    return returnval;
}


//------------------------------------------------------------------------------
// documentation is inherited
void TempDOECLIMComponent::shutDown() {
	H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void TempDOECLIMComponent::accept( AVisitor* visitor ) {
    visitor->visit( this );
}

}
