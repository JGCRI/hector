/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
// ocean_csys_class.cpp : Defines the entry point for the console application.
/*  Ocean Carbon Chemistry CODE File:
 *
 *  Created by Corinne Hartin  1/30/13.

 *  This code translated from MATLAB code  http://www.soest.hawaii.edu/oceanography/faculty/zeebe_files/CO2_System_in_Seawater/csys.html
 *  Reference: Richard E. Zeebe and Dieter A. Wolf-Gladrow

 *	Alfred Wegener Institute for
 *	Polar and Marine Research
 *	P.O. Box 12 01 61
 *   D-27515 Bremerhaven
 *	Germany
 *	e-mail: rzeebe@awi-bremerhaven.de   wolf@awi-bremerhaven.de
 *
 *   based on the book by Zeebe, R. E. and Wolf-Gladrow, D.: CO2 in Seawater: Equilibrium, Kinetics, Isotopes, Elsevier, Amsterdam, 2001.
 *
 *
 *   Other References
 *   DOE: Handbook of methods for the analysis of the various parameters
 *      of the carbon dioxide system in sea water, edited by: Dickson,
 *      A. G. and Goyet, C., ORNL/CDIAC-74, 1994.
 *   Hartin, C. A., Bond-Lamberty, B., Patel, P., and Mundra, A.:
 *      Ocean acidification over the next three centuries using a simple global
 *      climate carbon-cycle model: projections and sensitivities, Biogeosciences, 13, 4329–4342, https://doi.org/10.5194/bg-13-4329-2016, 2016.
 *   Lueker, T. J., Dickson, A. G., and Keeling, C. D.: Ocean pCO2
 *      calculated from dissolved inorganic carbon, alkalinity, and equations
 *      for K1 and K2; validation based on laboratory measurements
 *      of CO2 in gas and seawater at equilibrium, Mar. Chem., 70, 105–119, 2000.
 *  Mucci, A.: The solubility of calcite and aragonite in seawater at
 *      various salinities, temperatures and at one atmopshere pressure,
 *      Am. J. Sci., 283, 781–799, 1983.
 *   Riebesell U., Fabry V. J., Hansson L. & Gattuso J.-P. (eds) (2011) Guide to best practices
 *      for ocean acidification research and data reporting. [reprinted edition including
 *   	erratum]. Luxembourg, Publications Office of the European Union, 258pp.
 *   	(EUR 24872 EN). DOI 10.2777/66906
 *   Wanninkhof, R.: Relationship between wind speed and gas exchange
 *      over the ocean, J. Geophys. Res.-Oceans, 97, 7373–7382,
 *      doi:10.1029/92JC00188, 1992.
 *   Weiss, R. F.: Carbon dioxide in water and seawater: the solubility
 *      of a non-ideal gas, Mar. Chem., 2, 203–215, doi:10.1016/0304-4203(74)90015-2, 1974.
 *
 */

#include <math.h>

// some boost headers generate warnings under clang; not our problem, ignore
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <boost/math/tools/polynomial.hpp>
#pragma clang diagnostic pop

#include <boost/math/tools/roots.hpp>

#include "h_exception.hpp"
#include "ocean_csys.hpp"

namespace Hector {

using namespace std;

//------------------------------------------------------------------------------
/*! \brief new oceanbox logger
 *  oceanbox logger may or may not be defined and therefore we check before logging
 */
#define CS_LOG(log, level)  \
if( log != NULL ) H_LOG( (*log), level )

//------------------------------------------------------------------------------
/*! \brief constructor
 */
oceancsys::oceancsys() : ncoeffs(6), m_a(ncoeffs) {
	logger = NULL;
	S = alk = As = Ks = 0.0;
}

//------------------------------------------------------------------------------
/*! \brief A helper functor class used to evauluate a polynomial and its derivative.
 *  \details This helper class gives an interface that is callable from boost's numerical
 *           solvers which need to evaluate a function and it's derivative.  This class
 *           wrapps a polynomical by taking an array of coefficients in ascending order of
 *           the degree of the term they are associated.
 */
class PolyDerivFunctor {
    public:
        PolyDerivFunctor(const double* coefs, const int degree) {
            using namespace boost::math::tools;
            const int size = degree + 1;
            mPoly = polynomial<double>(coefs, degree);
            double* derivCoef = new double[size-1];
            for(int i = 1; i < size; ++i) {
                derivCoef[i - 1] = coefs[i] * static_cast<double>(i);
            }
            mPolyDeriv = polynomial<double>(derivCoef, degree-1);
            delete[] derivCoef;
        }

        pair<double, double> operator()(const double x) {
            return pair<double, double>(mPoly.evaluate(x), mPolyDeriv.evaluate(x));
        }

    private:
        //! The representation of the polynomial to calculate.
        boost::math::tools::polynomial<double> mPoly;

        //! The representation of the derivative of the polynomial to calculate.
        boost::math::tools::polynomial<double> mPolyDeriv;
};

//------------------------------------------------------------------------------
/*! \brief Find the largest real root, using GSL or appropriate algorithms
 *  \param ncoeffs   Number of coefficients
 *  \param a                Coefficients
 *  \return        Largest real root (H+ ion)
 */
double find_largest_root( const int ncoeffs, double* a ) {

    using namespace boost::math::tools;
    const int degree = ncoeffs-1;
    PolyDerivFunctor polyFunctor(a, degree);
    // Use Fujiwara's method to find an upper bound for the roots of the polynomial
    double max = pow(std::abs(a[0] / ( 2.0 * a[degree])), 1.0 / degree);
    for(int i = 1; i < degree; ++i) {
        max = std::max(max, pow(std::abs(a[i]/a[degree]), 1.0 / static_cast<double>(degree - i)));
    }
    max *= 2.0;
    // Use Newton's method to find the largest real root starting from the Fujiwara upper bound
    // arbitrarily solve unil 60% of the digits are correct.
    const int digits = numeric_limits<double>::digits;
    int get_digits = static_cast<int>(digits * 0.6);
    double h = newton_raphson_iterate(polyFunctor, max-0.001, 0.0, max, get_digits);

	return h;
}

//------------------------------------------------------------------------------
/*! \brief Run Ocean csys
 *
 * DIC and ALK calculate pH, pCO2, omega Ar, omega Ca
 * (from Zeebe and Wolfe-Gladrow 2001)
 * pCO2 is used to calculate ocean-atmosphere fluxes
 * (from Takahashi et al, 2009, eq. 7 & 8)
 */
void oceancsys::ocean_csys_run( unitval tbox, unitval carbon )
{

	double tmp, tmp1, tmp2, tmp3;

    // Convert carbon to dic value and temperature to K
    const double dic = convertToDIC( carbon ).value( U_UMOL_KG )/1e6;   // back to mol/kg
    const double Tc = tbox.value( U_DEGC );
    const double Tk = Tc + 273.15;

	// Check that all is OK with input data
	//H_ASSERT( Tk > 265 && Tk < 308, "bad Tk value" ); // Kelvin
    H_ASSERT( dic > 1000e-6 && dic < 3700e-6, "bad dic value" );  // mol/kg

    // alk should be constant once spinup is done, but check anyway
    H_ASSERT( alk >= 2000e-6 && alk <= 2750e-6, "bad alk value" );  // mol/kg

	/*---------------------------------------------------------------
     This section calculates the constants K0, Sc, K1, K2, Ksp, Ksi etc.
     ---------------------------------------------------------------*/

	// --------------------- K0 -----------------------------------
	// Solubility of CO2 Weiss 1974 (mol * L-1 * atm-1)
	// Weiss 1974 equation 12 constants listed in column 1 of TABLE I
	// CO2 solubility will be used to calculate fluxes.
	tmp1 = -58.0931 + 90.5069 * ( 100/Tk ) + 22.2940 * log( Tk/100 );
	tmp2 = S * ( 0.027766 - 0.025888 * ( Tk/100 ) + 0.0050578 * ( ( Tk/100 ) * ( Tk/100 ) ) );
	const double lnK0 =  tmp1 + tmp2;
	K0.set( exp( lnK0 ), U_MOL_L_ATM );

	//---------------------Sc------------------------------------------
	// Calculate Schmidt Number an expressions for gas transfer that depends on temperature and salinity.
	// Equation Sc = A -  Bt +  Ct 2 -  Dt 3 (t in degrees C) from WANNINKHOF 1992
	// see TABLE  A1 WANNINKHOF 1992 . for coefficients.
	const double Sc = 2073.1 - ( 125.62 * Tc ) + (3.6276 * Tc * Tc) - ( 0.043219 * Tc * Tc * Tc );

	// --------------------- Kwater -----------------------------------
	// Calculate equilibrium constants (on the total hydrogen ion scale) as a function of salinity (S) and temperature (T).
	// Table 1.1 of Part1: Seawater carbonate chemistry of Riebesell et al. 2011
	tmp1 = -13847.26/Tk + 148.96502 - 23.6521 * log( Tk ); // eq 1.10  Riebesell et al. 2011
	tmp2 = + (118.67/Tk - 5.977 + 1.0495*log( Tk ) ) * sqrt( S ) - 0.01615 * S; // eq 1.10 Riebesell et al. 2011
	const double lnKw =  tmp1 + tmp2;  // eq 1.10 Riebesell et al. 2011
	Kw.set( exp(lnKw), U_MOL_KG);


	//---------------------- Kh (K Henry) ----------------------------
	// Solubility of CO2 calculated using Henry's law Weiss 1974 (moles * atm * kg-1)
	// Weiss 1974 equation 12 constants listed in column 2 of TABLE I
	// Note Kh and K0 are identical equations with differing constants resulting in different units
	// used to calculate pCO2
	tmp = 9345.17 / Tk - 60.2409 + 23.3585 * log( Tk/100 );
	const double nKhwe74 = tmp + S * ( 0.023517-0.00023656 * Tk + 0.0047036e-4 * Tk * Tk );
	Kh.set( exp( nKhwe74 ), U_MOL_KG_ATM);

	// --------------------- K1 ---------------------------------------
	// First acidity constants of carbonic acid
	// Lueker et al. (2000) equation 16
	const double pK1mehr = 3633.86/Tk - 61.2172 + 9.6777*log( Tk ) - 0.011555 * S + 0.0001152 * S * S;
	const unitval K1( pow( 10, -pK1mehr ), U_MOL_KG);

	// --------------------- K2 ----------------------------------------
	// Second acidity constants of carbonic acid
	// Lueker et al. (2000) equation 17
	const double pK2mehr = 471.78/Tk + 25.9290 - 3.16967 * log( Tk ) - 0.01781 * S + 0.0001122 * S * S;
	const unitval K2( pow( 10.0, -pK2mehr ), U_MOL_KG);

	// --------------------- Kb  --------------------------------------------
	// The equilibrium constant of boric acid
	// DOE 1994
	tmp1 =  ( -8966.90-2890.53 * sqrt( S ) - 77.942 * S+ 1.728*pow( S,( 3.0/2.0 ) ) - 0.0996 * S * S )/Tk;
	tmp2 =   +148.0248+137.1942 * sqrt( S ) + 1.62142 * S;
	tmp3 = +(-24.4344-25.085 * sqrt( S )-0.2474 * S ) * log( Tk ) + 0.053105 * sqrt( S ) * Tk;
	const double lnKb = tmp1 + tmp2 + tmp3;
	const unitval Kb( exp(lnKb), U_MOL_KG);

	// --------------------- Kspc (calcite) ----------------------------
	// Solubility of calcite
	// Mucci 1983
	tmp1 = -171.9065-0.077993 * Tk + 2839.319/Tk + 71.595 * log10( Tk );
	tmp2 = +( -0.77712+0.0028426 * Tk + 178.34/Tk ) * sqrt( S );
	tmp3 = -0.07711 * S + 0.0041249 * pow( S, 1.5 );
	const double log10Kspc = tmp1 + tmp2 + tmp3;
	const double Kspc = pow( 10.0, log10Kspc ); // mol/kg

	// --------------------- Kspa (aragonite) ----------------------------
	// Solubility of aragonite
	// Mucci 1983
	tmp1 = -171.945 - 0.077993 * Tk + 2903.293 / Tk + 71.595 * log10( Tk );
	tmp2 = +( -0.068393+0.0017276 * Tk + 88.135/Tk ) * sqrt( S );
	tmp3 = -0.10018 * S + 0.0059415 * pow( S, 1.5 );
	const double log10Kspa = tmp1 + tmp2 + tmp3;
	const double Kspa = pow( 10.0, log10Kspa ); // mol/kg

	//------------------------- boron --------------------------------------
	// Total boron concentration related to seawater salinity
	// DOE 1994
	const double bor = 1 * ( 416.0 * ( S/35.0 ) ) * 1.e-6;   // (mol/kg)

	/* ---------------------------------------
     Since ALK and DIC are given, solve for pH and pCO2
     ------------------------------------------*/
	// DIC is calculated as a function of total carbon pool size
	// & ALK (total alkalinity) calculated at the end of model spinup
	// and held constant for the rest of the run. See Hartin 2015.

	// See Zeebe and Wolf-Gladrow 2001
	// Appendix B system of equations 15.

    const double Kb_val = Kb.value( U_MOL_KG );     // for convenience in eqns below
    const double K1_val = K1.value( U_MOL_KG );
    const double K2_val = K2.value( U_MOL_KG );
    const double Kw_val = Kw.value( U_MOL_KG );

    // Define the six variables of the carbonate system
	const double p5 = -1.0;
	const double p4 = -alk - Kb_val - K1_val;
	const double p3 = dic * K1_val - alk * ( Kb_val + K1_val )
        + Kb_val * bor + Kw_val - Kb_val
        * K1_val - K1_val * K2_val;
	tmp = dic * ( Kb_val * K1_val + 2.0 * K1_val * K2_val )
        -alk * (Kb_val * K1_val + K1_val * K2_val)
        + Kb_val * bor * K1_val;
	const double p2 = tmp + ( Kw_val * Kb_val + Kw_val * K1_val - Kb_val * K1_val * K2_val );
	tmp = 2.0 * dic * Kb_val * K1_val * K2_val
        - alk * Kb_val * K1_val * K2_val + Kb_val
        * bor * K1_val * K2_val;
	const double p1 = tmp + ( Kw_val * Kb_val * K1_val + Kw_val * K1_val * K2_val );
	const double p0 = Kw_val * Kb_val * K1_val * K2_val;

	m_a[ 0 ] = p0;
	m_a[ 1 ] = p1;
	m_a[ 2 ] = p2;
	m_a[ 3 ] = p3;
	m_a[ 4 ] = p4;
	m_a[ 5 ] = p5;

	// Find the solution to the polynomial of the carbonate system
	const double h      = find_largest_root( ncoeffs, &m_a[0] );

	// Solve for the remiaing carbonate variables
	const double co2st  = dic/( 1.0 + K1_val / h + K1_val * K2_val / h / h ); // co2st = CO2*
	const double hco3   = dic/( 1.0 + h / K1_val + K2_val / h );
	const double co3    = dic/( 1.0 + h / K2_val + h * h / K1_val / K2_val ); // mol/kg

	const double million = 1e6; // unit conversion

	// Output (all variables beginning with capital letter below)
	TCO2o.set( co2st * million, U_UMOL_KG );
	HCO3.set( hco3 * million, U_UMOL_KG );
	CO3.set( co3 * million, U_UMOL_KG );
	PCO2o.set ( co2st * million/Kh.value( U_MOL_KG_ATM ), U_UATM );
	pH.set (-log10( h ), U_PH);

    // ----------------------------------------------------------------------------
    /*! Calculate air-sea flux of carbon
     * based on Takahashi et al, 2009 Deep Sea Research
     * Uses K0 (solubility), Sc (Schmidt number) , U (wind stress), PCO2atm, PCO2o
     */

	Tr.set( ( 0.585 * K0.value( U_MOL_L_ATM )
             * pow( Sc, -0.5 ) * U * U ), U_gC_m2_month_uatm );  // units : gC m-2 month-1 uatm-1.
	// 0.585 is a unit conversion factor from Takahashi et al, 2009 equation 8
	// unit conversion * solubility * Schmidt number * wind speed^2

    //------------------------------------------------------------------------
    /*! \brief calculate Omega of Ca/Ar
     * Uses Ksp of Ca and Ar, CO3, S, and pH
     */

	// this is 0.010285*S/35
	const double calcium = 0.02128/40.087 * ( S/1.80655 ); //mol/kg Riley, and Tongudai, Chemical Geology 2:263-269, 1967
	OmegaCa.set( ( ( co3 * calcium ) / Kspc ), U_UNITLESS );
	OmegaAr.set( ( ( co3 * calcium ) / Kspa ), U_UNITLESS );
}

//-------------------------------------------------------------------------------
/*! \brief Calculate the (monthly) atmosphere-surface box flux
 *  \param Ca           Atmospheric CO2
 *  \param cpoolscale   Scale the box C pool by this amount (1.0=none)
 *  \return             Monthly atmospheric C flux, gC/m2/month (positive = into ocean)
 */
double oceancsys::calc_monthly_surface_flux( const unitval& Ca, const double cpoolscale ) const {
    // Tr is the gas transfer coefficient
	return ( ( Ca.value( U_PPMV_CO2 ) - PCO2o.value( U_UATM ) * cpoolscale ) * Tr.value( U_gC_m2_month_uatm ) ); // units : gC m-2 month-1
}

//-------------------------------------------------------------------------------
/*! \brief Calculate the (annualized) atmosphere-surface box flux
 *  \param Ca           Atmospheric CO2
 *  \param cpoolscale   Scale the box C pool by this amount (1.0=none)
 *  \return             Annual atmospheric C flux, Pg C/yr (positive = into ocean)
 */
unitval oceancsys::calc_annual_surface_flux( const unitval& Ca, const double cpoolscale ) const {
    // Surface flux (gC m-2 month-1) * area of the box * number of months in the year / conversion constant
    return unitval( ( calc_monthly_surface_flux( Ca, cpoolscale ) * As * 12.0 ) / 1e15, U_PGC_YR );
}

//-------------------------------------------------------------------------------
/*! \brief Convert the total carbon pool (PgC) to DIC
 *  \param carbon       Carbon value to convert (Pg C)
 * Uses carbon pool, mass of carbon, density of seawater and volume of the box
 */
unitval oceancsys::convertToDIC( const unitval carbon ) {
    // Carbon pool / (C atmoic mass * density of sea water * volume )
	const double dic = ( ( carbon.value( U_PGC ) * 1e15 ) * ( 1.0/12.01 ) * (1.0/1027.0 ) * ( 1.0/volumeofbox ) ); // mol/kg
	return unitval( dic * 1e6, U_UMOL_KG );
}

}
