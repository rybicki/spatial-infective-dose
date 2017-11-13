.PHONY: all
all:
	mkdir -p ../results/
	Rscript plot-mean-and-fit.r data.csv.gz ../results/fig_main.pdf ../results/fig_times.pdf ../results/fit-stats.csv
	Rscript plot-aggregate.r data.csv.gz ../results/fig_aggregation.pdf 
