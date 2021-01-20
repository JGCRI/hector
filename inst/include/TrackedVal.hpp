#ifndef trackedval_HPP
#define trackedval_HPP
#include <sstream>
#include <unordered_map> 
#include "unitval.hpp"
#include <vector>

using namespace std;

  /**
   * \brief trackedval Class: class to track origin of carbon (or whatever) in various carbon pools in simple climate model Hector
   * Designed so that it can be dropped in place of a unitval in the Hector C++ code base
   */

class trackedval {

public:
    // public constructor (see private one below)
    trackedval();
    trackedval(Hector::unitval total, string pool);
    
    // math operations
    trackedval operator+(const trackedval& flux);
    trackedval operator-(const Hector::unitval flux);
    trackedval operator*(const double d);  // note corresponding non-member function declared below
    trackedval operator/(const double d);
    bool operator==(const trackedval& rhs);
    bool operator!=(const trackedval& rhs);


    bool identical(trackedval x) const;  // I don't know if this is needed or not

    // accessor functions
    vector<string> get_sources() const;
    double get_fraction(string source) const;
    Hector::unitval get_total() const;

    // setter functions
    void set(Hector::unitval val);
    void set( double val, Hector::unit_types units, string pool);

    // tracking
    bool isTracking() const;
    void setTracking(bool do_track);

    // make flux for pool addition
    trackedval flux_from_pool(const Hector::unitval fluxVal) const;

    // adjust pool size to match output from ODE solver
    trackedval adjust_pool(const Hector::unitval solvedSize);

    // pretty printing
    friend ostream& operator<<(ostream &out, trackedval &ct);


private:
     // total amount of whatever we're tracking
    Hector::unitval total;
    // unordered map holds the fractions of `total` (map value) corresponding to each source pool (map key)
    unordered_map<string, double> ctmap;
    // are we tracking?
    bool track;
    
    // internal constructor with explicit source pool map
    trackedval(Hector::unitval total, unordered_map<string, double> pool_map, bool do_track);
};

// Non-member function for multiplication with double as first argument
trackedval operator*(double d, const trackedval& ct);


#endif
