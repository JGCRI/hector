/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  unitval.cpp
 *  hector
 *
 *  Created by Ben on 12/31/12.
 *
 */

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "logger.hpp"
#include "unitval.hpp"
#include "h_util.hpp"

namespace Hector {
  
using namespace std;
using namespace boost;

//------------------------------------------------------------------------------
/*! \brief Return name of given unit.
 */
string unitval::unitsName( const unit_types u ) {
    switch( u ) {
    case U_UNDEFINED: return "(undefined)";
        break;
    case U_UNITLESS: return "(unitless)";
        break;
    case U_PPMV_CO2: return "ppmv CO2";
        break;
    case U_PPBV: return "ppbv";
        break;
    case U_PPBV_CH4: return "ppbv CH4";
        break;
    case U_PPTV_CH4: return "pptv CH4";
        break;
    case U_PPBV_N2O: return "ppbv N2O";
        break;
    case U_PPTV_N2O: return "pptv N2O";
        break;
    case U_DU_O3: return "DU O3";
        break;
    case U_TG_N: return "Tg N";
        break;
    case U_GG_S: return "Gg S";
        break;
    case U_TG_CO: return "Tg CO";
        break;
    case U_TG_CH4: return "Tg CH4";
        break;
    case U_TG_NMVOC: return "Tg NMVOC";
        break;
    case U_TG_PPBV: return "Tg/ppbv";
        break;
    case U_MOL_YR: return "mol/yr";
        break;
    case U_PPTV: return "pptv";
        break;
    case U_DEGC: return "degC";
        break;
    case U_K: return "K";
        break;
    case U_1_K: return "1_K";
        break;
    case U_CM: return "cm";
        break;
    case U_CM_YR: return "cm/yr";
        break;
    case U_CM2_S: return "cm2/s";
        break;
    case U_G: return "g";
        break;
    case U_GG: return "Gg";
        break;
    case U_TG: return "Tg";
        break;
    case U_MOL: return "mol";
        break;
    case U_GMOL: return "Gmol";
        break;
    case U_GT: return "GT(?)";
        break;
    case U_PGC: return "Pg C";
        break;
    case U_PGC_YR: return "Pg C/yr";
        break;
    case U_W_M2: return "W/m2";
        break;
    case U_W_M2_PPTV: return "W/m2/pptv";
        break;
    case U_W_M2_K: return "W/m2/K";
        break;
    case U_PH: return "pH";
        break;
    case U_UATM: return "uatm";
        break;
    case U_UMOL_KG: return "umol/kg";
        break;
    case U_MOL_KG: return "mol/kg";
        break;
    case U_gC_m2_month_uatm: return "gC/m/month/atm";
        break;
    case U_MOL_L_ATM: return "mol/l/atm";
        break;
    case U_MOL_KG_ATM: return "mol/kg/atm";
        break;
    case U_DOBSON: return "dobson";
        break;
    case U_YRS: return "Years";
        break;
        
    default: H_THROW( "Unhandled unit!" );
    }
}

//------------------------------------------------------------------------------
/*! \brief Parse a string into its unit_types representation.
 *
 *  Uses names consistent with unitsName to translate between strings and
 *  unit_types.
 */
unit_types unitval::parseUnitsName( const string& unitsStr ) throw( h_exception ) {
    // Iterate over all the enumerated units and check if its unitsName() matches
    // the given units string.
    /*!
     * \warning All inits in unit_types must be defined before U_UNDEFINED.
     */
    for( int i = 0; i <= U_UNDEFINED; ++i ) {
        const unit_types u = static_cast<unit_types>( i );
        if( unitsStr == unitsName( u ) ) {
            return u;
        }
    }
    
    // No unitsNames matched.
    H_THROW( "Could not parse unknown units string: " + unitsStr );
}

//------------------------------------------------------------------------------
/*! \brief Parse a unitval from a single line of input.
 *
 *  Input is assumed to be in the format of [value],[units].  Extra whitespace is
 *  allowed.  Not having specified units (i.e. just the value) is currently allowed.
 *
 *  \param unitvalStr The string to be parsed into a unitval.
 *  \param expectedUnits The expected units that the input should be in.
 *  \exception h_exception An exception may be raised for the following reasons:
 *      - The value string could not be converted to a number.
 *      - The units string (if set) is not recognized by unitsName.
 *      - The units string (if set) does not match the expected units.
 */
unitval unitval::parse_unitval( const string& unitvalStr,
                               const unit_types& expectedUnits ) throw( h_exception )
{
    // we are assuming that should units exist they will be in the format of:
    // [value],[units]
    string::const_iterator commaPos = find( unitvalStr.begin(), unitvalStr.end(), ',' );
    
    string valueStr = string( unitvalStr.begin(), commaPos );
    string unitsStr = commaPos == unitvalStr.end() ? string() : string( commaPos + 1, unitvalStr.end() );
    
    // remove extra whitespace
    trim( valueStr );
    trim( unitsStr );
    
    return parse_unitval( valueStr, unitsStr, expectedUnits );
}

//------------------------------------------------------------------------------
/*! \brief Parse a unitval given a value string and units string.
 *
 *  Having an empty unitsStr is currently allowed in which case the units will be
 *  set to expectedUnits.
 *
 *  \param valueStr The numerical part of unitval to be parsed.
 *  \param unitsStr The (optional) units part of unitval to be parsed.
 *  \param expectedUnits The expected units that the unitval should be in.
 *  \exception h_exception An exception may be raised for the following reasons:
 *      - The value string could not be converted to a number.
 *      - The units string (if set) is not recognized by unitsName.
 *      - The units string (if set) does not match the expected units.
 */
unitval unitval::parse_unitval( const string& valueStr, const string& unitsStr,
                               const unit_types& expectedUnits ) throw( h_exception )
{
    // Double check if a user really provided a single unitval string in which
    // case it should be re-parsed before converting.
    if( unitsStr.empty() && find( valueStr.begin(), valueStr.end(), ',' ) != valueStr.end() ) {
        return parse_unitval( valueStr, expectedUnits );
    }
    
    double value;
    unit_types units;
    
    try {
        // parse the numerical value of the unitval
        value = lexical_cast<double>( valueStr );
    } catch( bad_lexical_cast& castException ) {
        H_THROW( "Could not convert value "+valueStr+", exception: "+castException.what() );
    }
    
    if( unitsStr.empty() ) {
        // we are currently allowing input files to not have to specify units and
        // are assuming they are in the expected units
        units = expectedUnits;
    } else {
        // if units are given then they must match the expected units
        units = parseUnitsName( unitsStr );
        H_ASSERT( units == expectedUnits, "Units: "+unitsStr+" do not match expected: "+unitsName( expectedUnits ) );
    }
    
    return unitval( value, units );
}

//------------------------------------------------------------------------------
/*! \brief Check if unit is correct or set it if no unit was given originally.
 *
 *  \param expectedUnits The expected units that the unitval should be in.
 *  \exception h_exception An exception may be raised for the following reasons:
 *      - The originally set units do not match the expected units.
 */
void unitval::expecting_unit( const unit_types& expectedUnits ) throw( h_exception ) {
    if (valUnits == U_UNDEFINED) {
        valUnits = expectedUnits;
    } else {
        H_ASSERT( valUnits == expectedUnits, "Units: "+unitsName( valUnits )
                  +" do not match expected: "+unitsName( expectedUnits ) );
    }
}

}
