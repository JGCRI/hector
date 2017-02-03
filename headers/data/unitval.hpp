/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef UNITVAL_H
#define UNITVAL_H
/*
 *  unitval.hpp - a very basic units system for numbers
 *  hector
 *
 *  Created by Ben on 12/31/12.
 *
 */

#include "core/logger.hpp"
#include "h_exception.hpp"

namespace Hector {
  
/*! \brief A simple value-and-units capability.
 *
 *  What was wrong with using the Boost Units (BU) package?
 *
 *  BU is really cool, in an academic c.s. kind of way. But its
 *  capabilities come at a large cost: one more library to download,
 *  increased complexity, impenetrable error messages, stringent compiler 
 *  requirements, verbose code, and high overhead. (Yes, I know, zero
 *  run-time overhead. But the development overhead is high.)
 *
 *  Our use of units is prompted primarily by safety--to avoid the
 *  "Mars Surveyor" problem of mismatched units--as well as having easy
 *  and nice-looking output. On-the-fly conversions are cool but, in the
 *  real world, how often do we need this?
 *
 *  A political metaphor: BU is a 'nanny state' approach (complex, cumbersome,
 *  and discouraging new development) while this file is a 'free market'
 *  approach (it provides some basic capabilities, but our fundamental 
 *  assumption is that model components will be responsible and check 
 *  themselves and their inputs). Anyway to summarize:
 *
 *                          Boost Units     This file
 *  Interprocess safety         X               X
 *  On-the-fly conversion       X
 *  Arbitrary input
 *  Ouptut                      X               X
 *  Complexity                  High            Low
 *
 *  </rant>
 */

enum unit_types {
                    U_UNITLESS,         // No units
    
                    U_PPMV_CO2,         // Atmospheric
                    U_PPBV,
					U_PPTV,
                    U_PPBV_CH4,
					U_PPTV_CH4,
					U_PPBV_N2O,
					U_PPTV_N2O,
                    U_MOL_YR,
					U_TG_CO,
					U_TG_CH4,
                    U_TG_N2O,
				    U_TG_NMVOC,
					U_DU_O3,
                    U_TG_N,             // NOX emissions given in TG-N/yr
                    U_GG_S,             // SO2 emissions in Gg-S/yr

                    U_TG_PPBV,          // Conversion for CH4 and N2O emission to concentrations
    
                    U_DEGC,             // Temperature
                    U_K,				// Temperature
                    U_1_K,               // Inverse Temperature
   
                    U_CM,               // Length-related
                    U_CM_YR,            
    
                    U_G,
                    U_TG,               // Mass-related
                    U_GG,				// Giga-grams
                    U_MOL,
                    U_GMOL,
					U_GT,	
    
                    U_PGC,              // Carbon pools and fluxes
                    U_PGC_YR,
    
                    U_W_M2,               // Forcing
                    U_W_M2_PPTV,          // Forcing efficiency
                    U_W_M2_K,             // ocean heat uptake efficiency
                    
                    U_M3_S,                // sverdrop (volume transport)
                    U_PH,                  // pH
					U_UATM,
					U_UMOL_KG,			   // umol/kg
					U_MOL_KG,			   // mol/kg
					U_gC_m2_month_uatm,    //Tr variable for testing
					U_MOL_L_ATM,		   // mol l-1 atm-1
					U_MOL_KG_ATM,		   // mol kg-1 atm-1
					U_J_KG_C,              // specific heat capacity

                    U_DOBSON,               // Dobson units (ozone)
                    
                    U_YRS,                  // Years

                    U_UNDEFINED             // Undefined units,
                                            // Warning: all units should be defined
                                            // before U_UNDEFINED
                };

class unitval {
    
    double      val;
    double      valErr;
    unit_types  valUnits;
    
public:
    static std::string unitsName( const unit_types );
    static unit_types parseUnitsName( const std::string& ) throw( h_exception );
        
    unitval();
    unitval( double, unit_types );
    
    void set( double, unit_types, double );
    
    double value( unit_types ) const throw( h_exception );
    unit_types units() const { return valUnits; };
    std::string unitsName() const { return unitsName( valUnits ); };
    void expecting_unit( const unit_types& ) throw( h_exception );

    static unitval parse_unitval( const std::string&, const unit_types& ) throw( h_exception );
    static unitval parse_unitval( const std::string&, const std::string&, const unit_types& ) throw( h_exception );

    /*! Allow us to assign a unitval to a double. 
     *  \note    Do not use this in Hector.  It is intended for other
     *           codes that use libhector and don't want to carry
     *           around units.
     */
    operator double() const {return val;}
    
    friend unitval operator+ ( const unitval&, const unitval& );
    friend unitval operator- ( const unitval&, const unitval& );
    friend unitval operator- ( const unitval& );
    friend unitval operator* ( const unitval&, const double );
    friend unitval operator* ( const double, const unitval& );
    friend unitval operator/ ( const unitval&, const double );
    friend unitval operator/ ( const double, const unitval& );
    friend double operator/ ( const unitval&, const unitval&  );
    friend std::ostream& operator<<( std::ostream &out, const unitval &x );

};


//-----------------------------------------------------------------------
/*! \brief Constructor for units data type.
 *
 *  Initializes internal variables.
 */
inline
unitval::unitval() {
    val = 0.0;
    valErr = 0.0;
    valUnits = U_UNDEFINED;
}

//-----------------------------------------------------------------------
/*! \brief Constructor for units data type.
 *
 *  Initializes internal variables and sets value and units.
 */
inline
unitval::unitval( double v, unit_types u ) {
    val=v;
    valUnits=u;
}

//-----------------------------------------------------------------------
/*! \brief Set the variable's value and optionally error.
 *
 *  Set value. Caller has to provide assumed units, as a check.
 */
inline
void unitval::set( double v, unit_types u, double err=0.0 ) {
    if( valUnits != U_UNDEFINED && u != valUnits ) {
        Logger& glog = Logger::getGlobalLogger();
        H_LOG( glog, Logger::WARNING) << "Warning: variable units have been redefined!" << std::endl;
    }
    val = v;
    valUnits = u;
    valErr = err;
}


//-----------------------------------------------------------------------
/*! \brief Get the variable's value.
 *
 *  Get value. Caller has to provide assumed units, as a check.
 */
inline
double unitval::value( unit_types u ) const throw( h_exception ) {
    H_ASSERT( u==valUnits, "variable is not of this type" );
    return( val );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: addition.
 *
 *  Add two unitvals. Must be of the same type.
 *  TODO: should we add the errors?
 */
inline
unitval operator+ ( const unitval& lhs, const unitval& rhs ) {
	H_ASSERT( lhs.valUnits==rhs.valUnits, "units mismatch" );
    return unitval( lhs.val+rhs.val, lhs.valUnits );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: subtraction.
 *
 *  Subtract two unitvals. Must be of the same type.
 *  TODO: should we add the errors?
 */
inline
unitval operator- ( const unitval& lhs, const unitval& rhs ) {
	H_ASSERT( lhs.valUnits==rhs.valUnits, "units mismatch" );
    return unitval( lhs.val-rhs.val, lhs.valUnits );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: unary minus.
 *
 *  Unary minus.
 */
inline
unitval operator- ( const unitval& rhs ) {
    return unitval( -rhs.val, rhs.valUnits );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: constant multiplication.
 *
 *  Multiply a unitval by a double.
 *  TODO: should we multiply error as well?
 */
inline
unitval operator* ( const unitval& lhs, const double rhs ) {
    return unitval( lhs.val*rhs, lhs.valUnits );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: constant multiplication.
 *
 *  Multiply a unitval by a double.
 *  TODO: should we multiply error as well?
 */
inline
unitval operator* ( const double lhs, const unitval& rhs ) {
    return unitval( lhs*rhs.val, rhs.valUnits );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: constant division.
 *
 *  Divide a unitval by a double.
 *  TODO: should we divide error as well?
 */
inline
unitval operator/ ( const unitval& lhs, const double rhs ) {
    return unitval( lhs.val/rhs, lhs.valUnits );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: constant division.
 *
 *  Divide a unitval by a double.
 *  TODO: should we divide error as well?
 */
inline
unitval operator/ ( const double lhs, const unitval& rhs ) {
    return unitval( lhs/rhs.val, rhs.valUnits );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: division.
 *
 *  Divide two unitvals. Must be of the same type, returning double.
 */
inline
double operator/ ( const unitval& lhs, const unitval& rhs ) {
	H_ASSERT( lhs.valUnits==rhs.valUnits, "units mismatch" );
    return lhs.val/rhs.val;
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: outputstream.
 *
 *  Print a unitval.
 */
inline
std::ostream& operator<<( std::ostream &out, const unitval &x ) {
    out << x.value( x.units() ) << " " << x.unitsName();
    return out;
}

}

#endif
