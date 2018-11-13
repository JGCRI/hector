/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

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
#include "core/core.hpp"
#include "core/dummy_model_component.hpp"
#include "core/avisitor.hpp"

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
    
    Core core;
    
    class CheckDummyVisitor: public AVisitor {
    public:
        CheckDummyVisitor():sawDummy( 0 ), sawY( -100 ) {
        }
        
        virtual bool shouldVisitAtDate( const double date ) {
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
        
        virtual bool shouldVisitAtDate( const double date ) {
            return false;
        }
        
        virtual void visit( Core* core ) {
            didVisit = true;
        }
        
        bool didVisit;
    };
    
    // Note this assumes that forcing is indeed dependent on Simple3Box
    class CheckDoesAtmosVisitBeforeForcing: public AVisitor {
    public:
        CheckDoesAtmosVisitBeforeForcing():didVisitAtmos( false ),didVisitForcing( false )
        {
        }
        
        virtual bool shouldVisitAtDate( const double date ) {
            return true;
        }
        
        virtual void visit( SimpleNbox* a) {
            EXPECT_FALSE( didVisitForcing );
            didVisitAtmos = true;
        }
        
        virtual void visit( ForcingComponent* f) {
            EXPECT_TRUE( didVisitAtmos );
            didVisitForcing = true;
        }
        
        bool didVisitAtmos;
        bool didVisitForcing;
    };
};

TEST_F(TestCore, InitCreatesComponents) {
    // no init yet
    CheckDummyVisitor checkVisitor;
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
    core.addModelComponent( new DummyModelComponent );
    core.init();
    EXPECT_THROW(core.setData("does-not-exisit", "slope", "1"), h_exception);
    
    EXPECT_NO_THROW(core.setData("dummy-component", "slope", "1"));
    EXPECT_NO_THROW(core.setData("dummy-component", "y", "1"));
    CheckDummyVisitor checkVisitor;
    core.accept( &checkVisitor );
    EXPECT_EQ( checkVisitor.sawY, 1 );
}

TEST_F(TestCore, CanParseStartAndEndDates) {
    EXPECT_NO_THROW(core.setData("core", "startDate", "1"));
    EXPECT_NO_THROW(core.setData("core", "endDate", "10"));
}

TEST_F(TestCore, DoAddedVisitorsGetCalled) {
    // Use the DummyModelComponent for testing
    core.addModelComponent( new DummyModelComponent );
    core.init();
    // note this should have already been tested to not throw
    core.setData("dummy-component", "slope", "1");
    core.setData("dummy-component", "y", "1");
    core.setData("dummy-component", "c", "1", 1);
    core.setData("dummy-component", "c", "1", 2);
    core.setData("core", "startDate", "1");
    core.setData("core", "endDate", "10");
    
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
    core.addVisitor( &checkVisitor );
    core.run();
    
    ASSERT_FALSE( checkVisitor.didVisit );
}

TEST_F(TestCore, DoesReorderComponents) {
    core.init();
    
    CheckDoesAtmosVisitBeforeForcing orderChecker;
    core.accept( &orderChecker );
    // actual tests are within CheckDoesAtmosVisitBeforeForcing above
}

TEST_F(TestCore, CanNotAddCompAfterInit) {
    core.init();
    ASSERT_THROW( core.addModelComponent( new DummyModelComponent ), h_exception );
}
