#ifndef __SIMULATOR_H_
#define __SIMULATOR_H_

#include <stdexcept>
#include <sstream>
#include <memory>
#include "model.h"
#include "tracker.h"

namespace pp {

/* If total propensity falls below this, halt.. */
static constexpr double MINIMUM_HALT_PROPENSITY = 1e-10;

class Simulator {
public:
    using HaltingConditionFunctor = std::function<bool(SimulationState const&)>;

    Simulator(double U, Model m) : done(false),
                                   model(m), 
                                   simulation_state(SimulationState(U, m.max_entity_id(), m.process_count())) { 
        model.initialise(&simulation_state);
        current_propensities.resize(m.process_count());
    }

    void add_new_point(Coord c, uint_t entity) {
        auto p = simulation_state.new_point(c, entity);
        process_added(p);
        simulation_state.add(p);
    }

    /* execute a single step of the simulation */
    inline double step() {
        DMSG("step()");
        update_propensity();

        if (is_done()) {
            return 0;
        }

        auto tau = next_time(); // FIXME: check that tau is finite
        auto rid = next_reaction(); 
        simulation_state.stats.update(tau, rid);
        run_reaction(rid);

        /* Notify writers */
        for(auto &w : writers) {
            w->process_activated(simulation_state, tau, rid);
        }

        return tau;
    }

    /* advance simulation for t time units */
    void run(double t) {
        DMSG("run(" << t << ")");
        halt_reason = "Maximum time limit reached."; // default reason

        /* Notify writers that simulation starts */
        for(auto &w : writers) {
            w->start(simulation_state);
        }

        double total_time = 0;
        while (total_time < t && !is_done()) {
            total_time += step();
        }

        /* Notify writers that simulation starts */
        for(auto &w : writers) {
            w->end(simulation_state);
        }
    }

    bool is_done() {
        for(auto i = 0u; i<halting_conditions.size(); i++) {
            auto &f = halting_conditions[i];
            if (f(simulation_state)) {
                DMSG("Halting condition " << i << " triggered.");
                done = true;
                std::stringstream s; 
                s << "Halting condition #" << i << " triggered";
                halt_reason = s.str();
                break;
            }
        }
        return done;
    }


    /* Randomly add points of given entity type with density */
    void fill(uint_t entity, double density) {
        DMSG("fill("<<entity<<", " << density << ")");
        int count = density*simulation_state.area();
        for(auto i = 0u; i<count; i++) {
            add_new_point(simulation_state.random_coord(), entity);
        }
    }

    /* Randomly add points within a circle specified by the coordinate and the kernel.
     * Note that the number of points added will be the integral of the kernel times 
     * area of the entire simulation space.
     */
    template<typename K>
    void fill_circle(uint_t entity, Coord c, K kernel) {
        DMSG("fill_circle(" << entity << ", " << join(" ", c.get_values()) << ", " << kernel << ")");
        int count = kernel.integral*simulation_state.area();
        for(auto i = 0; i<count; i++) {
            add_new_point(kernel.sample_around_w(simulation_state.rng(), c, simulation_state.U()), entity);
        }
    }

    template<typename F>
    void add_halting_condition(F f) {
        halting_conditions.push_back(f);
    }

    template<typename W, typename... Args>
    void make_writer(const Args&... args) {
        writers.push_back(std::unique_ptr<W>(new W(args...)));
    }

    const Model &get_model() const { return model; }
    const SimulationState &get_state() const { return simulation_state; }

    std::string get_halt_reason() const { return halt_reason; }

    template<typename T>
    void set_seed(T &s) {
        simulation_state.seed(s);
    }

    void update_propensity() {
        DMSG("update_propensity()");

        auto & trackers = model.get_trackers();
        for(auto i = 0; i<trackers.size(); i++) {
            auto p = trackers[i]->propensity();
            current_propensities[i] = p;
            DMSG(t->get_process() << " has propensity " << t->propensity());
        }
        
        current_propensity = sum_kh(current_propensities);
        // std::cout << std::fixed << std::setprecision(10) << "Propensities: " << join(" + ", current_propensities) << " = " << current_propensity << std::endl;

        if (current_propensity <= MINIMUM_HALT_PROPENSITY) {
            DMSG("Total propensity is 0 so halting");
            halt_reason = "Total propensity below minimum halting propensity.";
            done = true;
        }
    }

    /* Sample time until the next event */
    inline double next_time() { 
        auto rval = simulation_state.random_value();
        //auto tau = 1.0/current_propensity * log(1.0/rval);
        auto tau = -log(rval)/current_propensity;
        DMSG("next_time() = " << tau);
        return tau;
    }

    /* Sample the next reaction */
    inline uint_t next_reaction() {
        double rval = simulation_state.random_value() * current_propensity; // random value
        double mass = 0.0; // propensity mass
        double correction = 0; // correction term

        for(auto rid = 0; rid < model.get_trackers().size(); rid++) {
            auto p = current_propensities[rid];
            // A naive summation:
            //mass += p;
            
            // Kahan summation for precision..
            double y = p - correction;
            double t = mass + p;
            correction = (t - mass) - y;
            mass = t;

            if (mass >= rval) {
                DMSG("next_reaction() = " << rid << " with propensity " << p);
                //std::cout << rid << " reacts with " << p << std::endl;
                return rid;
            }
        }

        throw std::runtime_error("Simulator::next_reaction(): total propensity was less than sum of all propensities!");
    }

    /* Execute the selected reaction */
    inline void run_reaction(uint_t rid) {
        DMSG("run_reaction(" << rid << ")");
        // Clear buffers
        reactant_buffer.clear();
        product_buffer.clear();

        // Execute the process & populate buffers 
        DMSG("activating process "<< model.get_tracker(rid) << " which is " << model.get_tracker(rid)->get_process());
        model.get_tracker(rid)->activate(reactant_buffer, product_buffer);

        // Update simulation state and notify process trackers to update their state

        // 1. Remove reactants.
        DMSG("Removing " << reactant_buffer.size() << " reactants");
        for(auto p : reactant_buffer) { 
            DMSG("- Processing " << *p << " = " << p );
            for(auto rid : model.get_dependencies(p->get_entity())) {
                DMSG("- Notifying process " << rid);
                model.get_tracker(rid)->notify_removal(*p);
            }
            DMSG("- Deleting point " << p);
            simulation_state.destroy_point(p); // invalidates p!!!
        }
        // reactant_buffer is now invalid
        reactant_buffer.clear();

        // 2. Add products.
        DMSG("Adding " << product_buffer.size() << " products");
        for(auto p : product_buffer) {
            simulation_state.add(p);
            process_added(p);
        }
    }

    void process_added(Point *p) {
        // Inform all trackers of the existence of a new point p
        for(auto rid : model.get_dependencies(p->get_entity())) {
            model.get_tracker(rid)->notify_add(*p);
        }
    }

    // ---- Local variables of the object ----

    bool done; // has the simulation halted?
    std::string halt_reason; // What to output as halting reason
    std::vector<double> current_propensities; // propensity of each process
    double current_propensity; // sum of the above

    Model model; // the model specification and trackers
    SimulationState simulation_state; // current state of the simulation
    point_del_buf_t reactant_buffer; // buffer for reactants (removed points)
    point_add_buf_t product_buffer; // buffer for products (added points) 
    std::vector<HaltingConditionFunctor> halting_conditions; // list of functions checking halting condition
    std::vector<std::unique_ptr<Writer>> writers; // state writers
};

struct CheckExtinction {
    CheckExtinction(uint_t e) : entity(e) {}
    uint_t entity;

    bool operator()(const SimulationState &s) {
        return (s.get_count(entity) == 0);
    }
};

std::ostream &operator<< (std::ostream &os, const pp::Simulator &s) {
    return os << "Simulator(" << s.get_state() << ", " << s.get_model() << ")";
}

std::ostream &operator<< (std::ostream &os, const pp::CheckExtinction &c) {
    return os << "CheckExtinction(" << c.entity << ")";
}

} // namespace

#endif

