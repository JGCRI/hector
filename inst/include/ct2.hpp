#ifndef CT2_HPP
#define CT2_HPP
#include <sstream>
#include <unordered_map> 
#include "unitval.hpp"
#include <vector>

using namespace std;

  /**
   * \brief CT2 Class: class to track origin of carbon (or whatever) in various carbon pools in simple climate model Hector
   * Designed so that it can be dropped in place of a unitval in the Hector C++ code base
   */

class CT2 {

public:
    // public constructor (see private one below)
    CT2(Hector::unitval total, string pool);
    
    // math operations
    CT2 operator+(const CT2& flux);
    CT2 operator-(const Hector::unitval flux);
    CT2 operator-(const CT2& ct);
    CT2 operator*(const double d);  // note corresponding non-member function declared below
    CT2 operator/(const double d);
    bool operator==(const CT2& rhs);
    bool operator!=(const CT2& rhs);

    bool identical(CT2 x) const;  // I don't know if this is needed or not

    // accessor functions
    vector<string> get_sources() const;
    double get_fraction(std::string source) const;
    Hector::unitval get_total() const;

    // tracking
    bool isTracking() const;
    void setTracking(bool do_track);

    // pretty printing
    friend ostream& operator<<(ostream &out, CT2 &ct);


private:
     // total amount of whatever we're tracking
    Hector::unitval total;
    // unordered map holds the fractions of `total` (map value) corresponding to each source pool (map key)
    unordered_map<string, double> ctmap;
    // are we tracking?
    bool track;
    
    // internal constructor with explicit source pool map
    CT2(Hector::unitval total, unordered_map<string, double> pool_map, bool do_track);
};

// Non-member function for multiplication with double as first argument
CT2 operator*(double d, const CT2& ct);

#endif
