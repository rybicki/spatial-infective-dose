#
# Plot aggregation experiment results
# 
library(ggplot2)
library(gridExtra)

args <- commandArgs(trailingOnly = TRUE)

if (length(args) < 2) {
        print("Usage: Rscript plot-aggregate.r [data] [output]")
    quit()
}

in_csv <- args[1] # "experiments/data.csv.gz" 
out_pdf <- args[2] # "figures/fig_aggregation.pdf"

d <- read.csv(in_csv)

add_style <- function(p) {
 p +  xlab(expression('Initial dose (log'[10]*')')) + 
      ylim(0,1) + xlim(0,5) +
      scale_color_manual(values=colors, name="Toxin scale") +
      theme_classic() +
      theme(legend.position=c(0.5,0.95), legend.direction="horizontal",
              legend.background=element_rect(fill = "transparent", colour = "transparent"),
              legend.key=element_rect(fill = "transparent", colour = "transparent"),
              legend.key.height = unit(1,'lines'),
              axis.text=element_text(size=10),
              axis.title=element_text(size=12),
              plot.title = element_text(size=12,hjust=0.0,face="bold")) +
      guides(color=guide_legend(override.aes=list(fill=NA)))
}

d$tissue.fraction <- d$final.tissue/d$init.tissue
d$log.dose <- log(d$init.bacteria,10)
d$radius <- d$BacteriaEntryRadius

summary(d)

colors = c('firebrick',
           'darkgoldenrod',
           'olivedrab',
           'mediumpurple',
           'dodgerblue', 
           'blue4',
           'darkgray',
           'black')


classify_infection <- function(x) {
  as.integer(x >= 0.1) 
}

add_style <- function(p) {
 p +  xlab(expression('Initial dose (log'[10]*')')) + 
      ylim(0,1) + xlim(0,5) +
      scale_color_manual(values=colors, name="Toxin scale") +
      theme_classic() +
      theme(
              legend.background=element_rect(fill = "transparent", colour = "transparent"),
              legend.key=element_rect(fill = "transparent", colour = "transparent"),
              legend.key.height = unit(1,'lines'),
              axis.text=element_text(size=10),
              axis.title=element_text(size=12),
              plot.title = element_text(size=12,hjust=0.0,face="bold")) +
      guides(color=guide_legend(override.aes=list(fill=NA)))
}


my_plot <- function(data, label, key) {

p <- ggplot(data, mapping = aes(x = log.dose, y = classify_infection(tissue.fraction), colour=factor(ToxinDiffusionScale))) +
    stat_smooth(method=glm,method.args=list(family='binomial'),se=FALSE) +
    ylab('Probability of clearance') 
add_style(p)
}

pdf(file=out_pdf,width=7,height=2)
p1 <- my_plot(d, "Movement scale", d$ToxinDiffusionScale) + facet_wrap( ~ radius, ncol=4, labeller=label_both) + theme(legend.position="right")

print(p1)
dev.off()


