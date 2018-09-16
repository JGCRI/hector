/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef TVECTOR_H
#define TVECTOR_H
/*
 *  tvector.hpp - Vector of values indexed by time.
 *
 *  This class differs from the `tseries` class in that it doesn't
 *  offer an interpolation option; therefore, it can be used with
 *  types for which interpolation doesn't make sense, such as
 *  containers and other structures.  The attempt to instantiate the
 *  interp_helper class in `tseries` causes compilation to fail for
 *  such types, even if you never intend to use it.
 *
 *  This problem probably could have been solved within the tseries
 *  class with suitable application of type traits such as
 *  `std::is_convertible`, along with metaprogramming functions, such
 *  as `std::enable_if`, but honestly it's a lot quicker and easier
 *  just to make a stripped down class with no interpolation functions
 *  in it. Along the way we also add some nice-to-have functionality,
 *  such as allowing non-const data to be modified in place.
 *
 */

#include <map>
#include <limits>
#include <string>
#include <cmath>
#include <sstream>

#include "logger.hpp"
#include "h_exception.hpp"

namespace Hector {
  
/*! \brief Time vector data type.
 *
 *  Currently implemented as an STL map.
 */
template <class T_data>
class tvector {
    std::map<double, T_data> mapdata;
public:

    void set(double, const T_data &);
    const T_data &get(double) const throw( h_exception );
    T_data &get(double) throw(h_exception); // allow modify-in-place
    bool exists( double ) const;

    // indexing operators: the const version is just syntactic sugar
    // for get().  The non-const version works differently.  If the
    // requested object doesn't exist, it default constructs it,
    // assigns the new object, and returns the reference to the newly
    // created object.  This allows you to write tvec[t] = xx.
    const T_data &operator[](double t) const throw(h_exception) {
        return get(t);
    }
    T_data &operator[](double t);
    
    double firstdate() const;
    double lastdate() const;
    
    int size() const;
    
    void truncate(double t, bool after=true);
private:
    static double round(double t) {
        // round time values to prevent minute differences in
        // representation from resulting in a misidentificaiton.
        // Right now we round to the nearest half-integer, but that could
        // change in the future, if we need more time resolution.
        return 0.5 * ::round(2.0*t);
    }
};


//-----------------------------------------------------------------------
/*! \brief 'Set' for time vector data type.
 *
 *  Sets an (t, d) tuple, data d at time t.
 */
template <class T_data>
void tvector<T_data>::set(double t, const T_data &d) {
    mapdata[round(t)] = d;
}

//-----------------------------------------------------------------------
/*! \brief Does data exist at time (position) t?
 *
 *  Returns a bool to indicate if data exists.
 */
template <class T_data>
bool tvector<T_data>::exists( double t ) const {
    return ( mapdata.find( round(t) ) != mapdata.end() );
}

//-----------------------------------------------------------------------
/*! \brief 'Get' for time vector data type.
 *
 *  Return data associated with time t.
 *  If no value exists, raise an exception.
 */
template <class T_data>
const T_data &tvector<T_data>::get( double t ) const throw( h_exception ) {
    typename std::map<double,T_data>::const_iterator itr = mapdata.find( round(t) );
    if( itr != mapdata.end() )
        return (*itr).second;
    else {
        std::ostringstream errmsg;
        errmsg << "No data at requested time= " << round(t) << "\n";
        H_THROW(errmsg.str());
    }
}

/*! \brief mutable get method
 *
 * If the time vector object is not const, then get() returns a
 * non-const reference, allowing an object to be modified in place.
 */
template <class T_data>
T_data &tvector<T_data>::get( double t ) throw( h_exception ) {
    typename std::map<double,T_data>::iterator itr = mapdata.find( round(t) );
    if( itr != mapdata.end() )
        return itr->second;
    else {
        std::ostringstream errmsg;
        errmsg << "No data at requested time= " << round(t) << "\n"; 
        H_THROW(errmsg.str());
    }
}   

template <class T_data>
T_data &tvector<T_data>::operator[](double t) {
    try {
        return get(t);
    }
    catch(h_exception &fail) {
        // h_exception is thrown when the requested object doesn't exist.
        T_data newdat = T_data();
        set(t, newdat);
        return get(t);          // won't fail b/c we just created it
    }
}


//-----------------------------------------------------------------------
/*! \brief Return index of first element in vector.
 *
 *  Return index of first element in vector.
 */
template <class T_data>
double tvector<T_data>::firstdate() const {
    H_ASSERT( !mapdata.empty(), "no mapdata" );
    return (*mapdata.begin()).first;
}

//-----------------------------------------------------------------------
/*! \brief Return index of last element in vector.
 *
 *  Return index of last element in vector.
 */
template <class T_data>
double tvector<T_data>::lastdate() const {
    H_ASSERT( !mapdata.empty(), "no mapdata" );
    return (*mapdata.rbegin()).first;
}

//-----------------------------------------------------------------------
/*! \brief Return size of vector.
 *
 *  Return size of vector.
 */
template <class T_data>
int tvector<T_data>::size() const {
    return int( mapdata.size() );
}

/*! \brief truncate a time vector
 *
 *  \details The default is to wipe all of the data in the time vector
 *           after the input date.  By setting the optional after
 *           argument to false, you can wipe data before the input
 *           date instead. 
 */
template <class T>
void tvector<T>::truncate(double t, bool after)
{
    t = round(t);
    typename std::map<double, T>::iterator it1, it2;
    if(after) {
        it1 = mapdata.upper_bound(t);
        it2 = mapdata.end();
    }
    else {
        it1 = mapdata.begin();
        it2 = mapdata.lower_bound(t);
    } 
    mapdata.erase(it1,it2); 
}

}

#endif
