#ifndef TrackedVal_HPP
#define TrackedVal_HPP
#include <sstream>
#include <unordered_map> 
#include "unitval.hpp"
#include <vector>

using namespace std;

  /**
   * \brief TrackedVal Class: class to track origin of carbon (or whatever) in various carbon pools in simple climate model Hector
   * Designed so that it can be dropped in place of a unitval in the Hector C++ code base
   */

class TrackedVal {

public:
    // public constructor (see private one below)
    TrackedVal();
    TrackedVal(Hector::unitval total, string pool);
    
    // assignment operator
    TrackedVal& operator=(TrackedVal other);
    
    // math operations
    TrackedVal operator+(const TrackedVal& flux);
    TrackedVal operator-(const Hector::unitval flux);
    TrackedVal operator*(const double d);  // note corresponding non-member function declared below
    TrackedVal operator/(const double d);
    bool operator==(const TrackedVal& rhs);
    bool operator!=(const TrackedVal& rhs);

    // TEMPORARY UNTIL ALL POOLS ARE CONVERTED
    TrackedVal operator+(const Hector::unitval flux);

    bool identical(TrackedVal x) const;  // I don't know if this is needed or not

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

    // pretty printing
    friend ostream& operator<<(ostream &out, TrackedVal &ct);


private:
     // total amount of whatever we're tracking
    Hector::unitval total;
    // unordered map holds the fractions of `total` (map value) corresponding to each source pool (map key)
    unordered_map<string, double> ctmap;
    // are we tracking?
    bool track;
    
    // internal constructor with explicit source pool map
    TrackedVal(Hector::unitval total, unordered_map<string, double> pool_map, bool do_track);
};

// Non-member function for multiplication with double as first argument
TrackedVal operator*(double d, const TrackedVal& ct);

#endif
