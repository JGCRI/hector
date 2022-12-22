/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
/*
 *  h_interpolator.cpp
 *  hector
 *
 *  Created by Bill Emanuel on 9/21/10.
 *
 */

#include "h_exception.hpp"

namespace Hector {

/*  Interpolating cubic-spline function.
 Forsythe, G. E., M. A. Malcolm, and C. B. Moler. 1977. Computer Methods for
 Mathematical Computations. Prentice-Hall, Englewood Cliffs, New Jersey.
 Translated from Fortran by
 W. R. Emanuel, Joint Global Change Research Institute
 Pacific Northwest National Laboratory & University of Maryland
 E-Mail: william.emanuel@pnl.gov */

/*  The function spline() calculates cubic-spline coefficients such that the resulting
 set of polynomials interpolate the (x, y) pairs specified by the arguments x and y.
 The argument n is the number of data points, i.e., the length of the x and y arrays.
 The number of points must be greater then or equal to 2 (n >= 2). If n == 2, then
 a line will be derived to support linear interpolation.
 
 On return from function spline(), the arrays specified by the arguments b, c, and d
 contain the spline coefficients.
 
 If evaluation of spline coefficients fails, typically because n < 2, then spline()
 returns 0.
 
 The function seval() evaluates the interpolating function at the argument u. The
 data and spline coefficients are passed through the arguments x, y, b, c, and d.
 If u is not within the interval of definition of the function, (x[0], x[n-1]), the results
 are unpredictable.
 
 Based on Fortran code at http://netlib.sandia.gov/fmm/index.html,
 accessed September 2010.
 */


void spline_forsythe( int n, double *x, double *y, double *b, double *c, double *d ) {
    /* The coefficients b[i], c[i], and d[i], i = 1, 2, ..., n are computed for a cubic
     interpolating spline s(x) = y[i] + b[i]*(x-x[i]) + c[i]*(x-x[i])**2 + d[i]*(x-x[i])**3
     for  x[i] <= x <= x[i+1]. The accompanying function seval() can be used to
     evaluate the spline.
     
     Input:
     n = the number of data points or knots (n >= 2)
     x = the abscissas of the knots in strictly increasing order
     y = the ordinates of the knots
     
     Output:
     b, c, d  = arrays of spline coefficients as defined above.
     
     Using  p  to denote differentiation,
     y[i] = s(x[i])
     b[i] = sp(x[i])
     c[i] = spp(x[i])/2
     d[i] = sppp(x[i])/6  (derivative from the right). */
    
    int i, ib;
    double t;
    
    H_ASSERT( n && x && y && b && c && d, "spline arguments must be nonzero" );
    
    H_ASSERT( n >= 2, "Insufficient data for interpolation" );
    
    if (n < 3) {
        b[0] = (y[1] - y[0]) / (x[1] - x[0]);
        c[0] = 0.0;
        d[0] = 0.0;
        b[1] = b[0];
        c[1] = 0.0;
        d[1] = 0.0;
    }
    else {
        /* Set up tridiagonal system.
         b = diagonal, d = off-diagonal, c = right-hand side. */
        d[0] = x[1] - x[0];
        c[1] = (y[1] - y[0]) / d[0];
        for (i = 1; i < n-1; ++i) {
            d[i] = x[i+1] - x[i];
            b[i] = 2.0 * (d[i-1] + d[i]);
            c[i+1] = (y[i+1] - y[i]) / d[i];
            c[i] = c[i+1] - c[i];
        }
        
        /* End conditions. Third derivatives at  x[1]  and  x[n] obtained from
         divided differences. */
        b[0] = -d[0];
        b[n-1] = -d[n-2];
        c[0] = 0.0;
        c[n-1] = 0.0;
        if (n > 3) {
            c[0] = c[2] / (x[3] - x[1]) - c[1] / (x[2] - x[0]);
            c[n-1] = c[n-2] / (x[n-1] - x[n-3]) - c[n-3] / (x[n-2] - x[n-4]);
            c[0] = c[0] * d[0] * d[0] / (x[3] - x[0]);
            c[n-1] = -c[n-1] * d[n-2] * d[n-2] / (x[n-1] - x[n-4]);
        }
        
        /* Forward elimination. */
        for (i = 1; i < n; ++i) {
            t = d[i-1] / b[i-1];
            b[i] = b[i] - t * d[i-1];
            c[i] = c[i] - t * c[i-1];
        }
        
        /* Back substitution. */
        c[n-1] = c[n-1] / b[n-1];
        for (ib = 0; ib < n-1; ++ib) {
            i = n - ib - 1;
            c[i] = (c[i] - d[i] * c[i+1]) / b[i];
        }
        /* c[i] is now the sigma[i] of the text. */
        
        /* Compute polynomial coefficients. */
        b[n-1] = (y[n-1] - y[n-2]) / d[n-2] + d[n-2] * (c[n-2] + 2.0 * c[n-1]);
        for (i = 0; i < n-1; ++i) {
            b[i] = (y[i+1] - y[i]) / d[i] - d[i] * (c[i+1] + 2.0 * c[i]);
            d[i] = (c[i+1] - c[i]) / d[i];
            c[i] = 3.0 * c[i];
        }
        c[n-1] = 3.0 * c[n-1];
        d[n-1] = d[n-2];
    }
}

double seval_forsythe( int n, double u, double *x, double *y, double *b, double *c, double *d ) {
    /* Evaluate a cubic spline function.
     seval = y(i) + b(i)*(u-x(i)) + c(i)*(u-x(i))**2 + d(i)*(u-x(i))**3
     where  x(i) .lt. u .lt. x(i+1), using horner's rule.
     If  u .lt. x(1) then  i = 1  is used.
     If  u .ge. x(n) then  i = n  is used.
     Input:
     n = the number of data points
     u = the abscissa at which the spline is to be evaluated
     x,y = the arrays of data abscissas and ordinates
     b,c,d = arrays of spline coefficients computed by spline
     If  u  is not in the same interval as the previous call, then a binary search is
     performed to determine the proper interval.
     
     The function seval() is invoked with the (x, y) pairs underlying the interpolating
     function specified by the arguments x and y, and the spline coefficients that
     define the function as specified by b, c, and d. The argument n is the number
     of data points and the length of the coefficient arrays. */
    
    H_ASSERT( n && x && y && b && c && d, "seval_forsythe needs nonzero params" );
    
    static int i = 0;
    int j, k;
    double dx;
    
    /* Test for u within the interval of definition of the interpolating function. If not,
     return the value at an end point of the interval. */
    if (u < x[0]) return y[0];
    if (u > x[n-1]) return y[n-1];
    
    /* Search for the data points with independent values containing the
     argument u. */
    if (i >= n-1) i = 0;
    
    /* If u is not in the current interval, then execute a binary search. */
    if ((u < x[i]) || (u > x[i+1])) {
        i = 0;
        j = n;
        while (j > i + 1) {
            k = (int)(( i + j) / 2);
            if (u < x[k]) j = k;
            if (u >= x[k]) i = k;
        }
    }
    
    /* Evaluate spline function at the argument u. */
    dx = u - x[i];
    return y[i] + dx * (b[i] + dx * (c[i] + dx * d[i]));
}

double seval_deriv_forsythe( int n, double u, double *x, double *y, double *b, double *c, double *d ) {
    /* Evaluate the derivative of a cubic spline function.
     seval_deriv = b(i) + 2*c(i)*(u-x(i)) + 3*d(i)*(u-x(i))**2
     where  x(i) .lt. u .lt. x(i+1), using horner's rule.
     If  u .lt. x(1) then  i = 1  is used.
     If  u .ge. x(n) then  i = n  is used.
     Input:
     n = the number of data points
     u = the abscissa at which the spline is to be evaluated
     x,y = the arrays of data abscissas and ordinates
     b,c,d = arrays of spline coefficients computed by spline
     If  u  is not in the same interval as the previous call, then a binary search is
     performed to determine the proper interval.
     
     The function seval() is invoked with the (x, y) pairs underlying the interpolating
     function specified by the arguments x and y, and the spline coefficients that
     define the function as specified by b, c, and d. The argument n is the number
     of data points and the length of the coefficient arrays. */
    
    H_ASSERT( n && x && y && b && c && d, "seval_forsythe needs nonzero params" );
    
    static int i = 0;
    int j, k;
    double dx;
    
    /* Test for u within the interval of definition of the interpolating function. If not,
     return the value at an end point of the interval. */
    if (u < x[0]) { u = x[0]; }
    if (u > x[n-1]) { u = x[n-1]; }
    
    /* Search for the data points with independent values containing the
     argument u. */
    if (i >= n-1) i = 0;
    
    /* If u is not in the current interval, then execute a binary search. */
    if ((u < x[i]) || (u > x[i+1])) {
        i = 0;
        j = n;
        while (j > i + 1) {
            k = (int)(( i + j) / 2);
            if (u < x[k]) j = k;
            if (u >= x[k]) i = k;
        }
    }
    
    /* Evaluate the derivative of the spline function at the argument u. */
    dx = u - x[i];
    return b[i] + ( 2.0 * dx * c[i] ) + ( 3.0 * dx * dx * d[i] );
}
}
