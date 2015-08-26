/* Hector -- A Simple Climate Model
   Copyright (C) 2014-2015  Battelle Memorial Institute

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
    
    void release_data();
    void refit_data();

    //! last value of the lower neighbor.
    mutable int ilast;

    void locate(double x, int &iprev, int &inext) const;
    
public:
    h_interpolator();
    ~h_interpolator();
    double f( double );
    void newdata( int, double*, double* );
    void set_method( interpolation_methods );
};

inline void h_interpolator::locate(double x, int &iprev, int &inext) const
{
    if(ilast>=0) {
        iprev = ilast;
        // search near the last-used value
        if(xdata[iprev] <= x) {
            inext = iprev+1;
            if(iprev == ndata-1 || xdata[inext] >= x)
                return;         // found it in the same interval as last time, no need to update ilast
            else
                inext = ndata-1; // fall through to bisection search
        }
        else {
            // check the previous interval
            inext = iprev--;
            if(inext==0)
                return;         // don't set ilast to iprev; since iprev<0
            else if(xdata[iprev] <= x) {
                ilast = iprev;  // iprev >=0 in this case
                return;
            }
            else
                iprev = 0;      // fall through to search
        }
    }

    // no previous values, or the previous value is invalid.  set
    // up for bisection on full array
    // check edge cases
    if(x<xdata[0]) {
      iprev = -1;
      inext = ilast = 0;
      return;
    }
    else if(x>xdata[ndata-1]) {
      iprev = ilast = ndata-1;
      inext = ndata;
      return;
    }
    else {
      iprev = 0;
      inext = ndata-1;
    }
    
    // perform bisection search
    while(inext-iprev > 1) {
        int imid = (iprev+inext)/2;
        if(xdata[imid] <= x)
            iprev = imid;
        else
            inext = imid;
    }
    ilast = iprev;
    
}

}

#endif
