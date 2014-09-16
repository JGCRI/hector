/*
 *  dependency_finder.cpp
 *  hector
 *
 *  Created by Pralit Patel.
 *
 */

//#include <cassert>
#include <algorithm>
#include <limits.h>

#include "core/dependency_finder.hpp"
#include "h_exception.hpp"
#include "core/logger.hpp"

using namespace std;

/*!
 * \brief Constructor.
 */
DependencyFinder::DependencyFinder()
{
}

/*!
 * \brief Add a dependency for an object.
 * \details This function is used to mark a single dependency, from aObjectName
 *          to aDependency.
 * \param aObjectName Name of the object which has a new dependency.
 * \param aDependency name of the item aObjectName is dependent on.
 * \return Whether the dependency was added to the matrix.
 */
bool DependencyFinder::addDependency( const string& aObjectName,
                                     const string& aDependency ){
    // Check if the object is already in the mapping of object name to matrix index.
    ObjectIndexMap::iterator objectLocation = mObjectIndices.find( aObjectName );
    if( objectLocation == mObjectIndices.end() ){
        // Update the object location iterator after the item is added.
        objectLocation = addTrackedItem( aObjectName );
    }
    
    // Check if the dependency is already in the mapping.
    ObjectIndexMap::iterator dependencyLocation = mObjectIndices.find( aDependency );
    if( dependencyLocation == mObjectIndices.end() ){
        // Update the dependency location iterator after the item is added.
        dependencyLocation = addTrackedItem( aDependency );
    }
    
    // The matrix is now set up correctly, add the dependency.
    H_ASSERT( mDependencyMatrix.size() > objectLocation->second, "addDependency failure" );
    H_ASSERT( mDependencyMatrix[ objectLocation->second ].size() > dependencyLocation->second, "addDependency failure" );
    
    // Check if the dependency already exists.
    if( mDependencyMatrix[ objectLocation->second ][ dependencyLocation->second ] ){
        return false;
    }
    // Add the dependency and return that it was a new dependency.
    mDependencyMatrix[ objectLocation->second ][ dependencyLocation->second ] = true;
    return true;
}

/*!
 * \brief Find an ordering of the objects in the dependency finder which orders
 *        each object before each object that depends on it.
 * \details This is referred to as a topological sort. The algorithm is as
 *          follows: Search the adjacency matrix for a vertice, in this
 *          implementation a column in the matrix, with no dependencies. If there
 *          is none, the graph has a cycle and and exception will be thrown.
 *          Next, add the vertice found in the previous step to the ordering, and
 *          remove it and all dependencies on it from the adjacency matrix. Start
 *          over at the first step. Repeat this process until there are no nodes
 *          left in the graph.  Once this function has been called, the caller may
 *          then call getOrdering to return the ordered vector.
 *
 * \exception h_exception If there is a cycle in the dependencies.
 * \note This function removes nodes by setting the removed flag to true instead
 *       of actually removing the vertice.
 * \pre All relevant objects must have called addDependency() to add their dependencies to the matrix.
 * \todo The control flow of this function could be less confusing.
 * \sa getOrdering
 */
void DependencyFinder::createOrdering() throw ( h_exception ) {
    // If there is an existing stored ordering, clear it.
    mOrdering.clear();
    
    // Create a vector which marks which vertices are removed.
    vector<bool> removed( mDependencyMatrix.size() );
    
    // Search until the ordering contains all vertices in the matrix.
    while( mOrdering.size() < mDependencyMatrix.size() ){
        unsigned int verticeToRemove = INT_MAX;
        
        // Search for a vertex with no dependencies.
        for( unsigned int i = 0; i < mDependencyMatrix.size(); ++i ){
            // Only search the vertex if it has not been removed.
            if( removed[ i ] ){
                continue;
            }
            // Check for any dependencies.
            bool depFound = false;
            for( unsigned int j = 0; j < mDependencyMatrix.size(); ++j ){
                // Check if there is a dependency at this location.
                if( !removed[ j ] && mDependencyMatrix[ i ][ j ] ){
                    // Found a dependency so break out of the loop to stop
                    // searching this column and move onto the next.
                    depFound = true;
                    break;
                }
            }
            // If we did not find a dependency, set the index to remove and
            // break the loop so the object can be removed. Otherwise continue
            // searching.
            if( !depFound ){
                verticeToRemove = i;
                break;
            }
        }
        
        // Check if we found a vertex to remove.
        if( verticeToRemove == INT_MAX ){
            // Since there was no vertex with zero dependencies, this graph has
            // a cycle.
            H_THROW( "Could not sort dependencies; there is a cycle in the graph." );
        }
        else {
            // Add the vertex found to the ordering and remove it from the
            // matrix.
            mOrdering.push_back( getNameFromIndex( verticeToRemove ) );
            removed[ verticeToRemove ] = true;
        }
    }
    
    // Sorting finished, the internal ordering can now be fetched by
    // getOrdering.
}

/*!
 * \brief Get the object ordering.
 * \pre createOrdering has been called.
 * \return The correct object ordering.
 * \sa createOrdering
 */
const vector<string>& DependencyFinder::getOrdering() const {
    // Check if the ordering has been initialized. Print an error if it has
    // not.
    if( mOrdering.empty() ){
        Logger& globalLog = Logger::getGlobalLogger();
        H_LOG( globalLog, Logger::WARNING ) << "Returning an empty object ordering." << endl;
    }
    return mOrdering;
}

/*!
 * \brief Remove a dependency from the matrix.
 * \param aObject Object index for which to remove the dependency.
 * \param aDependency Dependency index to remove.
 */
void DependencyFinder::removeDependency( const size_t aObject, const size_t aDependency ){
    // Remove the dependency, or edge.
    H_ASSERT( mDependencyMatrix.size() > aObject, "removeDependency failure" );
    H_ASSERT( mDependencyMatrix[ aObject ].size() > aDependency, "removeDependency failure" );
    mDependencyMatrix[ aObject ][ aDependency ] = false;
}

/*!
 * \brief Add an item which should have its dependencies tracked.
 * \pre The item is not already being tracked.
 * \param aItem Name of the item to track.
 * \return An iterator to the location of the new item.
 */
DependencyFinder::ObjectIndexMap::iterator DependencyFinder::addTrackedItem( const string& aItem ){
    // Add the item to the mapping of item name to index within the matrix.
    const size_t newLocation = mDependencyMatrix.size();
    
    // Make pair creates a name value pair to insert into the matrix.
    pair<ObjectIndexMap::iterator, bool> newPositionPair = mObjectIndices.insert( make_pair( aItem, newLocation ) );
    
    // Check the precondition that the item does not already exist.
    H_ASSERT( newPositionPair.second, "addTrackedItem failure" );
    
    // Now add the item to the dependency matrix. Loop through the matrix and
    // add a new position on the end of each row.
    for( unsigned int row = 0; row < mDependencyMatrix.size(); ++row ){
        mDependencyMatrix[ row ].push_back( false );
    }
    
    // Add a new row to the matrix for the item. Default to not having any
    // dependencies.
    mDependencyMatrix.push_back( vector<bool>( newLocation + 1, false ) );
    
    // Return an iterator to the position within the index map.
    return newPositionPair.first;
}

/*!
 * \brief Get the name of an object from the matrix index.
 * \param A matrix index to fetch the name for.
 * \note This function is slow as it performs a reverse map lookup and should not
 be used in sections of code which are called frequently.
 * \return The name of the item associated with the index, NO_NAME if it is not
 found.
 */
const string& DependencyFinder::getNameFromIndex( const size_t aIndex ) const {
    // Search the map linearly as this is the reverse lookup.
    for( ObjectIndexMap::const_iterator item = mObjectIndices.begin(); item != mObjectIndices.end(); ++item ){
        // Check if we found the index we are searching for.
        if( item->second == aIndex ){
            return item->first;
        }
    }
    // The index does not exist.
    const static string& NO_NAME = "NO_NAME";
    return NO_NAME;
}

