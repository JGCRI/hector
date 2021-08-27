/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef CSV_TRACKING_VISITOR_H
#define CSV_TRACKING_VISITOR_H
/*
 *  csv_tracking_visitor.h
 *  hector
 *
 *  Created by Skylar Gering on 21 January 2021.
 *
 */

#include <string>

#include "avisitor.hpp"
#include "fluxpool.hpp"
#include "tseries.hpp"

#define DELIMITER ","

namespace Hector {
    /*! \brief A visitor which will the contents of each tracked pool at each model period.
 */
class CSVFluxPoolVisitor : public AVisitor {
public:
    CSVFluxPoolVisitor( ostream& outputStream, const bool printHeader = true );
    ~CSVFluxPoolVisitor();

    virtual bool shouldVisit( const bool in_spinup, const double date );
    virtual void visit( Core* c );
    virtual void visit( SimpleNbox* c );
    virtual void visit( OceanComponent* c );

    std::string get_buffer() const;
    void reset( const double reset_date );
    
private:
    //! The file output stream in which the csv output will be written to.
    std::ostream& csvFile;
    
    //! The buffer—holds output until ready to be returned to the core or written to the csv file
    tseries<string> csvBuffer;
    string header;
    
    // Data retained while the visitor is operating
    double current_date;
    double tracking_date;

    //! Current model date, stored as a string for output
    string datestring;

    //! Current model mode, stored as a string for output
    string spinupstring;

    //! Name of current run
    string run_name;

    //! Helper function: print one or more lines of tracking data for a given fluxpool
    virtual void print_pool( const fluxpool, const string );

    //! Pointers to other components and stuff
    Core*             core;
};

}

#endif // CSV_TRACKING_VISITOR_H
