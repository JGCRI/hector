#ifndef FLUXPOOL_H
#define FLUXPOOL_H

/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/

#include "unitval.hpp"
#include <unordered_map>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>    // std::set_union

using namespace std;

/*
 *  fluxpool.hpp - A unitval, but its value may not be negative
 *  Also can track the origins of its 'stuff'
 *
 *  hector
 *
 *  Created by Ben on 2021-02-05.
 *  Tracking implementation 2021 by Skylar Gering, Harvey Mudd College
 *
 */

namespace Hector {

class fluxpool: public unitval {

public:
    fluxpool();
    fluxpool( double, unit_types, bool, string  );
    void set( double, unit_types );
    
    // tracking-specific functions
    vector<string> get_sources() const;
    double get_fraction(string source) const;
    bool tracking;
    string name;
    fluxpool flux_from_unitval(unitval) const;

    // math operators
    friend fluxpool operator+ ( const fluxpool&, const fluxpool& );
    friend fluxpool operator+ ( const fluxpool&, const unitval& );
    friend fluxpool operator* ( const fluxpool&, const double );
    friend fluxpool operator* ( const double, const fluxpool& );
    friend fluxpool operator- ( const fluxpool&, const fluxpool& );
    friend fluxpool operator- ( const fluxpool&, const unitval& );
    friend fluxpool operator/ ( const fluxpool&, const double );
    friend double operator/ ( const fluxpool&, const fluxpool&  );
    friend bool operator== ( const fluxpool&, const fluxpool& );
    friend bool operator!= ( const fluxpool&, const fluxpool& );

    // pretty-printing
//    friend ostream& operator<<(ostream &out, fluxpool &ct);

private:
    fluxpool(unitval, unordered_map<string, double>, string);

    unordered_map<string, double> ctmap;
};

// Non-member function for multiplication with double as first argument
fluxpool operator*(double d, const fluxpool& ct);


//-----------------------------------------------------------------------
/*! \brief Public constructor
 */
inline
fluxpool::fluxpool() {
    tracking = false;
}

//-----------------------------------------------------------------------
/*! \brief Assign this fluxpool's name and then pass control to set()
 */
inline
fluxpool::fluxpool( double v, unit_types u, bool track = false, string pool_name = "?" ) {
    name = pool_name;
    tracking = track;
    set(v, u);
}

//-----------------------------------------------------------------------
/*! \brief Private constructor with explicit source pool map
 */
inline
fluxpool::fluxpool(unitval v, unordered_map<string, double> pool_map, string pool_name) {
    unitval::set(v.value(v.units()), v.units(), 0.0);
    tracking = true;
    ctmap = pool_map;
    name = pool_name;
    
    H_ASSERT(v >= 0, "Flux and pool values may not be negative in " + name);

     // check pool_map data
    double frac = 0.0;
    for (auto itr = ctmap.begin(); itr != ctmap.end(); itr++) {
        H_ASSERT(itr->second >= 0 && itr->second <= 1, "fractions must be 0-1 for " + pool_name);
        frac += itr->second;
    }
    H_ASSERT(frac - 1.0 < 1e-6, "pool_map must sum to ~1.0 for " + pool_name)
}

//-----------------------------------------------------------------------
/*! \brief Check that the value is >=0 before passing control to unitval
 */
inline
void fluxpool::set( double v, unit_types u ) {
    if(v < 0) {
 //        std::cout << "uh oh";
     }
    H_ASSERT(v >= 0, "Flux and pool values may not be negative in " + name);
    ctmap[name] = 1.0;
    unitval::set(v, u, 0.0);
}

//-----------------------------------------------------------------------
/*! \brief Return a string vector of the current sources
 */
inline
std::vector<std::string> fluxpool::get_sources() const {
    H_ASSERT(tracking, "get_sources() requires tracking to be on in " + name);
    vector<string> sources;
    for (auto itr = ctmap.begin(); itr != ctmap.end(); itr++) {
        sources.push_back(itr->first);
    }
    return sources;
}

//-----------------------------------------------------------------------
/*! \brief Return the fraction corresponding to a specific source
 */
inline
double fluxpool::get_fraction(string source) const {
    H_ASSERT(tracking, "get_fraction() requires tracking to be on in " + name);
    double val = 0.0;  // 0.0 is returned if not in our map
    auto x = ctmap.find(source);
    if(x != ctmap.end()) {
        val = x->second;
    }
    return val;
}

//-----------------------------------------------------------------------
/*! \brief Given a unitval, return a fluxpool with that total and our source pool map.
                        This is needed when dealing with LUC and other input (i.e. unitval) fluxes
 */
inline
fluxpool fluxpool::flux_from_unitval(unitval f) const {
    return fluxpool(f, ctmap, name);
}


//-----------------------------------------------------------------------
/*! \brief Operator overload: addition
 */
inline
fluxpool operator+ ( const fluxpool& lhs, const fluxpool& rhs ) {
    H_ASSERT( lhs.units() == rhs.units(), "units mismatch: " + lhs.name + " and " + rhs.name );
    H_ASSERT( lhs.tracking == rhs.tracking, "tracking mismatch: " + lhs.name + " and " + rhs.name )
    
    if (!lhs.tracking) {
        return fluxpool( lhs.val + rhs.val, lhs.units(), false, lhs.name );
    }
    
    // This is the complicated case

    // Compute the overall new total (a unitval)
    Hector::unitval new_total(lhs.val + rhs.val, lhs.units());
    
    // Construct a vector of the combined sources
    vector<string> lhs_src = lhs.get_sources(),
                   rhs_src = rhs.get_sources(),
                   both_sources;
    
    std::set_union(std::begin( lhs_src ), std::end( lhs_src ),
                                  std::rbegin( rhs_src ), std::rend( rhs_src ),
                                  std::back_inserter( both_sources ) );
    
    // Walk through the sources and compute the combined absolute value
    // Note get_fraction() return 0 if source not found, which is what we want
    unordered_map<string, unitval> new_pools;
    for (auto &s: both_sources) {
      new_pools[s] = unitval(lhs.val * lhs.get_fraction(s) + rhs.val * rhs.get_fraction(s), lhs.units());
    }
    
    // Now that we have the new pool values, compute new fractions
    unordered_map<string, double> new_origins;
    for (auto &s: both_sources) {
      if(new_total) {
          new_origins[s] = new_pools[s] / new_total;
      } else {  // uh oh, new total is zero
          new_origins[s] = 1.0 / new_pools.size();
      }
    }

    return fluxpool(new_total, new_origins, lhs.name);
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: addition
        You can add a unitval to a fluxpool, resulting in a fluxpool
 */
inline
fluxpool operator+ ( const fluxpool& lhs, const unitval& rhs ) {
    H_ASSERT( lhs.valUnits == rhs.units(), "units mismatch: " + lhs.name );
    H_ASSERT( !lhs.tracking, "Can't add a unitval to a tracking fluxpool");
    return fluxpool( lhs.val + rhs.value( lhs.valUnits ), lhs.valUnits );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: subtraction
 */
inline
fluxpool operator- ( const fluxpool& lhs, const fluxpool& rhs ) {
    H_ASSERT( lhs.valUnits == rhs.units(), "units mismatch: " + rhs.name );
    H_ASSERT( lhs.tracking == rhs.tracking, "tracking mismatch: " + lhs.name + " and " + rhs.name )
    return fluxpool( lhs.val - rhs.val, lhs.valUnits, lhs.tracking, lhs.name );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: subtraction
        You can subtract a unitval from a fluxpool, resulting in a fluxpool
 */
inline
fluxpool operator- ( const fluxpool& lhs, const unitval& rhs ) {
    H_ASSERT( lhs.valUnits == rhs.units(), "units mismatch: " + lhs.name );
    return fluxpool( lhs.val - rhs.value( lhs.valUnits ), lhs.valUnits, lhs.tracking, lhs.name );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: constant multiplication
 */
inline
fluxpool operator* ( const fluxpool& lhs, const double rhs ) {
    return fluxpool( lhs.val * rhs, lhs.valUnits, lhs.tracking, lhs.name );
}
inline
fluxpool operator* ( const double lhs, const fluxpool& rhs ) {
    return fluxpool( lhs * rhs.val, rhs.valUnits, rhs.tracking, rhs.name  );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: constant division
 */
inline
fluxpool operator/ ( const fluxpool& lhs, const double rhs ) {
    return fluxpool( lhs.val / rhs, lhs.valUnits, lhs.tracking, lhs.name );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: division
 */
inline
double operator/ ( const fluxpool& lhs, const fluxpool& rhs ) {
    H_ASSERT( lhs.valUnits == rhs.units(), "units mismatch: " + lhs.name + " and " + rhs.name );
    H_ASSERT( lhs.tracking == rhs.tracking, "tracking mismatch: " + lhs.name + " and " + rhs.name )
    return lhs.val / rhs.val;
}

//-----------------------------------------------------------------------
/*! \brief Equality and inequality: same total only
 */
inline
bool operator== ( const fluxpool& lhs, const fluxpool& rhs ) {
    H_ASSERT( lhs.valUnits == rhs.units(), "units mismatch: " + lhs.name + " and " + rhs.name );
    return lhs.val == rhs.val;
}
inline
bool operator!= ( const fluxpool& lhs, const fluxpool& rhs ) {
    H_ASSERT( lhs.valUnits == rhs.units(), "units mismatch: " + lhs.name + " and " + rhs.name );
    return lhs.val != rhs.val;
}

}

#endif /* FLUXPOOL_H */
