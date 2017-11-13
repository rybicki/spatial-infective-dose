cd simulator
make clean
make
cd ..
./setup.sh experiments/cases-1.json experiments/output-1/ 316375341 500 experiments/tmp-1/
./setup.sh experiments/cases-2.json experiments/output-2/ 997841921 1500 experiments/tmp-2/
./setup.sh experiments/cases-3.json experiments/output-3/ 803092937 50 experiments/tmp-3/
./setup.sh experiments/cases-4.json experiments/output-4/ 4581236073 150 experiments/tmp-4/
./setup.sh experiments/cases-3.json experiments/output-5/ 1856678136 200 experiments/tmp-5/
./setup.sh experiments/cases-4.json experiments/output-6/ 3712516286 600 experiments/tmp-6/

ALL_CASES=experiments/batch-all-cases.sh
SBATCHES=experiments/all-sbatches.sh

rm -f $ALL_CASES $SBATCHES

cat experiments/output-*/batch.sh > $ALL_CASES

for f in experiments/tmp-*/batch.sbatch; do 
    d=$(dirname $f)
    fname="$d/batch.sbatch"
    echo "sbatch $fname" >> $SBATCHES
done

echo "All simulation cases: $ALL_CASES"
echo "All SLURM array batch jobs: $SBATCHES"

