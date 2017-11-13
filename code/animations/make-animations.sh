#!/bin/sh
TIME=500
cd ../simulator
make 
cd ../animations
for i in ./input/parameters*.json; do
    name=$(basename $i | sed -e 's/parameters-\(.*\).json/\1/')
    dir="./output/$name"
    mkdir -p "$dir"
    snapshot="$dir/snapshot.txt"
    ../simulator/toxin -m $i --dt 0.5 --time $TIME -o $snapshot 
    python ../animate.py -i $snapshot -o "$dir/$name.gif" --tmpdir "$dir/tmp" --donotremove 1 -U 100 --style input/style.json --nolabel=1 --dpi=300 --noaxis=1 --size 3 3 --embedlabel 1
done

mkdir -p results
cp output/*/*.gif results/
./compress-animations.sh

