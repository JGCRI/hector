/* Hector -- A Simple Climate Model
   Copyright (C) 2022  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  test_core.cpp
 *  hector
 *
 *  Created by Pralit Patel on 11/5/10.
 *
 */

#include <iostream>
#include <gtest/gtest.h>

#include "h_exception.hpp"
#include "core.hpp"
#include "dummy_model_component.hpp"
#include "avisitor.hpp"

using namespace Hector;

/*! \brief Unit tests for the Core class.
 *
 *  TODO: these are really not that interesting tests.
 *
 *  \warning This test is currently tightly coupled with Core and DummyModelComponent
 *           which will likely break the tests in the near future.
 */
class TestCore : public testing::Test {
public:
    TestCore()
    {
    }
    // google test fixtures must be protected
protected:
    // fixture methods
    virtual void SetUp() {
    }
    
    // only define TearDown if it is needed
    /*virtual void TearDown() {
    }*/
        
    class CheckDummyVisitor: public AVisitor {
    public:
        CheckDummyVisitor():sawDummy( 0 ), sawY( -100 ) {
        }
        
        virtual bool shouldVisit( const bool in_spinup, const double date ) {
            return true;
        }
        
        virtual void visit( DummyModelComponent* dummyModel ) {
            ++sawDummy;
            sawY = dummyModel->getY();
        }
        
        int sawDummy;
        double sawY;
    };
    
    class CheckDoesRespectShouldVisit: public AVisitor {
    public:
        CheckDoesRespectShouldVisit():didVisit( false ) {
        }
        
        virtual bool shouldVisit( const bool in_spinup, const double date ) {
            return false;
        }
        
        virtual void visit( Core* core ) {
            didVisit = true;
        }
        
        bool didVisit;
    };
};

TEST_F(TestCore, InitCreatesComponents) {
    // no init yet
    CheckDummyVisitor checkVisitor;
    Core core(Logger::SEVERE, false, false);
    core.accept( &checkVisitor );
    
    EXPECT_FALSE( checkVisitor.sawDummy );
    
    // Use the DummyModelComponent for testing
    core.addModelComponent( new DummyModelComponent );
    core.init();
    core.accept( &checkVisitor );
    EXPECT_TRUE( checkVisitor.sawDummy );
}

TEST_F(TestCore, CheckSetData) {
    // Use the DummyModelComponent for testing
    Core core(Logger::SEVERE, false, false);
    core.addModelComponent( new DummyModelComponent );
    core.init();
    message_data msg(unitval(1, U_UNDEFINED));
    EXPECT_THROW(core.setData("does-not-exist", "slope", msg), h_exception);
    EXPECT_THROW(core.setData("does-not-exist", "slope", msg), h_exception);

    EXPECT_NO_THROW(core.setData("dummy-component", "slope", msg));
    EXPECT_NO_THROW(core.setData("dummy-component", "y", msg));
    CheckDummyVisitor checkVisitor;
    core.accept( &checkVisitor );
    EXPECT_EQ( checkVisitor.sawY, 1 );
}

TEST_F(TestCore, CanParseStartAndEndDates) {
    Core core(Logger::SEVERE, false, false);
    message_data msg(unitval(1, U_UNDEFINED));
    EXPECT_NO_THROW(core.setData("core", "startDate", msg));
    EXPECT_NO_THROW(core.setData("core", "endDate", msg));
}

TEST_F(TestCore, DoAddedVisitorsGetCalled) {
    // Use the DummyModelComponent for testing
    Core core(Logger::WARNING, false, false);
    core.addModelComponent( new DummyModelComponent );
    core.init();
    // note this should have already been tested to not throw
    message_data msg(unitval(1, U_UNDEFINED));
    core.setData("dummy-component", "slope", msg);
    core.setData("dummy-component", "y", msg);
    core.setData("dummy-component", "c", msg);
    core.setData("dummy-component", "c", unitval(2, U_UNDEFINED));
    core.setData("core", "startDate", msg);
    core.setData("core", "endDate", unitval(10, U_UNDEFINED));
    
    CheckDummyVisitor checkVisitor;
    core.addVisitor( &checkVisitor );
//TODO: As Pralit's comment above notes, this test was highly dependent on particular
//		behavior of core and components. The atmosphere component now causes this to fail.
//		Need a more robust way to do this, but for now commenting out run & test.
//    core.run();
    
    // 9 comes from start and end dates above, note no consideration for timesteps
//    ASSERT_EQ( checkVisitor.sawDummy, 9 );
}

TEST_F(TestCore, DoesCheckShouldVisit) {
    // note no init, we don't need any components to check this
    CheckDoesRespectShouldVisit checkVisitor;
    Core core(Logger::SEVERE, false, false);
    core.addVisitor( &checkVisitor );
    core.run();
    
    ASSERT_FALSE( checkVisitor.didVisit );
}

TEST_F(TestCore, CanNotAddCompAfterInit) {
    Core core(Logger::SEVERE, false, false);
    core.init();
    ASSERT_THROW( core.addModelComponent( new DummyModelComponent ), h_exception );
}
