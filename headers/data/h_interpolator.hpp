/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   Please see the accompanying file LICENSE.md for additional licensing
   information.
*/
#ifndef H_INTERPOLATOR_H
#define H_INTERPOLATOR_H
/*
 *  h_interpolator.h
 *  hector
 *
 *  Created by Ben on 9/21/10.
 *
 */

namespace Hector {

enum interpolation_methods { DEFAULT, LINEAR, SPLINE_FORSYTHE };

#define DEFAULT_METHOD LINEAR

//-----------------------------------------------------------------------
// Prototypes for interpolation methods
void spline_forsythe( int, double *, double *, double *, double *, double * );
double seval_forsythe( int, double, double *, double *, double *, double *, double * );
double seval_deriv_forsythe( int, double, double *, double *, double *, double *, double * );

//-----------------------------------------------------------------------
/*! \brief interpolator class header.
 *
 *  Regardless of backend implementation, offers two methods:
 *  accept new data (NEWDATA), and return y=f(x) (F).
 */
class h_interpolator {
private:
    interpolation_methods method;
    int ndata;
    double *xdata, *ydata;
    double *b_coef, *c_coef, *d_coef;
    
    double f_linear( double );
    double f_deriv_linear( double );
    
    void release_data();
    void refit_data();

    //! last value of the lower neighbor.
    mutable int ilast;

    void locate(double x, int &iprev, int &inext) const;
    
public:
    h_interpolator();
    ~h_interpolator();
    double f( double );
    double f_deriv( double );
    void newdata( int, double*, double* );
    void set_method( interpolation_methods );
};

inline void h_interpolator::locate(double x, int &iprev, int &inext) const
{
    /* Test for u within the interval of definition of the interpolating function. If not,
     return the value at an end point of the interval. */
    if(x<xdata[0]) {
      iprev = -1;
      inext = ilast = 0;
      return;
    }
    else if(x>=xdata[ndata-1]) {
      iprev = ilast = ndata-1;
      inext = ndata;
      return;
    }
    
    /* Search for the data points with independent values containing the
     argument u. */
    if (ilast >= ndata-1 || ilast < 0) ilast = 0;
    
    /* If u is not in the current in || ilast < 0terval, then execute a binary search. */
    if ((x < xdata[ilast]) || (x >= xdata[ilast+1])) {
        ilast = 0;
        iprev = ndata + 1;
        while (iprev > ilast + 1) {
            inext = (int)(( ilast + iprev) / 2);
            if (x < xdata[inext]) iprev = inext;
            if (x >= xdata[inext]) ilast = inext;
        }
    }
    iprev = ilast;
    inext = ilast + 1;
}

}

#endif
