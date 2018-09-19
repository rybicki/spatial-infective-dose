#!/bin/bash
mkdir -p figures
cd final-experiments
./make-plots.sh
cd ..
cp ./final-experiments/figures/cropped/* figures/
cd final-sensitivity
./make-sensitivity-plots.sh
cd ..
cp final-sensitivity/*.pdf figures/
