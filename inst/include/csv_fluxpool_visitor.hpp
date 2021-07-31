/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef CSV_FLUXPOOL_VISITOR_H
#define CSV_FLUXPOOL_VISITOR_H
/*
 *  csv_fluxpool_visitor.h
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
    CSVFluxPoolVisitor( std::ostream& outputStream, const bool printHeader = true );
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
    tseries<std::string> csvBuffer;
    std::string header;
    
    // Data retained while the visitor is operating
    double current_date;
    double tracking_date;

    //! Current model date, stored as a string for output
    std::string datestring;

    //! Current model mode, stored as a string for output
    std::string spinupstring;

    //! Name of current run
    std::string run_name;

    //! Helper function: print a line to the output stream
    virtual void print_pool(fluxpool);

    //! Temporary helper to print differences in flux from calcDerives
    virtual void print_diff(double, string);
    
    //! pointers to other components and stuff
    Core*             core;
};

}

#endif // CSV_FLUXPOOL_VISITOR_H
