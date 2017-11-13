#ifndef __CONFIGURATION_H_
#define __CONFIGURATION_H_

#include "common.h"
#include "point.h"
#include "accumulator.h"

#include <unordered_map>
#include <unordered_set>
#include <forward_list>
#include <list>
#include <iostream>
#include <functional>
#include <boost/pool/pool_alloc.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/functional/hash.hpp>

#include <algorithm>

namespace pp {

template<int IN>
struct NConfiguration {
    NConfiguration() {}
    template<typename... Args>
    NConfiguration(double w, const Args... args) : points{{args...}}, weight(w) {}
    static constexpr uint_t length = IN;
    std::array<Point*,IN> points;
    double weight;
    inline Point *operator[](const int index) const { 
        assert(index < IN);
        return points[index]; 
    }

    using set_t = std::list<NConfiguration*,boost::fast_pool_allocator<NConfiguration*>>;
    typename set_t::iterator it;
private:
    NConfiguration(NConfiguration &) {} // Prevent copying
};

template<int IN>
std::ostream &operator<< (std::ostream &os, const NConfiguration<IN> &c) {
    os << "Configuration({" << join(", ", c.points) << "}, ";
    os << "weight=" << c.weight <<")";
    return os;
}

/* Configuration container / pool */
template<int IN>
class ConfigurationSet {
public:
    using Configuration = NConfiguration<IN>;
    ConfigurationSet() : ConfigurationSet(DEFAULT_BUCKET_COUNT) {}

    ConfigurationSet(uint_t bc) : accumulator(Accumulator<unsigned long>(bc, log2(bc))) {
        buckets.resize(bc);
    }

    double get_total_weight() const { 
        return accumulator.get_nodes()[0]; 
    }

    uint_t size() const { 
        auto total = 0;
        for(auto s : buckets) {
            total += s.size();
        }
        return total;
    }
    
    template<typename... Args>
    inline Configuration* create(const Args... args) {
        return pool.construct(args...);
    }

    inline void destroy(Configuration *c) { 
        assert(!contains(c) /* Trying to DESTROY a configuration that is still in the data structure */);
        pool.destroy(c); 
    }

    void add(Configuration *c) {
        DMSG("Adding configuration " << *c );
        assert(!contains(c) /* Attempting to add a configuration that already exists */);
        auto b = get_bucket(c);
        buckets[b].push_back(c);
        c->it = std::prev(buckets[b].end());
        accumulator.increment(b, 1); // c->weight);
        assert(contains(c) /* Configuration set should contain a configuration that was just added */);
    }

    void remove(Configuration *c) {
        DMSG("remove(" << c << ")");
        assert(contains(c) && "Attempting to REMOVE a configuration that does not exist");
        auto b = get_bucket(c);
        auto it = c->it;
        buckets[b].erase(it);
        accumulator.increment(b, -1); // -c->weight);
        assert(!contains(c) && "Configuration set should not contain a configuration that was just removed");
    }

    template<typename... Args>
    void find_and_destroy(const Args&... args) {
        std::array<Point*,2> p = {{args...}};
        DMSG("Trying to find configuration matching " << join(" ", p));
        auto b = get_bucket_ps(p);
        bool ok;
        Configuration *found = nullptr;
        for(auto c : buckets[b]) {
            ok = true;
            for(auto i = 0u; i<IN; i++) {
                if (c->points[i] != p[i]) {
                    ok=false;
                    break;
                }
            }
            if (ok) {
                found = c;
                break;
            }
        }

        if (found == nullptr) {
            DMSG("No match for configuration " << join(" ", p));
            throw std::runtime_error("Could not find a matching configuration");
        }

        remove(found);
        destroy(found);
    }

    /* Check whether some stored configuration points to point p. This is for debugging purposes. */
    bool contains(Point *p) {
        for(auto &b : buckets) {
            for(auto &c : b) {
                for(Point* q : c->points) {
                    if (q == p) {
                        DMSG(*c << " contains " << p);
                        return true;
                    }
                }
            }
        }
        return false;
    }

    /* Check whether a given configuration is contained in one of the buckets */
    bool contains(Configuration *conf) {
        for(auto &b : buckets) {
            for(auto &c : b) {
                if(conf == c) {
                    return true;
                }
            }
        }
        return false;
    }

    const Configuration *get(double rval) const {
        assert(rval >= 0 && rval < 1);
        return get_random(rval);
        //return get_by_weight(rval * get_total_weight());
    }

    const Configuration *get_by_weight(double weight) const {
        assert(weight >= 0);
        assert(weight <= get_total_weight());
        auto x = accumulator.find_start_location(weight); // use the accumulator structure to find approximate bucket to start search in 
        auto start_loc = x.first; // bucket locations from which to start the search 
        auto remaining_weight = x.second; // how much weight needs to be skipped still 

        assert(start_loc >= 0); 
        assert(start_loc < buckets.size());
        while (remaining_weight >= 0 && start_loc < buckets.size()) {
            for(auto c : buckets[start_loc]) {
                remaining_weight -= c->weight;
                if (remaining_weight <= 0) return c;
            }
            start_loc += 1;  // if still here, need to continue onto the next bucket..
        }
        DMSG("get_by_weight("<<weight<<")");
        print_stats();
        throw std::runtime_error("Configuration::get_by_weight did not find an item");
    }

    Configuration *get_random(double rval) const {
        assert(rval < 1);
        assert(rval >= 0);
        assert(get_count()>0);
        uint_t n = get_count()*rval; // return nth point
        return get_nth(n);
    }

    Configuration *get_nth(int n) const {
        assert(get_count()>=n);
        auto r = accumulator.find_start_location(n);
        auto start = r.first;
        auto remaining = r.second;

        for(auto i = start; i<buckets.size(); i++) {
            for(auto c : buckets[i]) {
                if (remaining == 0) return c;
                remaining -= 1;
            }
        }

        /* Something went wrong! */
        DMSG("remaining " << remaining);
        std::stringstream s;
        s <<"Could not find nth configuration";
        s << "in get_nth(" << n << ")" << " remaining = " << remaining;
        s << " count=" << get_count();
        throw std::runtime_error(s.str());
    }

    uint_t get_count() const { 
        return accumulator.get_nodes()[0]; 
    }

    void print_stats() const {
        int min=-1, max=0, sum=0;
        for(auto i : accumulator.leaves()) {
            if (min == -1 || i < min) min=i;
            if (i > max) max = i;
            sum += i;
        }
        double avg = sum / accumulator.leaves().size();
        std::cout << "Total: " << accumulator.get_nodes()[0] << " Min: " << min << " Max: " << max << " Avg: " << avg << std::endl;
    }

protected:
    inline uint_t get_bucket_ps(std::array<Point*,IN> points) const {
        size_t hash = 0;
        for(auto p : points) {
            boost::hash_combine(hash, p->hash());
        }
        return hash % buckets.size();
    }

    inline uint_t get_bucket(const Configuration *c) const { 
        return get_bucket_ps(c->points);
    }

    static constexpr uint_t DEFAULT_BUCKET_COUNT = 4096;
    boost::object_pool<Configuration> pool;
    std::vector<typename Configuration::set_t,boost::fast_pool_allocator<typename Configuration::set_t>> buckets;
    Accumulator<unsigned long> accumulator;
};

}

#endif
