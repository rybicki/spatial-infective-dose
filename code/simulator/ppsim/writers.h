#ifndef __WRITER_H_
#define __WRITER_H_

#include <iostream>
#include "common.h"
#include "point.h"
#include "simulation_state.h"

namespace pp {

/*
 * Writer interface.
 */
struct Writer {
    virtual void process_activated(SimulationState &s, double tau, uint_t process_id) = 0;
    virtual void start(SimulationState &s) = 0;
    virtual void end(SimulationState &s) = 0;
};

struct SnapshotWriter : public Writer {

    SnapshotWriter(std::shared_ptr<std::ostream> o, double d) : out(o), delta(d), current(0) {}

    void process_activated(SimulationState &s, double tau, uint_t process_id) {
        current += tau;
        if (current >= delta) {
            write_state(s);
            current = 0;
        }
    }

    void write_state(SimulationState &s) {
        *out << s.stats.time << " " << s.stats.total_events << " ";
        for(auto p : s.enumerate()) {
            *out << p->get_entity() << " " << join(" ", p->get_coord().get_values()) << " ";
        }
        *out << std::endl;

    }

    void start(SimulationState &s) {
        write_state(s);
    }

    void end(SimulationState &s) {
        write_state(s);
    }

    std::shared_ptr<std::ostream> out;
    double delta;
    double current;
};

struct DensityWriter : public Writer {
    DensityWriter(std::shared_ptr<std::ostream> o, double d) : out(o), delta(d), current(0) {}
    
    void process_activated(SimulationState &s, double tau, uint_t process_id) {
        current += tau;
        if (current >= delta) {
            write_state(s);
            current=0;
        }
    }

    void write_state(SimulationState &s) {
        *out << s.stats.time << "\t" << s.stats.total_events;
        for(auto i = 0; i<=s.get_max_entities(); i++) {
            *out << "\t" << s.get_count(i);
        }
        *out << std::endl;
    }

    void start(SimulationState &s) {
        /* print header */
        *out << "time\tevents";
        for(auto i = 0; i<=s.get_max_entities(); i++) {
            *out << "\t" << i;
        }
        *out << std::endl;
        write_state(s);
    }

    void end(SimulationState &s) {
        write_state(s);
    }

    std::shared_ptr<std::ostream> out;
    double delta;
    double current;
};

} // namespace

#endif
