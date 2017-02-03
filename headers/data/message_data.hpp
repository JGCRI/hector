/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef MESSAGE_DATA_H
#define MESSAGE_DATA_H
/*
 *  message_data.h - A simple struct to hold data to be passed around with messages.
 *  hector
 *
 *  Created by Pralit on 7/12/13.
 *
 */

#include <string>

#include "core/core.hpp"
#include "data/unitval.hpp"

namespace Hector {
  
/*! \brief Message data type.
 *
 *  Contains fields for all the types of data that may be passed around with
 *  messages in hector.
 */
struct message_data {
    // Some constructors that can help with syntax.
    message_data(): date( Core::undefinedIndex() ), isVal(false)
    {
    }
    
    // Create a message data to pass a date.
    message_data( const double d ): date( d ), isVal(false)
    {
    }
    
    // Create a message data to pass a string.
    message_data( const std::string& value ): date( Core::undefinedIndex() ), value_str( value ), isVal(false)
    {
    }
    
    // Create a message data to pass a unitval.
    message_data( const unitval& value ): date( Core::undefinedIndex() ), value_unitval( value ), isVal(true)
    {
    }

    // create a message data to pass unitval and date
    message_data(double d, const unitval &val) : date(d), value_unitval(val),
                                                 isVal(true) {}

    //------------------------------------------------------------------------------
    /*! \brief retrieve message data as a unitval, even if it contains a string.
     *
     *  \param expectedUnits The expected units that the unitval should be in.
     *  \param strict  Flag indicating if units have to be given (default: false).
     *  \exception h_exception An exception may be raised for the following reasons:
     *      - The unitval units do not match the expected units.
     *      - The value string could not be parsed.
     */
    unitval getUnitval(const unit_types& expectedUnits, const bool strict = false) const throw ( h_exception ) {
        if(isVal) {
            if (strict) {
                H_ASSERT( value_unitval.units() == expectedUnits, "Units: "+value_unitval.unitsName()+" do not match expected: "+unitval::unitsName( expectedUnits ) );
            }
            unitval result = value_unitval; // copy to not change value_unitval of message
            result.expecting_unit(expectedUnits);
            return result;
        } else {
            if (strict && units_str.empty() && expectedUnits != U_UNDEFINED) {
                H_THROW( "Units: '' do not match expected: "+unitval::unitsName( expectedUnits ) );
            }
            return unitval::parse_unitval(value_str, units_str, expectedUnits);
        }
    }
    //! (optional) a date for which this value is for.
    double date;
    
    //! The message data as a string (if applicable).
    std::string value_str;

    //! The message data as a unitval (if applicable).
    unitval value_unitval;
    
    //! A string characterizing the units of value (if applicable).
    std::string units_str;

    //! Flag indicating whether the unitval is set
    bool isVal;
};

}

#endif
