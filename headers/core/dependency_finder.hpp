/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef _DEPENDENCY_FINDER_H_
#define _DEPENDENCY_FINDER_H_
#if defined(_MSC_VER)
#pragma once
#endif

/*
 *  dependency_finder.hpp
 *  hector
 *
 *  Created by Pralit Patel.
 *
 */

#include <vector>
#include <map>
#include <list>
#include <string>

#include "h_exception.hpp"

namespace Hector {

/*! 
* \brief This class calculates an ordering of named objects through a scheduling
*        algorithm based on supplied dependencies.
* \details This class can order any type of object that has a name represented by
*          a string. To use this class to create a correct object ordering,
*          the user should first use the addDependency function for each
*          dependency of a object. Once all dependencies have been added,
*          calling createOrdering constructs an internal ordering. Currently cycles
*          will generate and exception.  Additional dependencies can be added once
*          the ordering is created, but createOrdering must be called again. Once
*          createOrdering has completed, the user can then call getOrdering to
*          return an ordered list of objects, starting with the object which has
*          no dependencies.
*          The two key data structures are mDependencyMatrix and mObjectIndices.
*          mDependencyMatrix is a two-dimensional matrix where each cell
*          represents whether there is a dependency from the column object to
*          the row object. mObjectIndices contains a mapping of object name to
*          the index in mDependencyMatrix. Both of these data structures are
*          updated by the addDependency function.
* \author Josh Lurz
*/

class DependencyFinder
{
public:
    DependencyFinder();
    bool addDependency( const std::string& aObjectName,
                        const std::string& aDependency );
    void createOrdering() throw ( h_exception );
    const std::vector<std::string>& getOrdering() const;
private:
    void removeDependency( const size_t aObject, const size_t aDependency );
    const std::string& getNameFromIndex( const size_t aIndex ) const;
    
    // Need the typedef here as addTrackedItem returns a ObjectIndexMap
    // iterator.
    typedef std::map<const std::string, size_t> ObjectIndexMap;   
    ObjectIndexMap::iterator addTrackedItem( const std::string& aItem ); 
    
    //! Mapping of sector name to matrix index to avoid storing all names as
    //! strings within the matrix.
    ObjectIndexMap mObjectIndices;

    //! A square matrix where each cell contains whether the column sector
    //! depends on the row sector.
    std::vector<std::vector<bool> > mDependencyMatrix;
    
    //! The correctly ordered list of sectors.
    std::vector<std::string> mOrdering;
};

}

#endif // _DEPENDENCY_FINDER_H_
