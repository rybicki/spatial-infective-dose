#!/bin/sh
TIME=500
mkdir -p results
for i in ./input/parameters*.json; do
    name=$(basename $i | sed -e 's/parameters-\(.*\).json/\1/')
    dir="./output/$name"
    original="$dir/$name.gif"
    new="results/c-$name.gif"
    gifsicle -i $original -o $new -O2 --resize-width 400 --resize-method catrom --colors 32
done

