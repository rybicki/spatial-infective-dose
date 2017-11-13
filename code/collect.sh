#!/bin/sh
./gather experiments/output-1/ experiments/data-1.csv
./gather experiments/output-2/ experiments/data-2.csv
./gather experiments/output-3/ experiments/data-3.csv
./gather experiments/output-4/ experiments/data-4.csv
./gather experiments/output-5/ experiments/data-5.csv
./gather experiments/output-6/ experiments/data-6.csv
python join.py experiments/data-1.csv experiments/data-2.csv experiments/data-3.csv experiments/data-4.csv experiments/data-5.csv experiments/data-6.csv > experiments/data.csv
gzip experiments/data.csv
