/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  csv_table_reader.cpp
 *  hector
 *
 *  Created by Pralit Patel on 1/12/11.
 *
 */

#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <errno.h>

#include "core/core.hpp"
#include "data/message_data.hpp"
#include "input/csv_table_reader.hpp"

namespace Hector {
  
using namespace std;

//------------------------------------------------------------------------------
/*! \brief Constructor
 *
 *  Attempts to open the given file name.
 *
 *  \param fileName The name of a csv file to read from.
 *  \exception h_exception If there were errors when opening the file.
 */
CSVTableReader::CSVTableReader( const string& fileName ) throw ( h_exception )
:fileName( fileName )
{
    // allow exceptions from bad io operations
    tableInputStream.exceptions( ifstream::failbit | ifstream:: badbit );
    
    try {
        // attempt to open the file
        tableInputStream.open( fileName.c_str() );
    } catch( ifstream::failure e ) {
        // the macro errno in combination with strerror seem to be much more
        // informative than error message from the exception
        string errorStr = "Could not open csv file: "+fileName+" error: "+strerror(errno);
        H_THROW( errorStr );
    }
    lineNum = 0;
}

//------------------------------------------------------------------------------
/*! \brief Destructor
 *
 *  Closes the input stream.
 */
CSVTableReader::~CSVTableReader() {
    try {
        tableInputStream.close();
    } catch( ifstream::failure e ) {
        // ignoring close errors
    }
}


//------------------------------------------------------------------------------
/*! \brief Helper function
 *
 *  Get the next line that doesn't start with a semicolon.
 */
string CSVTableReader::csv_getline() {
    string s;
    bool semicolon = true;
    bool hash = true;
    
    while( ( semicolon || hash ) && !tableInputStream.eof() && tableInputStream.peek() != -1 ) {
        getline( tableInputStream, s ); //TODO: this depends on Unix line endings
        ++lineNum;
        semicolon = s[ 0 ] == ';';
        hash = s[ 0 ] == '#';
    }
    return s;
}

//------------------------------------------------------------------------------
/*! \brief Process the CSV file looking for the given varName and route the data
 *         into the core.
 *
 *  The input stream will be reset to allow to processing multiple times from
 *  this reader.  Next the header row is read and searched to find the column
 *  which varName is contained in.  Then each row of the table is read.  Should the
 *  the first column be UNITS it will use the row to set the units string to pass
 *  along with read data to provide units checking.  Otherwise it will assume
 *  the first column is the time series index and consistent columns for each
 *  row. Each value will be routed through the core as rows are read.  Extra
 *  white space will be removed before sending the value into the core.
 *
 *  \param core A pointer to the model core to route data through.
 *  \param componentName The model component to set varName in.
 *  \param varName The variable name to look for in the CSV file and set.
 *  \exception h_exception For any I/O errors, improper formatting, and inability
 *                         to find varName.  Also any errors while trying to
 *                         setData will also be propagated.
 */

void CSVTableReader::process( Core* core, const string& componentName,
                             const string& varName ) throw ( h_exception )
{
    using namespace boost;
    try {
        // reset the stream in case we are re-reading from this stream
        tableInputStream.clear();
        tableInputStream.seekg( 0, ifstream::beg );
        lineNum = 0;
        
        string line;
        vector<string> row;
        string unitsLabel;
        size_t columnIndex = 0; // code for "not found", takes advantage of skipping col==0 in the loop below.
        
        // read the header line and attempt to find varName. The first column is
        // not considered because that should be the index column.
        line = csv_getline();
        H_ASSERT( !line.empty(), "line empty" );
        split( row, line, is_any_of( "," ) );
//        const int headerRowSize = row.size();
        for( size_t col = 1; col < row.size() && columnIndex == 0; ++col ) {
            // ignore white space before comparing variable names
            trim( row[ col ] );
            if( row[ col ] == varName ) {
                columnIndex = col;
            }
        }
        if( columnIndex == 0 ) {
            H_THROW( "Could not find a column for "+varName+" in "+fileName+" header="+line );
        }
        
        // we are all set to process the table
        // note that getline sets the fail bit when it hits eof which is not what
        // want, a work around is to check peek
        int lines_read = 0;
        while( !tableInputStream.eof() && tableInputStream.peek() != -1 ) {
            // read the next row to process
            line = csv_getline();
            lines_read++;
            
            // Ignore blank lines. A stray windows line ending which may have made
            // its way in from a mixed line ending file can be skipped as well.
            if( line.empty() || line[ 0 ] == '\r' ) {
                continue;
            }
            
            split( row, line, is_any_of( "," ) );
            
            // we should have a constant number of columns
//TODO: figure this out for VS!  H_ASSERT( columnIndex < row.size() && headerRowSize == row.size(), "varying columns in data line "+ boost::lexical_cast<std::string>( lines_read ) );
            
            // remove extra white space from the table value
            trim( row[ 0 ] );
            trim( row[ columnIndex ] );
            
            if( row[ 0 ] == "UNITS" ) {
                // this row of the table is specifying units for all columns
                // we only need to keep track of the value for the column of interest
                unitsLabel = row[ columnIndex ];
            } else {
                // this row is a regular row of data
                // the first column is assumed to be the index
                double tseriesIndex = lexical_cast<double>( row[ 0 ] );
                
                // route the data to the appropriate model component
                if( !row[ columnIndex ].empty() ) {      // ignore blanks
                    message_data data( row[ columnIndex ] );
                    data.date = tseriesIndex;
                    data.units_str = unitsLabel;
                    core->setData( componentName, varName, data );
                }
            }
        }
        
    } catch( ifstream::failure e ) {
        string errorStr = "I/O exception while processing "+fileName+" error: "+strerror(errno);
        H_THROW( errorStr );
    } catch( bad_lexical_cast& castException ) {
        H_THROW( "Could not convert index to double on line: "+lexical_cast<string>( lineNum )+", exception: "
                +castException.what() );
    }
    // h_exceptions from setData should just be passed along
}

}
