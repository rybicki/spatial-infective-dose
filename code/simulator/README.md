# The point process simulator

## Dependencies

The simulator relies on the following external libraries:

* [JSON for Modern C++](https://github.com/nlohmann/json)
* [CXXopts](https://github.com/jarro2783/cxxopts)
* [PCG random number generator for C++](https://github.com/imneme/pcg-cpp/)
* [Catch](https://github.com/philsquared/Catch.git)
* [Boost C++ libraries](http://www.boost.org/)

The first four libraries are included under `external/include`.

## Compilation

Compile the non-debug (i.e. fast) version with

    make

and the debug build can be compiled with

    make debug

The debug build will be much slower as it does various (slow) sanity checks during the simulation.

Example usage: 

    ./toxin -s 99234567 --time 100 -U 100 -d output.density -o output.points --model parameters-toxin.json  --dt 0.8
