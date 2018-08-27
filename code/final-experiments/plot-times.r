library(ggplot2)
library(grid)
library(gridExtra)
library(dplyr)

args <- commandArgs(trailingOnly = TRUE)

if (length(args) < 2) {
    print("Usage: Rscript plot-mean-and-fit.r [data] [output pdf]")
    quit()
}

in_csv <- args[1]
out_pdf <- args[2]

data <- read.csv(in_csv)
data <- subset(data, BacteriaEntryRadius == 1)

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
 p +  
      ylim(0,1) + xlim(0,5) +
      scale_color_manual(values=colors, name="Toxin scale") +
      theme_classic() +
      theme( 
            legend.position="right",
              legend.background=element_rect(size=0, fill = "transparent", colour = "transparent"),
              legend.key=element_rect(fill = "transparent", colour = "transparent"),
              legend.key.height = unit(0.6,'lines'),
              legend.key.width = unit(0.8, 'lines'),
              legend.title = element_text(size=8),
              legend.text = element_text(size=7),
              legend.margin = margin(0),
              plot.margin = margin(0),
              panel.border = element_rect(fill=NA,color="black"),
              axis.text=element_text(size=7),
              axis.title=element_text(size=8),
              axis.line = element_blank(),
              strip.background = element_blank(),
              strip.text = element_text(size=8, hjust=0),
              plot.title = element_text(size=8,hjust=0.0,face="bold")) +
      guides(color=guide_legend(override.aes=list(fill=NA),nrow=6))
}

data$tissue.fraction <- data$final.tissue/data$init.tissue
data$log.dose <- log(data$init.bacteria,10)

# --- Mean plot
d <- group_by(filter(data, ToxinDiffusionScale %in% scales), log.dose, ToxinDiffusionScale)
s <- summarise(d, tissue.avg = mean(tissue.fraction))

# ----  Time plot

time_plot <- function(data, label) {
p <- ggplot(data, mapping = aes(x = log.dose, y = time.avg, colour=factor(ToxinDiffusionScale))) 
p <- add_style(p) + 
              ylim(75,300) + 
              coord_fixed(ratio=5/(300-75)) +
              geom_line(size=1, alpha=0.8) 
}

s0 <- summarise(filter(d, final.tissue < 0.01), time.avg = mean(time))
s10 <- summarise(filter(d, !is.na(time.tissue.at.0.1)), time.avg = mean(time.tissue.at.0.1))
s25 <- summarise(filter(d, !is.na(time.tissue.at.0.25)), time.avg = mean(time.tissue.at.0.25))
s50 <- summarise(filter(d, !is.na(time.tissue.at.0.5)), time.avg = mean(time.tissue.at.0.5))
s75 <- summarise(filter(d, !is.na(time.tissue.at.0.5)), time.avg = mean(time.tissue.at.0.75))

s10$type <- 90
s50$type <- 50
s <- rbind(s10,s50)

p <- ggplot(s, mapping = aes(x = log.dose, y = time.avg, colour=factor(ToxinDiffusionScale))) +
     facet_wrap(~ type, labeller=as_labeller(function(x) { 
                                                 y <- c("A. ", "B. ")
                                                 paste(y, x, "% consumed",sep='')})) 

p <- add_style(p) + 
      ylim(75,300) + 
      coord_fixed(ratio=5/(300-75)) +
      geom_line(size=1, alpha=0.8) + 
     ylab("Time") +
     xlab(expression('Initial dose (log'[10]*')'))

ggsave(out_pdf, p, width=8, units="cm")
