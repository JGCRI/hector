/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  h_reader.cpp
 *  hector
 *
 *  Created by Ben on 10/8/10.
 *
 */

#include <sstream>

#include "h_exception.hpp"
#include "h_reader.hpp"
#include "logger.hpp"

namespace Hector {

/*! \brief Constructor for h_reader.
 *
 *  Backend is currently an INIReader.
 */
h_reader::h_reader( std::string fname, readertype_t style, bool doparse ) {
    filename = fname;
    reader = NULL;
    if( doparse )
        parse();
}

/*! \brief Parse an INI-style file.
 *
 *  Backend is currently an INIReader.
 */
void h_reader::parse() throw( h_exception ) {
    if(reader)
        delete reader;
    
    reader = new INIReader( filename );
    int le = (*reader).ParseError();
    if( le ) {
        std::stringstream out;
        out << "Parse error in file " << filename << "  line: " << le;
        H_THROW(out.str());
    }
}

/*! \brief Search for and return a string.
 *
 *  Backend is currently an INIReader.
 */
std::string h_reader::get_string( std::string section, std::string name, std::string defaultvalue ) {
    return (*reader).Get( section, name, defaultvalue );
}

/*! \brief Search for and return a number.
 *
 *  Backend is currently an INIReader.
 */
double h_reader::get_number( std::string section, std::string name, double defaultvalue ) {
    return (*reader).GetInteger( section, name, defaultvalue );
}

}
