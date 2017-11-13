#ifndef __MODEL_H_
#define __MODEL_H_

#include <memory>
#include <set>
#include <stdexcept>

#include "sprocess.h"
#include "tracker.h"

namespace pp {
/* 
 *  Model specification interface.
 */
class Model {
public:
    //friend class Simulator;
    friend std::ostream &operator<< (std::ostream &os, const Model &m);

    using trackers_t = std::vector<std::shared_ptr<Tracker>>;
    using dependency_list_t = std::vector<std::vector<uint_t>>;
    using entities_t = std::set<uint_t>;

    template<typename P>
    void add(const P p) {
        if (initialised) throw std::runtime_error("Trying to add a new process to an already initialised model");
        trackers.push_back(make_tracker(p));
        update_entities(p);
    }

    template<typename P>
    Model &operator+=(const P p) {
        add(p);
        return *this;
    }
    
    template<typename P>
    Model &operator+(const P p) {
        add(p);
        return *this;
    }

    uint_t max_entity_id() const { return *entities.rbegin(); }
    uint_t process_count() const { return trackers.size(); }
    void initialise(SimulationState *s) { // friend class Simulator will call this
        // initialise the trackers with a pointer to the simulation state
        for(const auto &p : trackers) {
            p->initialise(s);
        }
    }

    void done() {
        compute_dependencies();
        initialised = true;
    }

    void update_entities(const IProcess &p) {
        for(auto i = 0u; i<p.get_input_count(); i++) {
            entities.insert(p.input(i));
        }   
        for(auto i = 0u; i<p.get_output_count(); i++) {
            entities.insert(p.output(i));
        }   
    }

    void compute_dependencies() {
        auto max_entity = max_entity_id();
        dependencies = dependency_list_t(max_entity+1, std::vector<uint_t>(0));

        // Identify dependencies
        for(auto i = 0u; i<trackers.size(); i++) {
            auto &p = trackers[i]->get_process();
            for(auto j = 0u; j<p.get_input_count(); j++) {
                auto e = p.input(j);
                dependencies[e].push_back(i);
            }
        }
    }

    std::vector<uint_t> &get_dependencies(uint_t entity) {
        return dependencies.at(entity);
    }

    std::shared_ptr<Tracker> &get_tracker(uint_t rid) {
        return trackers.at(rid);
    }

    trackers_t &get_trackers() {
        return trackers;
    }

private:
    bool initialised;
    entities_t entities; // list of entities
    dependency_list_t dependencies; // entity -> process dependency mapping
    trackers_t trackers; // give the tracker for ith process
};


/**
 * Print model objects to streams.
 */
std::ostream &operator<< (std::ostream &os, const Model &m) {
    os << "Model(entities=[" << join(", ", m.entities) << "]," << std::endl;
    os << "      dependencies=[";
    for(auto e : m.entities) {
        os << e << " -> [" << join(", ", m.dependencies[e]) << "] ";
    }
    os << "]," << std::endl;

    os << "      processes=["<<std::endl;

    for(auto i = 0u; i<m.trackers.size(); i++) {
        auto &p = m.trackers[i]->get_process();
        os << "                ";
        os << "#process " << i << " = " << p.info() << " -> " << p;
        os << ", " << std::endl;
    }
    os << "])";
    return os;
}

}

#endif
