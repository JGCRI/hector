/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef CSV_OUTPUT_VISITOR_H
#define CSV_OUTPUT_VISITOR_H
/*
 *  csv_output_visitor.h
 *  hector
 *
 *  Created by Pralit Patel on 11/4/10.
 *
 */

#include <string>

#include "component_data.hpp"
#include "core.hpp"
#include "unitval.hpp"
#include "h_util.hpp"
#include "avisitor.hpp"

namespace Hector {
  
/*! \brief A visitor which will report a few results at each model period.
 */
class CSVOutputVisitor : public AVisitor {
public:
    CSVOutputVisitor( const std::string& filename );
    ~CSVOutputVisitor();
    
    virtual bool shouldVisit( const bool in_spinup, const double date );
    
    virtual void visit( Core* c );
    
private:
    //! The file output stream in which the csv output will be written to.
    std::fstream csvFile;
    
    //! Store the current date for use while visiting.
    double currDate;
    
#define DELIMITER ","
};

}

#endif // CSV_OUTPUT_VISITOR_H
