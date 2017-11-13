input=$1 #experiments/cases.json
outdir=$2 #experiments/output-1/
seed=$3 #26072017
replicates=$4 #500
tmpdir=$5
curdir=$PWD
mkdir -p $outdir
./generate-cases --model experiments/toxin-model.json --cases $input --seed $seed --replicates $replicates --output $outdir --batch $outdir/batch.sh
./batch-tools/make-slurm-script $curdir/$outdir/batch.sh 1000 $curdir/$tmpdir
./batch-tools/chunkify $curdir/$outdir/batch.sh $curdir/$tmpdir//chunk 1000
