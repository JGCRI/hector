
#include <sstream>
#include <unordered_map>
#include "TrackedVal.hpp"
#include "unitval.hpp"

using namespace std;

// Public constructors
TrackedVal::TrackedVal(){
    track = false;
    Hector::unitval total_val(0, Hector::U_UNITLESS);
}

TrackedVal::TrackedVal(Hector::unitval total, string pool){
    track = false;
    this->total = total;
    ctmap[pool] = 1;
}

// Private constructor with explicit source pool map
TrackedVal::TrackedVal(Hector::unitval total, unordered_map<string, double> pool_map, bool do_track){
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

TrackedVal& TrackedVal::operator=(TrackedVal other){
    this->total = other.total;
    this->ctmap = other.ctmap;
    return *this;
}

bool TrackedVal::isTracking() const {
    return track;
}

void TrackedVal::setTracking(bool do_track){
    track = do_track;
}

// Accessor: return a string vector of the current sources
std::vector<std::string> TrackedVal::get_sources() const {
    H_ASSERT(track, "get_sources() requires tracking to be on");
    std::vector<std::string> sources;
    for (auto itr = ctmap.begin(); itr != ctmap.end(); itr++) {
        sources.push_back(itr->first);
    }
    return sources;
}

// Accessor: return total amount
Hector::unitval TrackedVal::get_total() const {
    return total;
}

// Accessor: return the fraction corresponding to a specific source
double TrackedVal::get_fraction(string source) const {
    H_ASSERT(track, "get_fraction() requires tracking to be on");
    double val = 0.0;  // 0.0 is returned if not in our map
    auto x = ctmap.find(source);
    if(x != ctmap.end()) {
        val = x->second;
    }
    return val;
}

// Setter: set the total value and units; the map only has the pool named below with frac 1
void TrackedVal::set( double val, Hector::unit_types units, string pool){
    Hector::unitval total_val(val, units);
    this->total = total_val;
    ctmap[pool] = 1;
}

void TrackedVal::set(Hector::unitval val){
    this->total = val;
}

// Addition: the complicated one
TrackedVal TrackedVal::operator+(const TrackedVal& flux){
    Hector::unitval new_total = total + flux.total;
    unordered_map<string, double> new_origins;
    
    if(track) {
        H_ASSERT(flux.isTracking(), "tracking mismatch")
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
        H_ASSERT(!flux.isTracking(), "tracking mismatch")
    }
    
    TrackedVal added_flux(new_total, new_origins, track);
    return added_flux;
}

// TEMPORARY: needed until all pools are converted in TrackedVal objects
TrackedVal TrackedVal::operator+(const Hector::unitval flux){
    Hector::unitval new_total = total + flux;
    unordered_map<string, double> new_origins;

    if(track){
        string not_tracked("not tracked");
        unordered_map<string, Hector::unitval> new_pools;
        if(ctmap.find(not_tracked) == ctmap.end()){
            new_pools[not_tracked] = flux;
        }
        for (auto itr = ctmap.begin(); itr != ctmap.end(); itr++) {
            if(itr->first == not_tracked){
                new_pools[itr->first] = itr->second*total + flux;
            }
            else{
                new_pools[itr->first] = itr->second*total;
            }
        }
        for (auto itr = new_pools.begin(); itr != new_pools.end(); itr++) {
            if(new_total) {
                new_origins[itr->first] = itr->second / new_total;
            } else {  // uh oh, new total is zero
                new_origins[itr->first] = 1 / new_pools.size();
            }
        }
    }
    TrackedVal added_flux(new_total, new_origins, track);
    return added_flux;
}


// Because we track a total and source fractions, subtraction is trivial
TrackedVal TrackedVal::operator-(const Hector::unitval flux){
    TrackedVal sub_ct(total - flux, ctmap, track);
    return sub_ct;
}

// Multiplication member function (when this object is lhs, and double rhs)
TrackedVal TrackedVal::operator*(const double d){
    TrackedVal ct(total * d, ctmap, track);
    return ct;
}
// ...nonmember function for when object is rhs; just flip and call member function
TrackedVal operator*(double d, const TrackedVal& ct){
    TrackedVal x = ct; // need to make non-const
    return x * d;
}

// Division
TrackedVal TrackedVal::operator/(const double d){
    TrackedVal ct(total / d, ctmap, track);
    return ct;
}

// Equality: same total only
bool TrackedVal::operator==(const TrackedVal& rhs){
    return total == rhs.get_total();
}
bool TrackedVal::operator!=(const TrackedVal& rhs){
    return total != rhs.get_total();
}

// Identicality: same total, same tracking, same sources, same fractions
bool TrackedVal::identical(TrackedVal x) const {
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

// Printing
ostream& operator<<(ostream &out, TrackedVal &ct ){
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
