/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef CSV_TRACKEDPOOL_VISITOR_H
#define CSV_TRACKEDPOOL_VISITOR_H
/*
 *  csv_trackedpool_visitor.h
 *  hector
 *
 *  Created by Skylar Gering on 21 January 2021.
 *
 */

#include <string>

#include "avisitor.hpp"

#define DELIMITER ","

namespace Hector{
    /*! \brief A visitor which will the contents of each tracked pool at each model period.
 */
class CSVTrackedPoolVisitor : public AVisitor {
public:
    CSVTrackedPoolVisitor( std::ostream& outputStream, const bool printHeader = true );
    ~CSVTrackedPoolVisitor();

    virtual bool shouldVisit( const bool in_spinup, const double date );
    virtual void visit( Core* c );
    virtual void visit( SimpleNbox* c );

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

#endif // CSV_TRACKEDPOOL_VISITOR_H
