/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
#include "input/h_reader.hpp"
#include "core/logger.hpp"

namespace Hector {

/*! \brief Constructor for h_reader.
 *
 *  Backend is currently an INIReader.
 */
h_reader::h_reader( std::string fname, readertype_t style, bool doparse ) {
    Logger& glog = Logger::getGlobalLogger();
    H_LOG( glog, Logger::DEBUG) << "h_reader created for " << fname << std::endl;
    
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
    Logger& glog = Logger::getGlobalLogger();
    H_LOG( glog, Logger::NOTICE) << "Parsing " << filename << std::endl;
    if(reader)
      delete reader;
    
	reader = new INIReader( filename );
	int le = (*reader).ParseError();
	if( le ) {
		std::string s;
		std::stringstream out;
		out << le;
		s = out.str();
        H_LOG( glog, Logger::SEVERE) << "Parse error in file " << filename << " line " << s;
		H_THROW( "Parse error in file " + filename + " line " + s )
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
