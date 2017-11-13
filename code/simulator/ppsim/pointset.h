#include <unordered_set>
#include <vector>
#include <forward_list>
#include <cassert>
#include <stdexcept>
#include <sstream>

#include "accumulator.h"
#include "point.h"
#include "common.h"

#ifndef __POOL_HASHSET_H_
#define __POOL_HASHSET_H_

namespace pp {

static boost::object_pool<Point> point_pool;

class PointSet {
public:
    PointSet(coord_t U_, coord_t bw) : U(U_), bucket_width(bw) {
        row_length = ceil(U/bucket_width);
        bucket_count = row_length * row_length;
        norm_coord = row_length/U;
        buckets.resize(bucket_count);
        for(auto i = 0u; i<bucket_count; i++) {
            buckets[i] = bucket_t();
        }

        auto depth = log2(bucket_count);
        accumulator = std::unique_ptr<Accumulator<long int>>(new Accumulator<long int>(bucket_count, depth));
    }

    ~PointSet() {
        buckets.clear();
    }

    uint_t get_count() const { 
        return accumulator->get_nodes()[0]; 
    }

    /* Allocate a new point but do NOT yet add it into the data structure */
    inline Point *new_point(coord_t x, coord_t y, uint_t e) {
        //auto p = new Point(x,y,e); 
        auto p = point_pool.construct(x,y,e);
        DMSG("PointSet::new_point(" << x << ", " << y << ", " << e << ") = " << p);
        assert(x >= 0 && x < U && "x-coord out of bounds; is your domain size too small?");
        assert(y >= 0 && y < U && "x-coord out of bounds; is your domain size too small?");
        assert(p->get_entity() == e);
        assert((*p)[0] == x);
        assert((*p)[1] == y);
        p->bucket = get_bucket(p);
        return p;
    }

    void destroy_point(Point *p) {
        DMSG("PointSet::destroy_point(" << p << ") which is " << *p <<")");
        assert(contains(p));
        remove(p);
        assert(!contains(p));
        point_pool.destroy(p); // This will invalidate the pointer p!!!
    }

    bool contains(const Point* p) const {
        assert(p != nullptr);
        auto b = p->bucket; 
        auto it = std::find(buckets[b].begin(), buckets[b].end(), p); 
        return (it != buckets[b].end());
    }

    void get_within(const Point *p, double distance, point_query_t &buffer) const {
        DMSG("get_within(" << *p << ", " << distance);
        auto cdistance = int((distance / bucket_width + 0.5));
        DMSG("cdistance="<<cdistance);
        if (2*cdistance + 1 >= row_length) {
            get_within_bruteforce(p, distance, buffer);
        } else {
            get_within_clever(p, distance, cdistance, buffer);
#if DEBUG
    DMSG("Testing whether point query computes correct solution"); 
    point_query_t test;
    get_within_bruteforce(p, distance, test);
    for(auto q : test) {
        bool found = std::find(buffer.begin(), buffer.end(), q) != buffer.end();
        if (!found) {
            std::cout << "Did not find " << *q << " within " << *p << std::endl;
            assert(false);
        }
    }
    DMSG("Bruteforce produced same result as clever find.");
#endif
        }
    }

    Point *get_random(double rval) {
        DMSG("get_random(" << rval << ")");
        assert(rval < 1);
        assert(rval >= 0);
        assert(get_count()>0);
        uint_t n = get_count()*rval; // return nth point
        return get_nth(n);
    }

    Point *get_nth(int n) {
        assert(get_count()>=n);
        auto r = accumulator->find_start_location(n);
        auto start = r.first;
        auto remaining = r.second;
        DMSG("finding " << n << "th: start at " << start << " and skip " << remaining);

        for(auto i = start; i<buckets.size(); i++) {
            for(auto p : buckets[i]) {
                if (remaining == 0) return p;
                remaining -= 1;
            }
        }

        /* Something went wrong! */
        DMSG("remaining " << remaining);
        std::stringstream s;
        s <<"Could not find nth point";
        s << "in get_nth(" << n << ")" << " remaining = " << remaining;
        s << " count=" << get_count();
        throw std::runtime_error(s.str());
    }

    void add(Point *p) {
        DMSG("PointSet::add(" << p << ") which is " << *p);
        assert(!contains(p) && "Adding a point that already has been added!");
        auto b = p->bucket; 
        buckets[b].push_back(p); // list
        p->it = std::prev(buckets[b].end());
        assert(p == *p->it);

        accumulator->increment(b,1);
        assert(contains(p) && "A point that was just added should be found!");
    }


private:
    friend class SimulationState;
    /* implementation details */
    void remove(Point *p) {
        auto b = p->bucket; 
        buckets[b].erase(p->it);
        accumulator->increment(b,-1);        
    }

    inline std::pair<int,int> get_bucket_coords(const Point *p) const {
        auto x = int((*p)[0] * norm_coord);
        auto y = int((*p)[1] * norm_coord);
        return std::pair<int,int>(x,y);
    }

    inline int wrap_bucket_coord(int x) const {
        if (x < 0) x = row_length + x;
        else if (x >= row_length) x = x - row_length;
        return x;
    }

    inline std::pair<int,int> wrap_bucket_coords(int x, int y) const {
        return std::pair<int,int>(wrap_coord<int>(x, row_length), wrap_coord<int>(y, row_length));
    }

    inline uint_t get_bucket_index(std::pair<int,int> cs) const {
        return cs.first + cs.second*row_length;
    }

    inline uint_t get_bucket(const Point* p) const {
        auto cs = get_bucket_coords(p);
        return get_bucket_index(cs);
    }

    void get_within_clever(const Point *p, coord_t distance, int cdistance, point_query_t &buffer) const {
        auto dsquared = distance*distance;
        auto cs = get_bucket_coords(p); /* bucket (x,y) which contains the focal point p */
        auto x = cs.first;
        auto y = cs.second;
        for(auto dx = -cdistance; dx <= cdistance; dx++) {
            for(auto dy = -cdistance; dy <= cdistance; dy++) {
                auto ws = wrap_bucket_coords(x+dx, y+dy);
                auto b = get_bucket_index(ws);
                for(const auto &q : buckets[b]) {
                    if (p->torus_squared_distance(*q, U) <= dsquared && !(p == q)) {
                        buffer.push_back(q);
                    }
                }
            }
        }
    }

    void get_within_bruteforce(const Point *p, double distance, point_query_t &buffer) const {
        auto dsquared = distance*distance;
        for(auto b=0; b<buckets.size(); b++)  {
            for(const auto &q : buckets[b]) {
                if (p->torus_squared_distance(*q,U) <= dsquared && !(p == q)) {
                    buffer.push_back(q);
                } 
            }
        }
    }

    bucket_list_t buckets;
    std::unique_ptr<Accumulator<long int>> accumulator;

    //uint_t count;
    coord_t norm_coord;
    const coord_t U, bucket_width;
    uint_t row_length, bucket_count;
};

}


#endif
