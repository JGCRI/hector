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
    message_data(): date( Core::undefinedIndex() )
    {
    }
    
    // Create a message data to pass a date.
    message_data( const double d ): date( d )
    {
    }
    
    // Create a message data to pass a string.
    message_data( const std::string& value ): date( Core::undefinedIndex() ), value_str( value )
    {
    }
    
    // Create a message data to pass a unitval.
    message_data( const unitval& value ): date( Core::undefinedIndex() ), value_unitval( value )
    {
    }

    // Create message data with both date and value
    message_data(double d, const unitval&val) : date(d), value_unitval(val){}
    
    //! (optional) a date for which this value is for.
    double date;
    
    //! The message data as a string (if applicable).
    std::string value_str;

    //! The message data as a unitval (if applicable).
    unitval value_unitval;
    
    //! A string characterizing the units of value (if applicable).
    std::string units_str;
};

}

#endif
