cd simulator
make clean
make
cd ..

./setup.sh final-sensitivity/cases-m.json final-sensitivity/output-m 21171620181 1000 final-sensitivity/tmp-m

./generate-sensitivity-cases experiments/toxin-model.json 0.1 > final-sensitivity/cases-0.1.json
./generate-sensitivity-cases experiments/toxin-model.json 0.25 > final-sensitivity/cases-0.25.json
./generate-sensitivity-cases experiments/toxin-model.json 0.5 > final-sensitivity/cases-0.5.json

./setup.sh final-sensitivity/cases-0.1.json final-sensitivity/output-1 117591520181 1000 final-sensitivity/tmp-1
./setup.sh final-sensitivity/cases-0.25.json final-sensitivity/output-2 217591520182 1000 final-sensitivity/tmp-2
./setup.sh final-sensitivity/cases-0.5.json final-sensitivity/output-3 317591520183 1000 final-sensitivity/tmp-3

ls -1 final-sensitivity/tmp-*/batch.sbatch
