#ifndef __SIMULATION_STATE_H_
#define __SIMULATION_STATE_H_

#include "pointset.h"
#include "kernel.h"

namespace pp {

/*
 * Store some basic statistics.
 */
struct Statistics {
    Statistics(uint_t processes) : time(0), number_of_events(std::vector<uint_t>(processes,0)), total_events(0) {}
    
    double time;
    std::vector<uint_t> number_of_events;
    uint_t total_events;

    void update(double tau, uint_t rid) {
        time += tau;
        number_of_events[rid] += 1;
        total_events += 1;
    }
};


class SimulationState {
public:
    static constexpr unsigned int DIM = 2; // TODO: Generalise

    SimulationState(double u, uint_t me, uint_t re) : stats(Statistics(re)), U_value(u), max_entities(me) {
        for(auto i = 0u; i<max_entities+1; i++) {
            point_sets.push_back(std::make_shared<PointSet>(u, 1)); 
        }
    }

    inline void add(Point *p) { point_sets[p->get_entity()]->add(p); }
    inline Point *new_point(coord_t x, coord_t y, uint_t e) { return point_sets[e]->new_point(x, y, e); }
    inline Point *new_point(const Coord &c, uint_t e) { return new_point(c[0], c[1], e); } 

    inline void destroy_point(Point *p) { // invalidates reference p
        point_sets[p->get_entity()]->destroy_point(p);
    }

    inline rng_t &rng() { return rng_instance; }
    inline coord_t U() const { return U_value; }
    inline coord_t area() const { return U_value * U_value; }
    inline Coord center() const { return Coord(U_value/2, U_value/2); }

    /* Get the entity count */
    inline int get_max_entities() const { 
        return max_entities; 
    }

    /* Distance query around point p. Fill the query buffer with results */
    void query_points(uint_t entity, const Point *p, coord_t distance, point_query_t &buffer) {
        point_sets[entity]->get_within(p, distance, buffer);
    }

    /* Get number of points of given entity type */
    uint_t get_count(uint_t entity) const { 
        return point_sets[entity]->get_count(); 
    }

    /* Get total count of points */
    uint_t get_count() const {
        uint_t total = 0;
        for(const auto &p : point_sets) {
            total += p->get_count();
        }
        return total;
    }

    /* Return a vector containing all points. This will be invalidated once the state is modified */
    point_enum_buf_t &enumerate() { 
        enum_buffer.clear();
        enumerate(enum_buffer);
        return enum_buffer;
    }

    void enumerate(point_enum_buf_t &buffer) const {
        /* FIXME: There's some unnecessary copying going on here */
        for(auto &ps : point_sets) {
            for(auto &b : ps->buckets) {
                for(auto p : b) {
                    buffer.push_back(p);
                }
            }
        }
    }

    point_enum_buf_t enumerate_copy() const {
        point_enum_buf_t vs;
        enumerate(vs);
        return vs;
    }

    /* Random value from range [0,1) */
    inline double random_value() { 
        return uniform_distribution(rng_instance); 
    }

    /* Random coordinate in the UxU space */
    inline Coord random_coord() { 
        return Coord(static_cast<coord_t>(random_value() * U()), static_cast<coord_t>(random_value() * U())); 
    }
    
    /* Select a random point of given entity type */
    inline Point *random_point(uint_t entity) { 
        return point_sets[entity]->get_random(random_value()); 
    }

    template<typename T>
    void seed(T &s) {
        rng_instance.seed(s);
    }

    Statistics stats;
private:
    inline std::shared_ptr<PointSet> get_ps(uint_t entity) const {
        #ifdef PP_CHECK_BOUNDS
            return point_sets.at(entity);
        #else
            return point_sets[entity];
        #endif
    }

    coord_t U_value;
    uint_t max_entities; 
    point_enum_buf_t enum_buffer;
    std::vector<std::shared_ptr<PointSet>> point_sets; // indexing from 1.. max_entities
    rng_t rng_instance;
};


/* Print statistics */
std::ostream &operator<< (std::ostream &os, const pp::Statistics &s) {
    return os << "Statistics(events=" << s.total_events << "=" <<
        join(" + ", s.number_of_events) 
        << ", time=" << s.time << ")";
}

/**
 * Printing SimulationState.
 */
std::ostream &operator<< (std::ostream &os, const pp::SimulationState &m) {
    os << "SimulationState(U=" << m.U() << ", entities=" << m.get_max_entities();
    for(auto i = 1; i<=m.get_max_entities(); i++) {
        os << ", " << i << "#" << m.get_count(i);
    }
    os << "stats=";
    os << m.stats;
    os << ")";
    return os;
}

} // namespace


#endif
