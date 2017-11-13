/* You can compile the simulator with your custom model by specifying
 * -DMODEL_FILE="mymodel.h" as a command line parameter.
 */
#ifndef MODEL_FILE
#define MODEL_FILE "model.h"
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <cstdint>

/* cxxopts library */
#include "cxxopts.hpp"

/* JSON library */
#include "json.hpp"
using json = nlohmann::json;

/* Simulator headers */
#include "ppsim/pp.h"

using namespace pp;

/* Include the model */
#include MODEL

/* Logging macro */
auto start_time = std::chrono::system_clock::now();
#define LOG(str) do { \
    auto current = std::chrono::system_clock::now(); \
    auto elapsed = std::chrono::duration<double>(current-start_time); \
    std::cout << std::fixed << std::setprecision(10) << elapsed.count() << " -- "; \
    std::cout << str << std::endl; } while( false )

using seed_t = uint64_t;

cxxopts::Options parse_args(int argc, char *argv[]) {
    try {
        cxxopts::Options options("ppsim", "Point process simulator");
        options.positional_help("[optional args]");
        options.add_options()
          ("h,help", "Print help")
          ("t,time", "Simulation time", cxxopts::value<double>())
          ("S,step", "Simulation steps (overrides time)", cxxopts::value<int>())
          ("dt", "Step size for saving state",cxxopts::value<double>()->default_value("1.0"))
          ("U,domain", "Domain size", cxxopts::value<double>())
          ("i,input", "Input file", cxxopts::value<std::string>())
          ("m,model", "Model input file", cxxopts::value<std::string>())
          ("o,output", "Snapshot output file", cxxopts::value<std::string>())
          ("d,density", "Density file", cxxopts::value<std::string>())
          ("s,seed", "RNG seed", cxxopts::value<seed_t>())
          ("p,propensity", "Print propensity of initial configuration", cxxopts::value<bool>())
          ("positional", "Positional arguments: these are the arguments that are entered without an option", cxxopts::value<std::vector<std::string>>())
          ;

        //options.parse_positional({"input", "output", "positional"});
        options.parse(argc, argv);

        if (options.count("help")) {
          std::cout << options.help({""}) << std::endl;
          exit(0);
        }
       
        //auto v = options["positional"];
        // Specify required parameters
        //cxxopts::check_required(options, {"time"});
        
        return options;
    } catch (const cxxopts::OptionException& e) {
        std::cerr << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }
}
int read_input_points(Simulator &s, std::ifstream in) { 
    int n = 0;
    std::array<coord_t,SimulationState::DIM> coords;
    uint_t entity;
    while(in >> entity) {
        for(uint_t i = 0; i<SimulationState::DIM; i++) {
            in >> coords[i];
            if (in.eof()) {
                throw std::runtime_error("Malformed input file");
            }
        }
        Coord c(coords);
        s.add_new_point(c, entity);
        n++;
    }

    assert(s.get_state().enumerate_copy().size() == n);
    return n;
}

#include <csignal> 


volatile sig_atomic_t SIG_INT_RECEIVED = 0;
void interrupt_handler(int) {
    SIG_INT_RECEIVED += 1;
    LOG("SIGINT received. Waiting for current step to finish...");
}

bool interrupt_received(const SimulationState &) {
    return SIG_INT_RECEIVED;
}

template<typename T>
T get_parameter(std::string name, json &defaults, cxxopts::Options &options) {
    bool opt_has = options.count(name);
    bool def_has = defaults.count(name);
    if (opt_has && def_has) {
        LOG("Overriding value '" << name << "' = " << defaults[name] << " in model json file with command line argument");
    }
    // cmd line arguments override defaults
    if (options.count(name)) {
        return options[name].as<T>();
    }
    if (defaults.count(name)) {
        return defaults[name];
    }
    LOG("Could not find a value for '" << name << "' in either model parameter file or command line arguments");
    exit(1);
}

bool is_set(std::string name, json &defaults, cxxopts::Options &options) {
    bool opt_has = options.count(name);
    bool def_has = defaults.count(name);
    return opt_has || def_has;
}

std::shared_ptr<std::fstream> open_output(std::string fname) {
    auto f = std::make_shared<std::fstream>(fname, std::ios::out);
    if (f->fail()) {
        LOG("Could not open file '" << fname << "': " << strerror(errno));
        exit(1);
    }
    return f;
}

int main(int argc, char *argv[]) {
#ifdef DEBUG
    LOG("DEBUG flag set");
#endif
#ifndef NDEBUG
    LOG("Asserts enabled");
#endif
    try {
        auto options = parse_args(argc, argv);

        auto dt = options["dt"].as<double>();

        LOG("Options parsed");

        /* Read model input */
        json json_model_input;
        if (options.count("model")) {
            auto modelfname = options["model"].as<std::string>();

            std::ifstream i(modelfname);
            LOG("Reading model input data from '" << modelfname << "'");
            i >> json_model_input;
            LOG("Input read");
        } else {
            LOG("No input model given");
        }

        auto defaults = json_model_input["simulator"];

        auto time = get_parameter<double>("time", defaults, options);
        double U = get_parameter<double>("domain", defaults, options);

        /* Construct the model */
        LOG("Constructing the model");
        auto m = get_model(json_model_input);
        m.done();
        LOG(m);
        

        /* Set up the simulator */
        LOG("Creating the simulator");
        auto s = Simulator(U,m);

        /* Register SIGINT handler for convenience */
        std::signal(SIGINT, interrupt_handler); 
        s.add_halting_condition(&interrupt_received);

        if (is_set("seed", defaults, options)) {
            auto seed = get_parameter<seed_t>("seed", defaults, options);
            //auto seed = std::stoull(seed_input); // in principle, we could convert input seed string in some other way for more entropy
            s.set_seed(seed);
            LOG("Using '" << seed << "' as seed");
        } 

        LOG("Setting up the initial state");
        setup_state(s, json_model_input);
        LOG("Initial state: " << s.get_state());

        if (options.count("input")) {
            auto infname = options["input"].as<std::string>();
            LOG("Reading input configuration from '" << infname << "'");
            auto c = read_input_points(s, std::ifstream(infname));
            LOG(c << " input points read");
        } else {
            LOG("No input point configuration given");
        }


        /* Open the snapshot output file */
        if (options.count("output")) {
            auto outfname = options["output"].as<std::string>();
            LOG("Output snapshots to '" << outfname << "'");
            s.make_writer<SnapshotWriter>(open_output(outfname),dt);
        }

        /* Open the density output file */
        if (options.count("density")) {
            auto outfname = options["density"].as<std::string>();
            LOG("Output density to '" << outfname << "'");
            s.make_writer<DensityWriter>(open_output(outfname),dt);
        }

        if (options.count("propensity")) {
            LOG("Propensities:");
            double total = 0;
            for(const auto &t : s.model.get_trackers()) {
                auto prop = t->propensity();
                total += prop;
                LOG(t->get_process() << " = " << prop);
            }
            LOG("====== TOTAL: " << total);
        }

        if (options.count("step")) {
            int step_count = options["step"].as<int>();
            LOG("Executing " << step_count << " steps of simulation");
            for(int i = 0; i<step_count; i++) {
                s.step();
            }
        } else {
            LOG("Running the simulation for " << time << " time units");
            s.run(time);
            LOG("Simulation stopped at time " << s.get_state().stats.time << ". Halting reason: " << s.get_halt_reason());
        }
     } catch (const std::exception& e) {
        LOG("Exception encountered: " << e.what());
        return 1;
    }

    return 0;
}

