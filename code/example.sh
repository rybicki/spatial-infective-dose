#!/bin/bash
echo "Compile the simulator source code"
cd simulator
make
cd ..

echo "Run a short simulation"
./simulator/toxin -m experiments/toxin-model.json --time 30 -o example-snapshot.txt -U 50 -d example-density.txt --dt 0.5 --seed 123456789

echo "Animate the simulation"
python animate.py -i example-snapshot.txt -o example.gif --tmpdir tmp -U 50 --style animations/input/style.json --nolabel=1 --dpi=300 --noaxis=1 --size 3 3 --embedlabel 1
