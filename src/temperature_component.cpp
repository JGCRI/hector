/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  temperature_component.cpp
 *  hector
 *
 *  Created by Ben Vega-Westhoff on 11/1/16.
 *
 */

#include <boost/lexical_cast.hpp>
#include <cmath>
#include <limits>
#define _USE_MATH_DEFINES

#include "temperature_component.hpp"
#include "core.hpp"
#include "h_util.hpp"
#include "simpleNbox.hpp"
#include "avisitor.hpp"

namespace Hector {

using namespace std;


//------------------------------------------------------------------------------
/*! \brief Constructor
 */
TemperatureComponent::TemperatureComponent() {
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 */
TemperatureComponent::~TemperatureComponent(){
}

//------------------------------------------------------------------------------
// documentation is inherited
string TemperatureComponent::getComponentName() const {
    const string name = TEMPERATURE_COMPONENT_NAME;

    return name;
}


//------------------------------------------------------------------------------
/*! \brief              Calculates inverse of x and stores in y
 *  \param[in] x        Assume x is setup like x = [a,b,c,d] -> x = |a, b|
 *                                                                  |c, d|
 *  \param[out] y        Inverted 1-d matrix
 *  \returns            void, inverse is stored in y
 */
void TemperatureComponent::invert_1d_2x2_matrix(double * x, double * y) {
    double temp_d = (x[0]*x[3] - x[1]*x[2]);

    if(temp_d == 0) {
        H_THROW("Temperature: Matrix inversion divide by zero.");
    }
    double temp = 1/temp_d;
    y[0] = temp * x[3];
    y[1] = temp * -1 * x[1];
    y[2] = temp * -1 * x[2];
    y[3] = temp * x[0];

    return;
}

//------------------------------------------------------------------------------
// documentation is inherited
void TemperatureComponent::init( Core* coreptr ) {
    logger.open( getComponentName(), false, coreptr->getGlobalLogger().getEchoToFile(), coreptr->getGlobalLogger().getMinLogLevel() );
    H_LOG( logger, Logger::DEBUG ) << "hello " << getComponentName() << std::endl;

    tgaveq.set( 0.0, U_DEGC, 0.0 );
    tgav.set( 0.0, U_DEGC, 0.0 );
    flux_mixed.set( 0.0, U_W_M2, 0.0 );
    flux_interior.set( 0.0, U_W_M2, 0.0 );
    heatflux.set( 0.0, U_W_M2, 0.0 );

    core = coreptr;

    tgav_constrain.allowInterp( true );
    tgav_constrain.name = D_TGAV_CONSTRAIN;

    // Define the doeclim parameters
    diff.set( 0.55, U_CM2_S );    // default ocean heat diffusivity, cm2/s. value is CDICE default (varname is kappa there).
    S.set( 3.0, U_DEGC );         // default climate sensitivity, K (varname is t2co in CDICE).
    alpha.set( 1.0, U_UNITLESS);  // default aerosol scaling, unitless (similar to alpha in CDICE).
    volscl.set(1.0, U_UNITLESS);  // Default volcanic scaling, unitless (works the same way as alpha)

    // Register the data we can provide
    core->registerCapability( D_GLOBAL_TEMP, getComponentName() );
    core->registerCapability( D_LAND_TEMP, getComponentName() );
    core->registerCapability( D_OCEAN_AIR_TEMP, getComponentName() );
    core->registerCapability( D_OCEAN_SURFACE_TEMP, getComponentName() );
    core->registerCapability( D_GLOBAL_TEMPEQ, getComponentName() );
    core->registerCapability( D_FLUX_MIXED, getComponentName() );
    core->registerCapability( D_FLUX_INTERIOR, getComponentName() );
    core->registerCapability( D_HEAT_FLUX, getComponentName() );

    // Register our dependencies
    core->registerDependency( D_RF_TOTAL, getComponentName() );
    core->registerDependency( D_RF_BC, getComponentName() );
    core->registerDependency( D_RF_OC, getComponentName() );
    core->registerDependency( D_RF_SO2d, getComponentName() );
    core->registerDependency( D_RF_SO2i, getComponentName() );
    core->registerDependency( D_RF_VOL, getComponentName() );

    // Register the inputs we can receive from outside
    core->registerInput(D_ECS, getComponentName());
    core->registerInput(D_DIFFUSIVITY, getComponentName());
    core->registerInput(D_AERO_SCALE, getComponentName());
    core->registerInput(D_VOLCANIC_SCALE, getComponentName());
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval TemperatureComponent::sendMessage( const std::string& message,
                                    const std::string& datum,
                                    const message_data info ) throw ( h_exception )
{
    unitval returnval;

    if( message==M_GETDATA ) {          //! Caller is requesting data
        return getData( datum, info.date );
    } else if( message==M_SETDATA ) {   //! Caller is requesting to set data
        setData(datum, info);
    } else {                        //! We don't handle any other messages
        H_THROW( "Caller sent unknown message: "+message );
    }

    return returnval;
}

//------------------------------------------------------------------------------
// documentation is inherited
void TemperatureComponent::setData( const string& varName,
                              const message_data& data ) throw ( h_exception )
{
    using namespace boost;

    H_LOG( logger, Logger::DEBUG ) << "Setting " << varName << "[" << data.date << "]=" << data.value_str << std::endl;

    try {
        if( varName == D_ECS ) {
            H_ASSERT( data.date == Core::undefinedIndex() , "date not allowed" );
            S = data.getUnitval(U_DEGC);
        } else if( varName == D_DIFFUSIVITY ) {
            H_ASSERT( data.date == Core::undefinedIndex(), "date not allowed" );
            diff = data.getUnitval(U_CM2_S);
	} else if( varName == D_AERO_SCALE ) {
            H_ASSERT( data.date == Core::undefinedIndex(), "date not allowed" );
            alpha = data.getUnitval(U_UNITLESS);
        } else if(varName == D_VOLCANIC_SCALE) {
            H_ASSERT( data.date == Core::undefinedIndex(), "date not allowed" );
            volscl = data.getUnitval(U_UNITLESS);
        } else if( varName == D_TGAV_CONSTRAIN ) {
            H_ASSERT( data.date != Core::undefinedIndex(), "date required" );
            tgav_constrain.set(data.date, data.getUnitval(U_DEGC));
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
void TemperatureComponent::prepareToRun() throw ( h_exception ) {

    H_LOG( logger, Logger::DEBUG ) << "prepareToRun " << std::endl;

    if( tgav_constrain.size() ) {
        Logger& glog = core->getGlobalLogger();
        H_LOG( glog, Logger::WARNING ) << "Temperature will be overwritten by user-supplied values!" << std::endl;
    }

    // Initializing all model components that depend on the number of timesteps (ns)
    ns = core->getEndDate() - core->getStartDate() + 1;

    KT0 = std::vector<double>(ns, 0.0);
    KTA1 = std::vector<double>(ns, 0.0);
    KTB1 = std::vector<double>(ns, 0.0);
    KTA2 = std::vector<double>(ns, 0.0);
    KTB2 = std::vector<double>(ns, 0.0);
    KTA3 = std::vector<double>(ns, 0.0);
    KTB3 = std::vector<double>(ns, 0.0);

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

    // DOECLIM parameters calculated from constants set in header
    ocean_area = (1.0 - flnd) * earth_area;    // m2
    cnum = rlam * flnd + bsi * (1.0 - flnd);   // factor from sea-surface climate sensitivity to global mean
    cden = rlam * flnd - ak * (rlam - bsi);    // intermediate parameter
    cfl = flnd * cnum / cden * q2co / S - bk * (rlam - bsi) / cden;      // land climate feedback parameter, W/m2/K
    cfs = (rlam * flnd - ak / (1.0 - flnd) * (rlam - bsi)) * cnum / cden * q2co / S + rlam * flnd / (1.0 - flnd) * bk * (rlam - bsi) / cden;                                // sea climate feedback parameter, W/m2/K
    kls = bk * rlam * flnd / cden - ak * flnd * cnum / cden * q2co / S;  // land-sea heat exchange coefficient, W/m2/K
    keff = kcon * diff;                                                  // ocean heat diffusivity, m2/yr
    taubot = pow(zbot,2) / keff;                                         // ocean bottom diffusion time scale, yr
    powtoheat = ocean_area * secs_per_Year / pow(10.0,22);               // convert flux to total ocean heat
    taucfs = cas / cfs;                                                  // sea climate feedback time scale, yr
    taucfl = cal / cfl;                                                  // land climate feedback time scale, yr
    taudif = pow(cas,2) / pow(csw,2) * M_PI / keff;                      // interior ocean heat uptake time scale, yr
    tauksl  = (1.0 - flnd) * cas / kls;                                  // sea-land heat exchange time scale, yr
    taukls  = flnd * cal / kls;                                          // land-sea heat exchange time scale, yr

    // Components of the analytical solution to the integral found in the temperature difference equation
    // Third order bottom correction terms will be "more than sufficient" for simulations out to 2500
    // (Equation A.25, EK05, or 2.3.23, TK07)

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

    // Correction terms, remove oscillation artefacts due to short-term forcings
    // (Equation 2.3.27, TK07)
    C[0] = 1.0 / pow(taucfl, 2.0) + 1.0 / pow(taukls, 2.0) + 2.0 / taucfl / taukls + bsi / taukls / tauksl;
    C[1] = -1 * bsi / pow(taukls, 2.0) - bsi / taucfl / taukls - bsi / taucfs / taukls - pow(bsi, 2.0) / taukls / tauksl;
    C[2] = -1 * bsi / pow(tauksl, 2.0) - 1.0 / taucfs / tauksl - 1.0 / taucfl / tauksl -1.0 / taukls / tauksl;
    C[3] = 1.0 / pow(taucfs, 2.0) + pow(bsi, 2.0) / pow(tauksl, 2.0) + 2.0 * bsi / taucfs / tauksl + bsi / taukls / tauksl;
    for(int i=0; i<4; i++) {
        C[i] = C[i] * (pow(double(dt), 2.0) / 12.0);
    }

    //------------------------------------------------------------------
    // Matrices of difference equation system B*T(i+1) = Q(i) + A*T(i)
    // T = (TL,TS)
    // (Equations 2.3.24 and 2.3.27, TK07)
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
void TemperatureComponent::run( const double runToDate ) throw ( h_exception ) {
    H_LOG( logger, Logger::DEBUG ) << "temperature run " << runToDate << std::endl;

    // Commented section below is for case of user-specified temperature record.
    // Temperature component can't handle that at the moment!

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
        H_LOG( logger, Logger::SEVERE ) << "** ERROR - Temperature can't currently handle user-supplied temperature" << std::endl;
        H_THROW("User-supplied temperature not yet implemented.")
    }

    // Some needed inputs
    int tstep = runToDate - core->getStartDate();
    double aero_forcing =
        double(core->sendMessage( M_GETDATA, D_RF_BC ).value( U_W_M2 )) + double(core->sendMessage( M_GETDATA, D_RF_OC).value( U_W_M2 )) +
        double(core->sendMessage( M_GETDATA, D_RF_SO2d ).value( U_W_M2 )) + double(core->sendMessage( M_GETDATA, D_RF_SO2i ).value( U_W_M2 ));
    double volcanic_forcing = double(core->sendMessage(M_GETDATA, D_RF_VOL));

    forcing[tstep] = double(core->sendMessage(M_GETDATA, D_RF_TOTAL).value(U_W_M2))
                      - (1.0 - alpha) * aero_forcing
                      - (1.0 - volscl) * volcanic_forcing;

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

    setoutputs(tstep);
    H_LOG( logger, Logger::DEBUG ) << " tgav=" << tgav << " in " << runToDate << std::endl;
}

//------------------------------------------------------------------------------
// documentation is inherited
unitval TemperatureComponent::getData( const std::string& varName,
                                const double date ) throw ( h_exception ) {

    unitval returnval;

    if(date == Core::undefinedIndex()) {
        // If no date is supplied, return the current value
        if( varName == D_GLOBAL_TEMP ) {
            returnval = tgav;
        } else if( varName == D_GLOBAL_TEMPEQ ) {
            returnval = tgaveq;
        } else if( varName == D_LAND_TEMP ) {
            returnval = tgav_land;
        } else if( varName == D_OCEAN_SURFACE_TEMP ) {
            returnval = tgav_sst;
        } else if( varName == D_OCEAN_AIR_TEMP ) {
            returnval = tgav_oceanair;
        } else if( varName == D_DIFFUSIVITY ) {
            returnval = diff;
        } else if( varName == D_AERO_SCALE ) {
	    returnval = alpha;
        } else if( varName == D_FLUX_MIXED ) {
	    returnval = flux_mixed;
        } else if( varName == D_FLUX_INTERIOR ) {
	    returnval = flux_interior;
        } else if( varName == D_HEAT_FLUX) {
            returnval = heatflux;
        } else if( varName == D_ECS ) {
            returnval = S;
        } else if(varName == D_VOLCANIC_SCALE) {
            returnval = volscl;
        } else {
            H_THROW( "Caller is requesting unknown variable: " + varName );
        }
    }
    else {
        // If a date is supplied, get the needed value from the
        // vectors.  Some values, such as model parameters, don't have
        // time-indexed values, so asking for one of those with a date
        // is an error.
        H_ASSERT(date <= core->getCurrentDate(), "Date must be <= current date.");
        int tstep = date - core->getStartDate();

        if( varName == D_GLOBAL_TEMP ) {
            returnval = unitval(temp[tstep], U_DEGC);
        } else if( varName == D_LAND_TEMP ) {
            returnval = unitval(temp_landair[tstep], U_DEGC);
        } else if( varName == D_OCEAN_SURFACE_TEMP ) {
            returnval = unitval(temp_sst[tstep], U_DEGC);
        } else if( varName == D_OCEAN_AIR_TEMP ) {
            returnval = bsi * unitval(temp_sst[tstep], U_DEGC);
        } else if( varName == D_GLOBAL_TEMPEQ ) {
            returnval = unitval(temp[tstep], U_DEGC);
        } else if( varName == D_FLUX_MIXED ) {
	    returnval = unitval(heatflux_mixed[tstep], U_W_M2);
        } else if( varName == D_FLUX_INTERIOR ) {
	    returnval = unitval(heatflux_interior[tstep], U_W_M2);
        } else if( varName == D_HEAT_FLUX) {
            double value = heatflux_mixed[tstep] + fso*heatflux_interior[tstep];
            returnval = unitval(value, U_W_M2);
        }
    }

    return returnval;
}


void TemperatureComponent::reset(double time) throw(h_exception)
{
    // We take a slightly different approach in this component's reset method than we have in other components.  The
    // temperature component doesn't have its own time counter, and it stores its history in a collection of double
    // vectors.  Therefore, all we do here is set the unitval versions of that stored data to their values from the
    // vectors.
    if(time < core->getStartDate()) // in this case, reset to the starting value.
        time = core->getStartDate();

    int tstep = time - core->getStartDate();
    setoutputs(tstep);
    H_LOG(logger, Logger::NOTICE)
        << getComponentName() << " reset to time= " << time << "\n";
}



//------------------------------------------------------------------------------
// documentation is inherited
void TemperatureComponent::shutDown() {
	H_LOG( logger, Logger::DEBUG ) << "goodbye " << getComponentName() << std::endl;
    logger.close();
}

//------------------------------------------------------------------------------
// documentation is inherited
void TemperatureComponent::accept( AVisitor* visitor ) {
    visitor->visit( this );
}


void TemperatureComponent::setoutputs(int tstep)
{
    double temp_oceanair;

    flux_mixed.set( heatflux_mixed[tstep], U_W_M2, 0.0 );
    flux_interior.set( heatflux_interior[tstep], U_W_M2, 0.0 );
    heatflux.set( heatflux_mixed[tstep] + fso * heatflux_interior[tstep], U_W_M2, 0.0 );
    tgav.set(temp[tstep], U_DEGC, 0.0);
    tgaveq.set(temp[tstep], U_DEGC, 0.0); // per comment line 140 of temperature_component.hpp
    tgav_land.set(temp_landair[tstep], U_DEGC, 0.0);
    tgav_sst.set(temp_sst[tstep], U_DEGC, 0.0);
    temp_oceanair = bsi * temp_sst[tstep];
    tgav_oceanair.set(temp_oceanair, U_DEGC, 0.0);
}

}

