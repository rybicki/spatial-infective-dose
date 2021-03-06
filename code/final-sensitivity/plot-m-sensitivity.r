library(ggplot2)
library(gridExtra)
library(grid)
library(plyr)
library(dplyr)

args <- commandArgs(trailingOnly = TRUE)

baseline <- read.csv("../final-experiments/data.csv.gz") %>%
            subset(BacteriaEntryRadius == 1)

sensfiles <- c("sensitivity-1.csv.gz",
               "sensitivity-2.csv.gz",
               "sensitivity-3.csv.gz",
               "sensitivity-m.csv.gz")

all_sensdata <- read.csv(sensfiles[1])
all_sensdata$dataset <- 1
all_sensdata$BacteriaJumpScale.1 <- NULL
all_sensdata$ToxinDeathRate.1 <- NULL

for (i in 2:length(sensfiles)) {
    ns <- read.csv(sensfiles[i])
    ns$dataset <- i
    ns$BacteriaJumpScale.1 <- NULL
    ns$ToxinDeathRate.1 <- NULL
    if (i == 5) {
        ns <- filter(ns, ToxinDeathRate == 0)
    }
    all_sensdata <- rbind(all_sensdata, ns)
}

all_sensdata$tissue.fraction <- all_sensdata$final.tissue/all_sensdata$init.tissue
all_sensdata$log.dose <- log(all_sensdata$init.bacteria,10)

baseline$tissue.fraction <- baseline$final.tissue/baseline$init.tissue
baseline$log.dose <- log(baseline$init.bacteria,10)

colors = c('firebrick','blue4')

add_style <- function(p) {
 p +  xlab(expression('Initial dose (log'[10]*')')) + 
      ylim(0,1) + xlim(0,4.5) +
      scale_color_manual(values=colors, name="Toxin scale") +
      scale_fill_manual(values=colors, name="Toxin scale") +
      theme_classic() +
      theme(legend.position='bottom',#legend.position=c(0.4,0.9), #legend.direction="horizontal", legend.box = "vertical",
              legend.background=element_rect(fill = "transparent", colour = "transparent"),
              legend.key=element_rect(fill = "transparent", colour = "transparent"),
              legend.key.height = unit(0,'lines'),
              plot.title = element_text(size=12,hjust=0.5,face="bold"),
              axis.title=element_blank(),
              axis.text=element_text(size=12)) +
      guides(color=guide_legend(override.aes=list(fill=NA))) 

}
params <- c("ToxinDeathRate")
varnames <- c(expression(paste('Toxin removal rate, ',italic('m'))))

filter_data <- function(d, pname) {
    group_by_(d, "log.dose", "ToxinDiffusionScale", "dvar"=pname) %>%
    summarise(tissue.avg = mean(tissue.fraction))
}

get_label_locs <- function(r, delta) {
    # Find locations for annotation labels
    lowys <- 1-filter(r, log.dose==0)$lower
    upys <- 1-filter(r, log.dose==0)$upper
    if (any(upys < lowys)) {
        delta <- -delta
    }
    cap <- function(x){
                        if(x < 0) {
                            0
                        } else if (x > 1) {
                            1
                        } else {
                            x
                        }
                    } 
    low_capped <- mapply(cap, lowys - delta) # don't let small values go outside plotting area
    up_capped <- mapply(cap, upys+delta)

    list("low"=c(low_capped), "up"=c(up_capped))
}


make_rdf <- function(sensdata, pname) {
    sd <- subset(sensdata, complete.cases(sensdata[pname]))
    mins = sd[sd[pname] == min(sd[pname]),]
    maxs = sd[sd[pname] == max(sd[pname]),] 
    smin <- filter_data(mins, pname)
    smax <- filter_data(maxs, pname)

    r <- data.frame(upper=smax$tissue.avg, 
                    lower=smin$tissue.avg, 
                    log.dose=smin$log.dose, 
                    tissue.avg=smin$tissue.avg, 
                    tox.scale=smin$ToxinDiffusionScale, 
                    neglabels = factor(smin$ToxinDiffusionScale, labels='-'), 
                    poslabels = factor(smax$ToxinDiffusionScale, labels='+'))
}

do_mean_plots <- function(scales, colors) {

    mean_plots <- list()
    for (k in 1:length(scales)) {
        scale <- scales[k]
        color <- colors[k]
        s <- filter(baseline, ToxinDiffusionScale == scale) %>%
             filter(log.dose <= 4.5) %>%
             group_by(log.dose) %>%
             summarise(tissue.avg = mean(tissue.fraction))

        mean_plot <- ggplot() + 
                     ylab('Fraction of tissue consumed') +
                     geom_line(data=s, aes(x=log.dose, y=1-tissue.avg), color=color, size=1)

        styles <- c("solid", "dashed", "dotted", "longdash", "dotdash")
        for (i in 1:length(sensfiles)) {
            r <- make_rdf(filter(all_sensdata, dataset == i) %>% filter(ToxinDiffusionScale == scale) %>% filter(ToxinDeathRate != 0.1), "ToxinDeathRate")
            l <- get_label_locs(r, 0.05)
            low = l$low
            up = l$up

            mean_plot <- mean_plot +
                         geom_line(data=r, aes(x=log.dose, y=1-lower), size=0.25, color=color, linetype=styles[i]) + 
                         geom_line(data=r, aes(x=log.dose, y=1-upper), size=0.25, color=color, linetype=styles[i])

            if (i < length(sensfiles)) {
                mean_plot <- mean_plot + geom_ribbon(data=r, aes(x=log.dose, ymin = 1-lower, ymax = 1-upper), fill=color, alpha=0.1)
            }

            if (i == length(sensfiles)-1) {
                mean_plot <- mean_plot +
                             annotate("text", x = 0, y = low, label = "-", color=color) +
                             annotate("text", x = 0, y = up, label = "+",  color=color) 
            }
        }


        titlestr <- expression(paste('Toxin removal rate, ',italic('m'), ' (Local)'))
        if (scale > 1) {
            titlestr <- expression(paste('Toxin removal rate, ',italic('m'), ' (Distant)'))
        }
        mean_plot <- add_style(mean_plot) + labs(x='',y='', title=titlestr) 
        mean_plots <- c(mean_plots, list(mean_plot))
    }
    mean_plots
}

pdf(file="sensitivity-m.pdf",width=6,height=3)
do.call(grid.arrange, c(do_mean_plots(c(1,32), c('firebrick','blue4')), nrow = 1, ncol=2, left='Fraction of tissue consumed', bottom='Log initial dose'))
dev.off()


warnings()




