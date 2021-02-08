
#include <sstream>
#include <unordered_map>
#include "trackedval.hpp"
#include "unitval.hpp"

using namespace std;

// Public constructors
trackedval::trackedval(){
    track = false;
    Hector::unitval total_val(0, Hector::U_UNITLESS);
}

trackedval::trackedval(Hector::unitval total, string pool){
    track = false;
    this->total = total;
    ctmap[pool] = 1;
}

// Private constructor with explicit source pool map
trackedval::trackedval(Hector::unitval total, unordered_map<string, double> pool_map, bool do_track){
    track = do_track;
    this->total = total;
    ctmap = pool_map;
    
    // check pool_map data
    double frac = 0.0;
    for (auto itr = ctmap.begin(); itr != ctmap.end(); itr++) {
        H_ASSERT(itr->second >= 0 && itr->second <= 1, "fractions must be 0-1");
        frac += itr->second;
    }
    H_ASSERT(frac - 1.0 < 1e-6, "pool_map must sum to ~1.0")
}

bool trackedval::isTracking() const {
    return track;
}

void trackedval::setTracking(bool do_track){
    track = do_track;
}

// Accessor: return a string vector of the current sources
std::vector<std::string> trackedval::get_sources() const {
    H_ASSERT(track, "get_sources() requires tracking to be on");
    std::vector<std::string> sources;
    for (auto itr = ctmap.begin(); itr != ctmap.end(); itr++) {
        sources.push_back(itr->first);
    }
    return sources;
}

// Accessor: return total amount
Hector::unitval trackedval::get_total() const {
    return total;
}

// Accessor: return the fraction corresponding to a specific source
double trackedval::get_fraction(string source) const {
    H_ASSERT(track, "get_fraction() requires tracking to be on");
    double val = 0.0;  // 0.0 is returned if not in our map
    auto x = ctmap.find(source);
    if(x != ctmap.end()) {
        val = x->second;
    }
    return val;
}

// Setter: set the total value and units; the map only has the pool named below with frac 1
void trackedval::set( double val, Hector::unit_types units, string pool){
    Hector::unitval total_val(val, units);
    this->total = total_val;
    ctmap.clear();
    ctmap[pool] = 1;
}

// Setter: use to set pool total - use sparingly because doesn't change map
void trackedval::set(Hector::unitval val){
    this->total = val;
}

// Addition: the complicated one
trackedval trackedval::operator+(const trackedval& flux){
    Hector::unitval new_total = total + flux.total;
    unordered_map<string, double> new_origins;
    if(track) {
        H_ASSERT(flux.isTracking(), "tracking mismatch")
        if(flux.total > 0) {
            unordered_map<string, Hector::unitval> new_pools;
        
            // Look through *our* sources, and if any in other object, add
            for (auto itr = ctmap.begin(); itr != ctmap.end(); itr++) {
                new_pools[itr->first] = total * itr->second + flux.total * flux.get_fraction(itr->first);
            }
            
            // Look through the *other* object sources, and if any NOT in our map, add
            std::vector<std::string> sources = flux.get_sources();
            for (int i = 0; i < sources.size(); i++) {
                const string src = sources[i];
                if(ctmap.find(src) == ctmap.end()) {  // source that's not in our map
                    new_pools[src] = flux.total * flux.get_fraction(src);
                }
            }
            
            // Now that we have the new pool values, compute new fractions
            for (auto itr = new_pools.begin(); itr != new_pools.end(); itr++) {
                if(new_total) {
                    new_origins[itr->first] = itr->second / new_total;
                } else {  // uh oh, new total is zero
                    new_origins[itr->first] = 1 / new_pools.size();
                }
            }
        } else {
            new_origins = ctmap;
        }
        
    } else {
        H_ASSERT(!flux.isTracking(), "tracking mismatch")
    }
    
    trackedval added_flux(new_total, new_origins, track);
    return added_flux;
}


// Because we track a total and source fractions, subtraction is trivial
trackedval trackedval::operator-(const Hector::unitval flux){
    H_ASSERT(flux >= 0, "Subtraction of a negative from a trackedval not allowed");
    trackedval sub_ct(total - flux, ctmap, track);
    return sub_ct;
}

// Multiplication member function (when this object is lhs, and double rhs)
trackedval trackedval::operator*(const double d){
    trackedval ct(total * d, ctmap, track);
    return ct;
}
// ...nonmember function for when object is rhs; just flip and call member function
trackedval operator*(double d, const trackedval& ct){
    trackedval x = ct; // need to make non-const
    return x * d;
}

// Division
trackedval trackedval::operator/(const double d){
    trackedval ct(total / d, ctmap, track);
    return ct;
}

// Equality: same total only
bool trackedval::operator==(const trackedval& rhs){
    return total == rhs.get_total();
}
bool trackedval::operator!=(const trackedval& rhs){
    return total != rhs.get_total();
}

// Identicality: same total, same tracking, same sources, same fractions
bool trackedval::identical(trackedval x) const {
    bool same = total == x.get_total();
    
    same = same && x.isTracking() == track;
    
    if(x.isTracking() && track) {
        std::vector<std::string> xsources = x.get_sources();
        std::vector<std::string> sources = get_sources();
        same = same && xsources == sources;
        
        for (int i = 0; i < xsources.size(); i++) {
            same = same && x.get_fraction(xsources[i]) == get_fraction(sources[i]);
        }
    }

    return same;
}

// Creates a flux from a pool so the flux can be added to other pools
trackedval trackedval::flux_from_pool(Hector::unitval fluxVal) const{
    trackedval flux(fluxVal, ctmap, track);
    return flux;
}

// adjusts pools by the flux given - accounts for if flux is positive or negative
void trackedval::adjust_pools_by_flux(trackedval& fluxTo, Hector::unitval fluxSize){
    trackedval fluxFrom(total, ctmap, track);
    H_ASSERT(fluxFrom.get_total() >= 0, "Function adjust_pools_by_flux does not work on negative pools");
    H_ASSERT(fluxTo.get_total() >= 0, "Function adjust_pools_by_flux does not work on negative pools");

    if(fluxSize > 0) {
        fluxTo = fluxTo + fluxFrom.flux_from_pool(fluxSize);
        fluxFrom = fluxFrom - fluxSize;
    }
    else { // if flux is negative, then it is actually entering the "from" pool
        fluxSize = -fluxSize;
        fluxFrom = fluxFrom + fluxTo.flux_from_pool(fluxSize);
        fluxTo = fluxTo - fluxSize;
    }
    *this = fluxFrom;
}


// Adjusts pool size to match that output by the ODE solver
trackedval trackedval::adjust_pool_to_val(const Hector::unitval solvedSize){
    trackedval adjusted(total, ctmap, track);
    Hector::unitval untracked = solvedSize - total;
    if(track) {
        if(untracked > 0){
            trackedval flux(untracked, "not tracked");
            flux.setTracking(true);
            adjusted = adjusted + flux;
        }
        else{
            untracked = -untracked; // need positive val to subtract
            adjusted = adjusted - untracked;
        }
    }
    else{
        adjusted.set(solvedSize);
    }
    return adjusted;
}

// Printing
ostream& operator<<(ostream &out, trackedval &ct ){
    out << ct.total;
    if(ct.isTracking()) {
        out << endl;
        std::vector<std::string> sources = ct.get_sources();
        for (int i = 0; i < sources.size(); i++) {
            out << "\t" << sources[i] << ": " << ct.get_fraction(sources[i]) << endl;
        }
    }
    return out;
}