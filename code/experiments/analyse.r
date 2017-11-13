data <- read.csv("data.csv.gz")
data <- subset(data, BacteriaEntryRadius == 1)
doses <- sort(unique(data$init.bacteria))
scales <- sort(unique(data$ToxinDiffusionScale))

for (s in scales) {
    print(sprintf("Scale: %i", s));
    for (id in doses) {
        print(sprintf("Dose: %f", id));
        d <- subset(data, init.bacteria == id & ToxinDiffusionScale == s)
        fs <- d$final.tissue / d$init.tissue
        print(sprintf("mean: %f, stdev: %f", mean(fs), sd(fs)));
    }
}
