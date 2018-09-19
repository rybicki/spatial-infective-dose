DATA="pooled-data.csv.gz"
rm -rf figures
mkdir -p figures
mkdir -p figures/cropped/

Rscript plot-times.r $DATA figures/fig-times.pdf
Rscript plot-aggregate.r $DATA figures/fig-aggregation.pdf
Rscript plot-contours.r 1 $DATA figures/fig-stochasticity-1
Rscript plot-contours.r 4 $DATA figures/fig-stochasticity-4
Rscript plot-contours.r 8 $DATA figures/fig-stochasticity-8
Rscript plot-contours.r 16 $DATA figures/fig-stochasticity-16

for f in figures/*.tiff; do 
    out="figures/cropped/$(basename $f)"
    echo "Trimming '$f' to '$out'"
    convert $f -trim $out
done

for f in figures/*.pdf; do 
    out="figures/cropped/$(basename $f)"
    echo "Trimming '$f' to '$out'"
    pdfcrop $f $out
done
