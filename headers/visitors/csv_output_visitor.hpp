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

#include "components/component_data.hpp"
#include "core/core.hpp"
#include "data/unitval.hpp"
#include "h_util.hpp"
#include "visitors/avisitor.hpp"

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
