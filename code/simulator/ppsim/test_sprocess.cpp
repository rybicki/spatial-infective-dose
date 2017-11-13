#include <iostream>
#include <fstream>
#include "writers.h"
#include "simulator.h"
#include "process_definitions.h"

int main() {
    
    auto m = Model()
           + DensityIndependentDeath(1,1)
           + Jump<Tophat>(1, 1, 1)
           + Immigration(1, 2) 
           + ChangeInType(1,0,1)
           + ChangeInType(0,1,1)
           + Consume<Tophat>(1,0,1,1);

    auto s = Simulator<DeltaTextWriter>(100,m);
    s.set_writer(DeltaTextWriter(&std::cout, 1));
    
    s.run(100);

    s.get_writer().write_state(s.get_state());
/*
    auto ps = s.get_state().enumerate_points();
    for(auto p : ps) {
        std::cout << *p << std::endl;
    }
    */
    return 0;
}
