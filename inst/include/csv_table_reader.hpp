/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef CSV_TABLE_READER_H
#define CSV_TABLE_READER_H
/*
 *  csv_table_reader.h
 *  hector
 *
 *  Created by Pralit Patel on 1/12/11.
 *
 */

#include <fstream>

#include "h_exception.hpp"

namespace Hector {

class Core;

/*! \brief A class responsible for reading time series data from a CSV file and
 *         routing this data through the core.
 *
 *  This class assumes the CSV file will be organized in the following way:
 *      - The first row is header information which corresponds to the actual
 *        variable name to be set.
 *      - The first column is the date index to use when setting the variable.
 *      - If first column is UNITS then the row is assumed to be the units labels
 *        for the table.  This units string will then be passed along with the data
 *        processed form subsequent rows to provide units checking.
 *
 *  When instructed to process the class requires routing information including
 *  the variable to set so that it can identify which column to process.  It will
 *  then route data as each row is read.  Note that the file will remain open
 *  and available for reprocessing until it is destructed.
 */
class CSVTableReader {
public:
    CSVTableReader( const std::string& fileName ) throw ( h_exception );
    ~CSVTableReader();
    
    void process( Core* core, const std::string& componentName,
                  const std::string& varName ) throw ( h_exception );
    
private:
    //! The file name to read data from.  Kept around for error reporting.
    const std::string fileName;
    
    //! Input stream to read data from.
    std::ifstream tableInputStream;
    
    //! Current line (that has just been read)
    int lineNum;
    
    // Helper function to find next non-commented line
    std::string csv_getline();

};

}

#endif // CSV_TABLE_READER_H
