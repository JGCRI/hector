#ifndef FLUXPOOL_H
#define FLUXPOOL_H

/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/

#include "unitval.hpp"

/*
 *  fluxpool.hpp - a unitval, but its value may not be negative
 *
 *  hector
 *
 *  Created by Ben on 2021-02-05.
 *
 */

namespace Hector {

class fluxpool: public unitval {

public:
    fluxpool() {};
    fluxpool( double, unit_types );
    void set( double, unit_types, double );

    friend fluxpool operator+ ( const fluxpool&, const fluxpool& );
    friend fluxpool operator+ ( const fluxpool&, const unitval& );
    friend fluxpool operator* ( const fluxpool&, const double );
    friend fluxpool operator* ( const double, const fluxpool& );
    friend fluxpool operator- ( const fluxpool&, const fluxpool& );
    friend fluxpool operator- ( const fluxpool&, const unitval& );
    friend fluxpool operator/ ( const fluxpool&, const double );
    friend double operator/ ( const fluxpool&, const fluxpool&  );
};


//-----------------------------------------------------------------------
/*! \brief Check that the value is >=0 before passing control to unitval
 */
inline
fluxpool::fluxpool( double v, unit_types u ) {
    set(v, u, 0.0);
}

//-----------------------------------------------------------------------
/*! \brief Check that the value is >=0 before passing control to unitval
 */
inline
void fluxpool::set( double v, unit_types u, double err = 0.0 ) {
    if(v < 0) {
         std::cout << "uh oh";
     }
    H_ASSERT(v >= 0, "Flux and pool values may not be negative");
    unitval::set(v, u, err);
}


//-----------------------------------------------------------------------
/*! \brief Operator overload: addition.
 */
inline
fluxpool operator+ ( const fluxpool& lhs, const fluxpool& rhs ) {
    H_ASSERT( lhs.valUnits == rhs.valUnits, "units mismatch" );
    return fluxpool( lhs.val + rhs.val, lhs.valUnits );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: addition.
        You can add a unitval to a fluxpool, resulting in a fluxpool
 */
inline
fluxpool operator+ ( const fluxpool& lhs, const unitval& rhs ) {
    H_ASSERT( lhs.valUnits == rhs.units(), "units mismatch" );
    return fluxpool( lhs.val + rhs.value( lhs.valUnits ), lhs.valUnits );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: subtraction.
 */
inline
fluxpool operator- ( const fluxpool& lhs, const fluxpool& rhs ) {
    H_ASSERT( lhs.valUnits == rhs.valUnits, "units mismatch" );
    return fluxpool( lhs.val - rhs.val, lhs.valUnits );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: subtraction.
        You can subtract a unitval from a fluxpool, resulting in a fluxpool
 */
inline
fluxpool operator- ( const fluxpool& lhs, const unitval& rhs ) {
    H_ASSERT( lhs.valUnits == rhs.units(), "units mismatch" );
    return fluxpool( lhs.val - rhs.value( lhs.valUnits ), lhs.valUnits );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: constant multiplication.
 */
inline
fluxpool operator* ( const fluxpool& lhs, const double rhs ) {
    return fluxpool( lhs.val * rhs, lhs.valUnits );
}
inline
fluxpool operator* ( const double lhs, const fluxpool& rhs ) {
    return fluxpool( lhs * rhs.val, rhs.valUnits );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: constant division.
 */
inline
fluxpool operator/ ( const fluxpool& lhs, const double rhs ) {
    return fluxpool( lhs.val / rhs, lhs.valUnits );
}

//-----------------------------------------------------------------------
/*! \brief Operator overload: division.
 */
inline
double operator/ ( const fluxpool& lhs, const fluxpool& rhs ) {
    H_ASSERT( lhs.valUnits == rhs.valUnits, "units mismatch" );
    return lhs.val / rhs.val;
}

}

#endif /* FLUXPOOL_H */
