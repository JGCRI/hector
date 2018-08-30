/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef INI_TO_CORE_READER_H
#define INI_TO_CORE_READER_H
/*
 *  ini_to_core_reader.h
 *  hector
 *
 *  Created by Pralit Patel on 11/1/10.
 *
 */

#include "h_exception.hpp"

namespace Hector {

class Core;

/*! \brief An adaptor class to send data read from an INI file directly to the
 *         core for routing to the proper model subcomponent.
 *
 *  Additional processing is done by this class to allow for reading time series
 *  data into the core.  Two strategies are allow and the core will remain ignorant
 *  about these:
 *      - The variable name can contain square brackets enclosing an index for
 *        example: variableName[2000] = 5.0
 *      - The variable value has a special identifier followed by a file name
 *        such as: variableName = csv:input/table.csv see CSVTableReader
 */
class INIToCoreReader {
    public:
    INIToCoreReader( Core* core );
    ~INIToCoreReader();

    void parse( const std::string& filename ) throw ( h_exception );

    private:
    //! Weak reference to a Core object that will handle parsed values
    Core* core;

    //! Path of the INI file
    std::string iniFilePath;

    //! The exception set by value handler should an exception occur.
    //! Note that this would only be valid if valueHandler returned
    //! an error code.
    h_exception valueHandlerException;
    
    static int valueHandler( void* user, const char* section, const char* name,
                             const char* value);
    
    typedef std::string::const_iterator StringIter;
    static double parseTSeriesIndex( const StringIter startBracket, 
                                     const StringIter endBracket,
                                     const StringIter strEnd );
};

}

#endif // INI_TO_CORE_READER_H
