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
#ifndef CSV_OUTPUTSTREAM_VISITOR_H
#define CSV_OUTPUTSTREAM_VISITOR_H
/*
 *  csv_outputstream_visitor.h
 *  hector
 *
 *  Created by Ben on 1 March 2011.
 *
 */

#include <string>

#include "visitors/avisitor.hpp"

#define DELIMITER ","

namespace Hector {
  
/*! \brief A visitor which will report all results at each model period.
 */
class CSVOutputStreamVisitor : public AVisitor {
public:
    CSVOutputStreamVisitor( std::ostream& outputStream, const bool printHeader = true );
    ~CSVOutputStreamVisitor();
    
    virtual bool shouldVisit( const bool in_spinup, const double date );
    
    virtual void visit( Core* c );
    virtual void visit( ForcingComponent* c );
    virtual void visit( SimpleNbox* c );
    virtual void visit( HalocarbonComponent* c );
    virtual void visit( TemperatureComponent* c );
    virtual void visit( BlackCarbonComponent* c );
    virtual void visit( OrganicCarbonComponent* c );
    virtual void visit( slrComponent* c );
    virtual void visit( OceanComponent* c );
    virtual void visit( OzoneComponent* c );
    virtual void visit( OHComponent* c );
    virtual void visit( CH4Component* c );
	virtual void visit( N2OComponent* c );

private:
    //! The file output stream in which the csv output will be written to.
    std::ostream& csvFile;
    
    // Data retained while the visitor is operating
    double current_date;
    
    // Spin up Flag
    bool in_spinup;
        
    //! Current model date, stored as a string for output
    std::string datestring;
    
    //! Current model mode, stored as a string for output
    std::string spinupstring;
    
    //! Name of current run
    std::string run_name;
    
    //! Helper function: prints beginning of each output line
    std::string linestamp() ;

    //! pointers to other components and stuff
    Core*             core;
};

}

#endif // CSV_OUTPUTSTREAM_VISITOR_H
