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
 *  h_input.h
 *  hector
 *
 *  Created by Ben on 10/8/10.
 *  Copyright 2010 DOE Pacific Northwest Lab. All rights reserved.
 *
 */

#ifndef H_READER_HPP_
#define H_READER_HPP_

#include "inih/INIReader.h"
#include "h_exception.hpp"

namespace Hector {

enum readertype_t { INI_style, table_style };

/*! \brief Class to read input data of various types.
 *
 *  Backend is currently an INIReader.
 */
class h_reader {
public:
    h_reader( std::string fname, readertype_t style, bool doparse=true );
    virtual ~h_reader() {delete reader;}
    void parse() throw( h_exception );
    std::string get_string( std::string section, std::string name, std::string defaultvalue );
    double get_number( std::string section, std::string name, double defaultvalue );
private:
    INIReader* reader;
    std::string filename;
};

}

#endif
