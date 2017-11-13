#!/bin/sh
echo "Downloading external dependencies"
git clone https://github.com/nlohmann/json
git clone https://github.com/jarro2783/cxxopts
git clone https://github.com/imneme/pcg-cpp/
git clone https://github.com/philsquared/Catch.git
mkdir -p include
cp json/src/json.hpp include/
cp cxxopts/include/cxxopts.hpp include/
cp pcg-cpp/include/*.hpp include
