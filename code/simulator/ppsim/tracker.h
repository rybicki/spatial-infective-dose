#ifndef __TRACKER_H_
#define __TRACKER_H_

#include "point.h"
#include "configuration.h"
#include "sprocess.h"
#include "simulation_state.h"
#include "pointset.h"

#include <vector>
#include <initializer_list>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <stdexcept>

#include <boost/pool/pool_alloc.hpp>

namespace pp {

/*
 * Process tracker interface. KISS.
 */
class Tracker {
public:
    Tracker() : simulation_state(nullptr) {}
    virtual void activate(point_del_buf_t &removed, point_add_buf_t &added) = 0; 
    virtual double propensity() const = 0; 
    virtual void notify_removal(Point &p) = 0;
    virtual void notify_add(Point &p) = 0;
    virtual const IProcess &get_process() const = 0;

    /* Before simulation starts, initialise the tracker with this */
    void initialise(SimulationState *s) { 
        DMSG("Initialised process " << get_process());
        simulation_state = s; 
    }
protected:
    SimulationState *simulation_state;
};

/* Implementations of the Tracker interface are as follows */
template<typename P, int INPUTS>
class ImplTracker;

template<typename P>
class ImplTracker<P, 0> : public Tracker {
public:
    static_assert(P::input_count == 0, "ImplTracker<P,0> called with a process that does not have 0 inputs");
    ImplTracker<P,0>(P p) : process(p) { }

    inline void activate(point_del_buf_t &removed, point_add_buf_t &added) {
        process.activate(*simulation_state, removed, added);
    }

    const IProcess &get_process() const { return process; }

    double propensity() const { return process.propensity(*simulation_state); }
    void notify_removal(Point &p) { }
    void notify_add(Point &p) { }
protected:
    P process;
};

template<typename P>
class ImplTracker<P, 1> : public Tracker {
public:
    static_assert(P::input_count == 1, "Trying to generate ImplTracker<P,1> with a process that does not have 1 input");
    ImplTracker<P,1>(P p) : process(p) {}

    inline void activate(point_del_buf_t &removed, point_add_buf_t &added) {
        auto p = simulation_state->random_point(process.input(0));
        process.activate(*simulation_state, p, removed, added);
    }

    const IProcess &get_process() const { return process; }

    /* for single point processes the propensity is given by the number of points */
    inline double propensity() const { 
        return process.propensity() * simulation_state->get_count(process.input(0));
    }
    
    /* no need to update internal state here; simulation_state's pointsets do it for us */
    void notify_removal(Point &p) { }
    void notify_add(Point &p) { } 
protected:
    P process;
};


template<typename P>
class ImplTracker<P, 2> : public Tracker {
public:
    static_assert(P::input_count == 2, "Trying to generate ImplTracker<P,2> with a process that does not have two inputs");

    ImplTracker<P,2>(P p) : process(p) {
        compute_entity_index_mapping();
    }

    inline void activate(point_del_buf_t &removed, point_add_buf_t &added) {
        auto rval = simulation_state->random_value();
        auto c = configurations.get(rval);
        process.activate(*simulation_state, *c, removed, added);
    }

    const IProcess &get_process() const { return process; }

    inline double propensity() const { 
        /* NOTE: We assume Tophat kernel everywhere, that is, each configuration has the same propensity */
        return configurations.get_total_weight() * process.propensity(); 
    }
    
    void notify_removal(Point &p) {
        DMSG("ImplTracker<2>::notify_removal(" << p << " = " << &p <<  ")" << " (Tracking " << process << ")");
        for(auto index : entity_indices[p.get_entity()]) {
            populate_query_buffers(&p, index);
            /* Generate configurations */
            for(auto p1 : query_results[0]) {
                for(auto p2 : query_results[1]) {
                    assert(p1->get_entity() == process.input(0));
                    assert(p2->get_entity() == process.input(1));
                    configurations.find_and_destroy(p1, p2);
                }
            }
        }  
        assert(!configurations.contains(&p));
    }

    void notify_add(Point &p) { 
        DMSG("ImplTracker<2>::notify_add(" << p << " = " << &p << ")" << " (Tracking " << process << ")");
        for(auto index : entity_indices[p.get_entity()]) {
            populate_query_buffers(&p, index);
            /* Generate configurations */
            for(auto p1 : query_results[0]) {
                for(auto p2 : query_results[1]) {
                    assert(p1->get_entity() == process.input(0));
                    assert(p2->get_entity() == process.input(1));
                    auto w = process.propensity(*simulation_state, *p1, *p2);
                    if (w > 0) {
                        auto c = configurations.create(w, p1, p2);
                        configurations.add(c);
                        DMSG("ImplTracker<2> added configuration " << *c);
                    }
                }
            }
        } 
    } 

protected:
    using entity_indices_t = std::vector<std::vector<uint_t>>; 
    using query_results_t = std::array<point_query_t,2>;

    void compute_entity_index_mapping() {
        // Compute entity -> indices mapping
        // Find unique entitites
        std::set<uint_t> entities;
        for(auto i = 0u; i<process.input_count; i++) {
            entities.insert(process.input(i));
        }   
        auto max = *entities.rbegin();
        entity_indices = entity_indices_t(max+1);
        // Find indices
        for(auto e : entities) {
            entity_indices[e] = std::vector<uint_t>();
            for (auto i = 0u; i<process.input_count; i++) {
                if (process.input(i) == e) {
                    entity_indices[e].push_back(i);
                }
            }
        }
    }

    void populate_query_buffers(Point *p, int index) {
        for(auto i = 0u; i < process.input_count; i++) {
            query_results[i].clear();
            if (index == i) { // focal point
                query_results[i].push_back(p);
            } else { // get other points nearby
                auto entity = process.input(i);
                simulation_state->query_points(entity, p, process.get_input_radius(), query_results[i]);
            }
        }
    }

    P process;
    ConfigurationSet<2> configurations;
    entity_indices_t entity_indices;
    query_results_t query_results;
};

template <typename P>
std::unique_ptr<Tracker> make_tracker(P p) {
    return std::unique_ptr<Tracker>(new ImplTracker<P,P::input_count>(p));
}

} // namespace

#endif
