library(ggplot2)
library(grid)
library(gridExtra)
library(dplyr)

args <- commandArgs(trailingOnly = TRUE)

if (length(args) < 2) {
    print("Usage: Rscript plot-mean-and-fit.r [data] [output 1] ")
    quit()
}

in_csv <- args[1]
out_pdf <- args[2]

data <- read.csv(in_csv)
#data <- subset(data, BacteriaEntryRadius == 1)

scales <- unique(data$ToxinDiffusionScale)

colors = c('firebrick',
           'orange',
           'olivedrab',
           'mediumpurple',
           'dodgerblue', 
           'blue4',
           'darkgray',
           'black')

add_style <- function(p) {
 p +  xlab(expression('Initial dose (log'[10]*')')) + 
      ylab("Tissue consumed") + 
      ylim(0,1) + xlim(0,5) +
      scale_color_manual(values=colors, name="Toxin scale") +
      theme_classic() +
      theme(#legend.position=c(0.95,0.4), 
            #legend.direction="vertical",
            legend.position="right",
              legend.background=element_rect(fill = "transparent", colour = "transparent",size=0),
              legend.margin = margin(0),
              legend.key=element_rect(fill = "transparent", colour = "transparent"),
              legend.key.height = unit(0.6,'lines'),
              legend.key.width = unit(0.8, 'lines'),
              legend.title = element_text(size=8),
              legend.text = element_text(size=7),
              plot.margin = margin(0),
              panel.border = element_rect(fill=NA,color="black"),
              axis.text=element_text(size=7),
              axis.title=element_text(size=8),
              axis.line = element_blank(),
              strip.background = element_blank(),
              strip.text = element_text(size=8, hjust=0),
              plot.title = element_text(size=8,hjust=0.0,face="bold")) +
      guides(color=guide_legend(override.aes=list(fill=NA),ncol=1))
}

data$tissue.fraction <- data$final.tissue/data$init.tissue
data$log.dose <- log(data$init.bacteria,10)

# --- Mean plot

d <- group_by(filter(data, ToxinDiffusionScale %in% scales), log.dose, ToxinDiffusionScale, BacteriaEntryRadius)
s <- summarise(d, tissue.avg = mean(tissue.fraction), entryradius=mean(BacteriaEntryRadius))

make_label <- function(value) {
  x <- as.character(value)
  bquote(italic(.(x))~subjects)
}

plot_labeller <- function(variable, value) {
  do.call(expression, lapply(levels(value), make_label))
}


warnings()
p <- ggplot(s, aes(x=log.dose,y=1-tissue.avg,colour=as.factor(ToxinDiffusionScale))) +
     geom_line(size=1.0) +
     facet_wrap(~entryradius, ncol=4, 
                labeller=as_labeller(function(x) { 
                                         y <- c("A.","B.","C.","D.");
                                         paste(y, "Radius", x)
                                     })) +
     coord_fixed(ratio=5) 
p <- add_style(p) 

w = 11.4
ggsave(out_pdf, p, width=w, units="cm", device="pdf")

