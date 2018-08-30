/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
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

#include "INIReader.h"
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
