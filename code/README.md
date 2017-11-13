# Supplementary material for "A model of bacterial toxin-dependent pathogenesis explains infective dose"

This package contains the source code for simulator used in the manuscript:

    A model of bacterial toxin-dependent pathogenesis explains infective dose
    Joel Rybicki and Jani Anttila
    Date: November 2017

Below are the instruction on how to run the simulations and replicate the analysis.

## Requirements and dependencies

For running the simulations, you need to have a C++ compiler and the following additional software and packages installed:

* [R 3.3](https://www.r-project.org/)
    * ggplot2
    * dplyr
    * gridExtra
* [Python 2.7](https://www.python.org/)
    * numpy (1.12.0)
    * matplotlib (2.0.0)
* [ImageMagick](https://www.imagemagick.org/)
* [Boost C++ libraries](http://www.boost.org/)

The simulator also relies on the following libraries, which are included in `simulator/external/include`.

* [JSON for Modern C++](https://github.com/nlohmann/json)
* [CXXopts](https://github.com/jarro2783/cxxopts)
* [PCG random number generator for C++](https://github.com/imneme/pcg-cpp/)
* [Catch](https://github.com/philsquared/Catch.git)

The software has been tested on MacOS X 10.12 (Sierra) and on CentOS 6 (GNU/Linux).

## The simulation data

The simulation data is available in the file `experiments/data.csv.gz`. This is a (large) gzip compressed CSV file, which you can open in e.g. R by using 

    data <- read.table("experiments/data.csv.gz", sep=",", header=TRUE)

## Reproducing the results

The source code for generating the simulation data is provided here. However, running the simulations takes considerable amount of time and takes some space as it produces a lot of data files.

* If you want to run all of the simulations again, then follow the instructions in the next subsection.
* If you want to just generate the plots and animations, then you can skip the next subsection.

### Running the simulations and collecting data

Run the initialisation script

    ./prepare.sh

This will compile the simulator and generate required batch files. There are two files:

* `experiments/batch-all-cases.sh` that lists all the individual commands for running simulations. Note that running these may take several months of CPU time on a single machine.
* `experiments/all-sbatches.sh` that lists all SLURM array batch jobs; these can be executed on a computing cluster that has the [SLURM workload manager](https://slurm.schedmd.com/) installed.

Once all the commands in the batch file have been executed successfully, it remains to collect all the data in a single summary file:

    ./collect.sh

The end result is the file `experiments/data.csv.gz`.

## Running the plotting and analysis scripts

To produce the figures and other final data files, run the script

    ./analyse.sh

This will produce the subdirectory `results/` containing the final output files.

## Additional illustration and animations

To produce the example animations, run 

    cd animations
    ./make-animations.sh

This may take some time.
