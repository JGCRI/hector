#ifndef INI_RESTART_VISITOR_H
#define INI_RESTART_VISITOR_H
/*
 *  ini_restart_visitor.h
 *  hector
 *
 *  Created by Pralit Patel on 11/1/10.
 *
 */

#include <string>

#include "visitors/avisitor.hpp"

namespace Hector {
  
/*! \brief A visitor which will attempt to gather all information to restart the
 *         model and write them in an INI style restart file.
 *  \todo This class strips all units from the output variables since unit input
 *        parsing has yet to be done.
 */
class INIRestartVisitor : public AVisitor {
public:
    INIRestartVisitor( const std::string& filename, const double restartDate );
    ~INIRestartVisitor();
    
    virtual bool shouldVisit( const bool in_spinup, const double date );
    
    virtual void visit( Core* c );
    virtual void visit( SimpleNbox* c );
    virtual void visit( ForcingComponent* c );
    virtual void visit( HalocarbonComponent* c );
    
private:
    //! The file output stream in which the restart file will be written to.
    std::fstream restartFile;
    
    //! The date in which to write a restart file.  TODO: This may need to be more
    //! sophisticated in the future.
    const double restartDate;
};

}

#endif // INI_RESTART_VISITOR_H
