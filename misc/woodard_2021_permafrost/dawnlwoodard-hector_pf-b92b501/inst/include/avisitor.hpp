/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef AVISITOR_H
#define AVISITOR_H
/*
 *  avisitor.h
 *  hector
 *
 *  Created by Pralit Patel on 10/29/10.
 *
 */

namespace Hector {
  
// Forward declare all visitable subclasses.
class Core;
class DummyModelComponent;
class ForcingComponent;
class slrComponent;
class HalocarbonComponent;
class SimpleNbox;
class CarbonCycleSolver;
class CH4Component;
class OHComponent;
class N2OComponent;
class TemperatureComponent;
class BlackCarbonComponent;
class OrganicCarbonComponent;
class OceanComponent;
class OneLineOceanComponent;
class SulfurComponent;
class OzoneComponent;

//------------------------------------------------------------------------------
/*! \brief AVisitor abstract class provides a base for subclasses to visit only
 *         the IVisitable subclasses that they are interested in.
 */
class AVisitor {
public:
    inline virtual ~AVisitor();
    
    //------------------------------------------------------------------------------
    /*! \brief Determine if the visitor needs to collect data at the given model
     *         date.
     *  \param date The model date which just finished solving.
     *  \return True if the visitor wants to visit at date.
     */
    virtual bool shouldVisit( const bool in_spinup, const double date ) = 0;
    
    //------------------------------------------------------------------------------
    // Add a visit for all visitable subclasses here.
    // TODO: should we create a .cpp for these?
    virtual void visit( Core* core ) {}
    virtual void visit( DummyModelComponent* c ) {}
    virtual void visit( ForcingComponent* c ) {}
    virtual void visit( slrComponent* c ) {}
    virtual void visit( CarbonCycleSolver* c ) {}
    virtual void visit( SimpleNbox* c ) {}
    virtual void visit( HalocarbonComponent* c ) {}
    virtual void visit( OHComponent* c ) {}
    virtual void visit( CH4Component* c ) {}
    virtual void visit( N2OComponent* c ) {}
    virtual void visit( TemperatureComponent* c ) {}
    virtual void visit( BlackCarbonComponent* c ) {}
    virtual void visit( OrganicCarbonComponent* c ) {}
    virtual void visit( OceanComponent* c ) {}
    virtual void visit( OneLineOceanComponent* c ) {}
	virtual void visit( SulfurComponent* c ) {}
	virtual void visit( OzoneComponent* c ) {}
};

// Inline methods
AVisitor::~AVisitor() {
}

}

#endif // AVISITOR_H
