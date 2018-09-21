/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  test_dependency_finder.cpp
 *  hector
 *
 *  Created by Pralit Patel on 3/28/11.
 *
 */

#include <iostream>
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <algorithm>

#include "h_exception.hpp"
#include "dependency_finder.hpp"

using namespace std;

/*! \brief Unit tests for the dependency finder.
 *
 */
class TestDependencyFinder : public testing::Test {
public:
    TestDependencyFinder()
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
};

TEST_F(TestDependencyFinder, SimpleLinear) {
    DependencyFinder depFinder;
    // Create a simple linear set of dependencies.
    // a -> b -> c
    depFinder.addDependency( "a", "b" );
    depFinder.addDependency( "b", "c" );

	ASSERT_NO_THROW( depFinder.createOrdering() );
    
    // We expect the ordering in reverse:
    vector<string> expectedOrdering( 3 );
    expectedOrdering[ 0 ] = "c";
    expectedOrdering[ 1 ] = "b";
    expectedOrdering[ 2 ] = "a";
    
    vector<string> ordering = depFinder.getOrdering();
    
    ASSERT_EQ( expectedOrdering.size(), ordering.size() );
    
    for( int i = 0; i < ordering.size(); ++i ) {
        EXPECT_EQ( expectedOrdering[ i ], ordering[ i ] );
    }
}

TEST_F(TestDependencyFinder, CycleFails) {
    DependencyFinder depFinder;
    // Create a cycle.
    //   a  ->  b
    //   ^     /
    //    \   V
    //      c
    depFinder.addDependency( "a", "b" );
    depFinder.addDependency( "b", "c" );
    depFinder.addDependency( "c", "a" );
    
	EXPECT_THROW( depFinder.createOrdering(), h_exception );
}

TEST_F(TestDependencyFinder, TwoSeperate) {
    DependencyFinder depFinder;
    // Create two sets of dependencies that are not related to each other.
    // a -> b
    // c -> d
    depFinder.addDependency( "a", "b" );
    depFinder.addDependency( "c", "d" );
    
	ASSERT_NO_THROW( depFinder.createOrdering() );
    
    vector<string> ordering = depFinder.getOrdering();
    
    typedef vector<string>::const_iterator iter;
    
    iter aIt = find( ordering.begin(), ordering.end(), "a" );
    ASSERT_TRUE( aIt != ordering.end() );
    iter bIt = find( ordering.begin(), ordering.end(), "b" );
    ASSERT_TRUE( bIt != ordering.end() );
    iter cIt = find( ordering.begin(), ordering.end(), "c" );
    ASSERT_TRUE( cIt != ordering.end() );
    iter dIt = find( ordering.begin(), ordering.end(), "d" );
    ASSERT_TRUE( dIt != ordering.end() );
    
    // expect that b is before a
    EXPECT_LT( bIt - ordering.begin(), aIt - ordering.begin() ) << "Ordering: " << ordering << endl;
    // expect that d is before c
    EXPECT_LT( dIt - ordering.begin(), cIt - ordering.begin() ) << "Ordering: " << ordering << endl;
    
    // can't say anything about the other relationships
}

TEST_F(TestDependencyFinder, MultipleDependencies) {
    DependencyFinder depFinder;
    // Create a scenario where something is dependent on two objects and
    // two objects are dependant on one object
    //   a
    //  / \
    // V   V
    // b   c
    // \   /
    //  V V
    //   d
    depFinder.addDependency( "a", "b" );
    depFinder.addDependency( "a", "c" );
    depFinder.addDependency( "b", "d" );
    depFinder.addDependency( "c", "d" );
    
	ASSERT_NO_THROW( depFinder.createOrdering() );
    
    vector<string> ordering = depFinder.getOrdering();
    
    typedef vector<string>::const_iterator iter;
    
    iter aIt = find( ordering.begin(), ordering.end(), "a" );
    ASSERT_TRUE( aIt != ordering.end() );
    iter bIt = find( ordering.begin(), ordering.end(), "b" );
    ASSERT_TRUE( bIt != ordering.end() );
    iter cIt = find( ordering.begin(), ordering.end(), "c" );
    ASSERT_TRUE( cIt != ordering.end() );
    iter dIt = find( ordering.begin(), ordering.end(), "d" );
    ASSERT_TRUE( dIt != ordering.end() );
    
    // expect that b is before a
    EXPECT_LT( bIt - ordering.begin(), aIt - ordering.begin() ) << "Ordering: " << ordering << endl;
    // expect that c is before a
    EXPECT_LT( cIt - ordering.begin(), aIt - ordering.begin() ) << "Ordering: " << ordering << endl;
    // expect that d is before b
    EXPECT_LT( dIt - ordering.begin(), bIt - ordering.begin() ) << "Ordering: " << ordering << endl;
    // expect that d is before c
    EXPECT_LT( dIt - ordering.begin(), cIt - ordering.begin() ) << "Ordering: " << ordering << endl;
}

ostream& operator<<( ostream& out, const vector<string>& vec ) {
    if( vec.empty() ) {
        return out << "[]";
    }
    out << "[";
    vector<string>::const_iterator it = vec.begin();
    for( ; it != vec.end()-1; ++it ) {
        out << *it <<  ", ";
    }
    return out << *it << "]";
}
