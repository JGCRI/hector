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

    // math operators
    friend fluxpool operator+ ( const fluxpool&, const fluxpool& );
    friend fluxpool operator+ ( const fluxpool&, const unitval& );
    friend fluxpool operator* ( const fluxpool&, const double );
    friend fluxpool operator* ( const double, const fluxpool& );
    friend fluxpool operator- ( const fluxpool&, const fluxpool& );
    friend fluxpool operator- ( const fluxpool&, const unitval& );
    friend fluxpool operator/ ( const fluxpool&, const double );
    friend double operator/ ( const fluxpool&, const fluxpool&  );
    bool operator==(const fluxpool& rhs);
    bool operator!=(const fluxpool& rhs);
    
    // pretty-printing
//    friend ostream& operator<<(ostream &out, fluxpool &ct);

private:
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
    std::cout << "map size = " << ctmap.size() << "\n";
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
/*! \brief Operator overload: addition.
 */
inline
fluxpool operator+ ( const fluxpool& lhs, const fluxpool& rhs ) {
    H_ASSERT( lhs.valUnits == rhs.units(), "units mismatch: " + lhs.name + " and " + rhs.name );
    H_ASSERT( lhs.tracking == rhs.tracking, "tracking mismatch: " + lhs.name + " and " + rhs.name )
    return fluxpool( lhs.val + rhs.val, lhs.valUnits, lhs.tracking || rhs.tracking );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: addition.
        You can add a unitval to a fluxpool, resulting in a fluxpool
 */
inline
fluxpool operator+ ( const fluxpool& lhs, const unitval& rhs ) {
    H_ASSERT( lhs.valUnits == rhs.units(), "units mismatch: " + lhs.name );
    H_ASSERT( !lhs.tracking, "Can't add a unitval to a tracking fluxpool");
    return fluxpool( lhs.val + rhs.value( lhs.valUnits ), lhs.valUnits );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: subtraction.
 */
inline
fluxpool operator- ( const fluxpool& lhs, const fluxpool& rhs ) {
    H_ASSERT( lhs.valUnits == rhs.units(), "units mismatch: " + rhs.name );
    H_ASSERT( lhs.tracking == rhs.tracking, "tracking mismatch: " + lhs.name + " and " + rhs.name )
    return fluxpool( lhs.val - rhs.val, lhs.valUnits, lhs.tracking, lhs.name );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: subtraction.
        You can subtract a unitval from a fluxpool, resulting in a fluxpool
 */
inline
fluxpool operator- ( const fluxpool& lhs, const unitval& rhs ) {
    H_ASSERT( lhs.valUnits == rhs.units(), "units mismatch: " + lhs.name );
    return fluxpool( lhs.val - rhs.value( lhs.valUnits ), lhs.valUnits, lhs.tracking, lhs.name );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: constant multiplication.
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
/*! \brief Operator overload: constant division.
 */
inline
fluxpool operator/ ( const fluxpool& lhs, const double rhs ) {
    return fluxpool( lhs.val / rhs, lhs.valUnits, lhs.tracking, lhs.name );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: division.
 */
inline
double operator/ ( const fluxpool& lhs, const fluxpool& rhs ) {
    H_ASSERT( lhs.valUnits == rhs.units(), "units mismatch: " + lhs.name + " and " + rhs.name );
    H_ASSERT( lhs.tracking == rhs.tracking, "tracking mismatch: " + lhs.name + " and " + rhs.name )
    return lhs.val / rhs.val;
}

}

#endif /* FLUXPOOL_H */
