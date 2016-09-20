/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  INIReader.cpp
 *  hector
 *
 *  Added by Ben on 10/7/10.
 *  Based on inih -- simple .INI file parser
 *
 *  inih is released under the New BSD license (see LICENSE.txt). Go to the project
 *  home page for more info:
 *
 *  http://code.google.com/p/inih/
 *
 */ 

// Read an INI file into easy-to-access name/value pairs.

#include <cctype>
#include <cstdlib>
#include <sstream>

#include "input/inih/ini.h"
#include "input/inih/INIReader.h"

namespace Hector {
  
using std::string;

INIReader::INIReader( string filename )
{
    _error = ini_parse(filename.c_str(), ValueHandler, this);
}

int INIReader::ParseError()
{
   return _error;
}

string INIReader::Get(string section, string name, string default_value)
{
    string key = MakeKey(section, name);
    return _values.count(key) ? _values[key] : default_value;
}

long INIReader::GetInteger(string section, string name, long default_value)
{
    string valstr = Get(section, name, "");
    const char* value = valstr.c_str();
    char* end;
    // This parses "1234" (decimal) and also "0x4D2" (hex)
    long n = strtol(value, &end, 0);
    return end > value ? n : default_value;
}

string INIReader::MakeKey(string section, string name)
{
    string key = section + "." + name;
    // Convert to lower case to make lookups case-insensitive
    for (unsigned i = 0; i < key.length(); i++)
        key[i] = tolower(key[i]);
    return key;
}

int INIReader::ValueHandler(void* user, const char* section, const char* name,
                            const char* value)
{
    INIReader* reader = (INIReader*)user;
    reader->_values[MakeKey(section, name)] = value;
    return 1;
}

}
