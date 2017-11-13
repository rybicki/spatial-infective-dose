#ifndef __SPROCESS_H_
#define __SPROCESS_H_

#include "point.h"
#include "simulation_state.h"
#include "kernel.h"
#include "configuration.h"

#include <array>
#include <vector>
#include <stdexcept>
#include <string>

namespace pp {

/* Process interface for Model/Simulator to figure out dependencies */
struct IProcess {
    /* Get number if input elements */
    virtual uint_t get_input_count() const = 0;
    /* Get number of output elements */
    virtual uint_t get_output_count() const = 0;
    /* Get the ith input element */
    virtual uint_t input(uint_t i) const = 0;
    /* Get the ith output element */
    virtual uint_t output(uint_t i) const = 0;
    /* Return the input radius of the process; i.e. how far can points influence the propensity of this process */
    virtual double get_input_radius() const = 0;

    virtual std::string info() const { return ""; }

    /* Some helpers */
    std::vector<uint_t> get_input_list() const {
        auto v = std::vector<uint_t>();
        for(auto i = 0u; i<get_input_count(); i++) {
            v.push_back(input(i));
        }
        return v;
    }

    std::vector<uint_t> get_output_list() const {
        auto v = std::vector<uint_t>();
        for(auto i = 0u; i<get_output_count(); i++) {
            v.push_back(output(i));
        }
        return v;
    }
};

std::ostream &operator<< (std::ostream &os, const IProcess &p) {
    os << typeid(p).name() << "(inputs=[";
    auto ins = std::vector<uint_t>();
    os << join(", ", p.get_input_list());
    os << "], outputs=[";
    os << join(", ", p.get_output_list());
    return os << "], input_radius=" << p.get_input_radius() << ")";
}

template<int IN, int OUT>
struct Process : public IProcess {
public:
    using Configuration = NConfiguration<IN>;
    static constexpr uint_t input_count = IN;
    static constexpr uint_t output_count = OUT;

    Process(std::array<uint_t,IN> in, std::array<uint_t,OUT> out, double inr) : inputs(in), outputs(out), in_radius(inr) {}

    inline uint_t get_input_count() const { return IN; }
    inline uint_t get_output_count() const { return OUT; }

    inline uint_t input(uint_t i) const { return inputs[i]; }
    inline uint_t output(uint_t i) const { return outputs[i]; }

    inline double get_input_radius() const { return in_radius; }
protected:
    std::array<uint_t,IN> inputs; // input entity IDs
    std::array<uint_t,OUT> outputs; // output entity IDs
    double in_radius; 
};

} // namespace

#endif
