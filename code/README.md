# Supplementary material for "A model of bacterial toxin-dependent pathogenesis explains infective dose"

This package contains the source code for simulator used in the manuscript:

    A model of bacterial toxin-dependent pathogenesis explains infective dose
    Joel Rybicki, Eva Kisdi and Jani V. Anttila
    Date: August 2018

Below are the instruction on how to run the simulations and replicate the analyses.

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

# Generating the figures

To generate the figures appearing in the paper, just run 

    ./make-figures.sh

The datasets used for the final figures in the main text and SI text is located in the files `final-experiments/pooled-data.csv.gz` and `final-sensitivity/sensitivity-*.csv.gz`.

Data has been generated from the parameter specifications given in the JSON files `final-experiments/*.json` (see below on examples how to regenerate the data).

# Generating additional illustration and animations

To produce the example animations, first check that the simulator compiles by running

    make

and then run the following:

    cd animations
    ./make-animations.sh

This may take some time. If you have gifsicle installed, then you can also compress the animations by running

    ./compress-animations.sh

# Minimal example for running the simulations

If you want to play with the simulator, then take a look at the file `example.sh` for a minimal example. Running this generates the following files:

* snapshot data of all point locations (`example-snapshot.txt`)
* density data (`example-density.txt`)
* animate the snapshot data (`example.gif`)

If you look into the file `example.sh`, you can see that the simulator is run with the command

    ./simulator/toxin -m experiments/toxin-model.json --time 30 -o example-snapshot.txt -U 50 -d example-density.txt --dt 0.5 --seed 123456789

The command line parameters are

* `-m experiments/toxin-model.json` specifies the model parameter file
* `-o example-snapshot.txt` determines the output file for the snapshot data
* `-d example-density.txt` determines the density data (the column names are given in `experiments/toxin-model.json`)
* `--time 30` determines the simulation time
* `--U 50` determines the size of the simulation domain
* `--dt 0.5` gives the frequency of the snapshots (every 0.5 time units in this case)
* `--seed 123456789` sets the seed for the random number generator

# Version 1 (November 2017)

## The simulation data

The simulation data is available in the file `experiments/data.csv.gz`. This is a (large) gzip compressed CSV file, which you can open in e.g. R by using 

    data <- read.table("experiments/data.csv.gz", sep=",", header=TRUE)

## Reproducing the results

The source code for generating the simulation data is provided here. However, running the simulations takes considerable amount of time and takes some space as it produces a lot of data files.

* If you want to run all of the simulations again, then follow the instructions in the next subsection.
* If you want to just generate the plots and animations, then you can skip the next subsection.

### Running the simulations 

Run the initialisation script

    ./prepare.sh

This will compile the simulator and generate required batch files. There are two files:

* `experiments/batch-all-cases.sh` that lists all the individual commands for running simulations. Note that running these may take several months of CPU time on a single machine.
* `experiments/all-sbatches.sh` that lists all SLURM array batch jobs; these can be executed on a computing cluster that has the [SLURM workload manager](https://slurm.schedmd.com/) installed.

Once all the commands in the batch file have been executed successfully, it remains to collect all the data in a single summary file:

    ./collect.sh

The end result is the file `experiments/data.csv.gz`. Note that the final results dataset `final-experiments/pooled-data.csv.gz` contain some more replicates than what is contained in the `experiments/data.csv.gz`. 

Similarly, you can run the sensitivity analysis by executing `prepare-sensitivity.sh`, the resulting batch files, and then finally `collect-sensitivity.sh`. 


