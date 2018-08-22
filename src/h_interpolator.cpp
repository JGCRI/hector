/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  h_interpolator.cpp
 *  hector
 *
 *  Created by Ben on 9/21/10.
 *
 */

#include <iostream>

#include "h_interpolator.hpp"
#include "h_exception.hpp"


namespace Hector {
  
//-----------------------------------------------------------------------
/*! \brief Constructor for spline class.
 *
 *  Initializes any internal variables.
 */
h_interpolator::h_interpolator() {
    xdata=0;            // NULL
    ydata=0;
    b_coef=0;
    c_coef=0;
    d_coef=0;
    ndata=0;
    ilast = -1;
    set_method( DEFAULT );
}

//-----------------------------------------------------------------------
/*! \brief Destructor for spline class.
 *
 *  De-initializes any internal variables.
 */
h_interpolator::~h_interpolator() {
    release_data();
}

//-----------------------------------------------------------------------
/*! \brief Release memory used by internal vars.
 *
 *  Release memory used by internal vars.
 */
void h_interpolator::release_data() {
	delete[]( xdata );        // free old memory, if necessary
	delete[]( ydata );
    delete[]( b_coef );
    delete[]( c_coef );
    delete[]( d_coef );
}

//-----------------------------------------------------------------------
/*! \brief Refit data.
 *
 *  Refit spline, or whatever, to existing data.
 */
void h_interpolator::refit_data() {
    
    switch( method ) {
        case LINEAR: /* nothing to do */
            break;
        case SPLINE_FORSYTHE:
            spline_forsythe( ndata, xdata, ydata, b_coef, c_coef, d_coef );
            break;
        default: H_THROW( "Undefined interpolation method" );
    }
}

//-----------------------------------------------------------------------
/*! \brief Receive new data.
 *
 *  Receive new data: make a private copy, and fit spline
 *  (or whatever) as necessary.
 */
void h_interpolator::newdata( int n, double* x, double* y ) {
    H_ASSERT( n, "interpolator newdata n=0" );
    release_data();
    
    ndata = n;
    xdata = new double[ ndata ];
    ydata = new double[ ndata ];
    b_coef = new double[ ndata ];
    c_coef = new double[ ndata ];
    d_coef = new double[ ndata ];
    for( int i=0; i<ndata; i++ ) {
        xdata[ i ] = x[ i ];
        ydata[ i ] = y[ i ];
    }
    
    //TODO: sort points!
	// Not necessary here, as tseries guarantees in-order
    // but if anything else uses interpolator, need to do this!
    
    refit_data();
}

//-----------------------------------------------------------------------
/*! \brief Return y=f(x) using linear interpolation.
 *
 *  Return y=f(x) using linear interpolation.
 */
double h_interpolator::f_linear( double x ) {
    
    int iprev, inext;
    locate(x, iprev,inext);

    
    double returnval;
    if( iprev < 0 )
        returnval = ydata[ 0 ];
    else if( inext >= ndata )
        returnval = ydata[ ndata-1 ];
    else	// the actual linear interpolation
        returnval = ydata[ iprev ] + (x - xdata[ iprev ] ) *
            ( ydata[ inext ] - ydata[ iprev ] ) / ( xdata[ inext ] - xdata[ iprev ] );
    
    return returnval;
}

//-----------------------------------------------------------------------
/*! \brief Return y=f'(x) using linear interpolation.
 *
 *  Return y=f'(x) using linear interpolation.
 */
double h_interpolator::f_deriv_linear( double x ) {
    
    int iprev, inext;
    locate(x, iprev,inext);

    
    double returnval;
    if( iprev < 0 ) {
        // before the series, throw error?
        returnval = 0;
    }
    else if( inext >= ndata && x > xdata[ ndata-1 ] ) {
        // after the series, throw error?
        returnval = 0;
    }
    else if( x == xdata[ iprev ] && (iprev != 0 && inext != ndata)) {
        // Taking the derivative at the abscissas is problematic since the function
        // is not continous here.  Instead we will average the slopes on the surrounding
        // segments
        // Note for abscissas at the very begining or end of the data we assume the
        // slope of the segment it is connected to (same as the general case).
        double slopePrev = ( ydata[ inext-1 ] - ydata[ iprev-1 ] ) / (xdata[ inext-1 ] - xdata[ iprev-1 ] );
        double slopeNext = ( ydata[ inext ] - ydata[ iprev ] ) / (xdata[ inext ] - xdata[ iprev ] );
        returnval = ( slopePrev + slopeNext ) / 2.0;
    }
    else {
        if(inext == ndata) {
            // Special case for the very last point. We just use the slope of the
            // previous segment.
            --iprev;
            --inext;
        } 
        // we can just use the slope of this segment
        returnval = ( ydata[ inext ] - ydata[ iprev ] ) / (xdata[ inext ] - xdata[ iprev ] );
    }
    
    return returnval;
}

//-----------------------------------------------------------------------
/*! \brief Return y=f(x) using current interpolation.
 *
 *  Return y=f(x) using current interpolation method.
 */
double h_interpolator::f( double x ) {
    
    // Following section will be replaced by spline code
    switch( method ) {
        case LINEAR:
            return f_linear( x );
            break;
        case SPLINE_FORSYTHE:
            return seval_forsythe( ndata, x, xdata, ydata, b_coef, c_coef, d_coef );
            break;
            
        default: H_THROW( "Undefined interpolation method" );
    }
}

//-----------------------------------------------------------------------
/*! \brief Return y=f'(x) using current interpolation.
 *
 *  Return y=f'(x) using current interpolation method.
 */
double h_interpolator::f_deriv( double x ) {
    
    // Following section will be replaced by spline code
    switch( method ) {
        case LINEAR:
            return f_deriv_linear( x );
            break;
        case SPLINE_FORSYTHE:
            return seval_deriv_forsythe( ndata, x, xdata, ydata, b_coef, c_coef, d_coef );
            break;
            
        default: H_THROW( "Undefined interpolation method" );
    }
}

//-----------------------------------------------------------------------
/*! \brief Set spline method.
 *
 *  Set spline method.
 */
void h_interpolator::set_method( interpolation_methods m ) {
    method = ( m==DEFAULT ) ? DEFAULT_METHOD : m;
    //TODO: log method set
    if( ndata )
        refit_data();
}

}
