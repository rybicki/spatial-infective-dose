library(ggplot2)
library(gridExtra)
library(grid)
library(plyr)
library(dplyr)

args <- commandArgs(trailingOnly = TRUE)

if (length(args) < 2) {
    print("Usage: Rscript plot-sensitivity.r [data] [data2] [output 1] ")
    quit()
}

in_csv <- args[1]
in2_csv <- args[2]
out_pdf <- args[3]

data <- read.csv(in_csv)
data <- subset(data, BacteriaEntryRadius == 1)
sensdata <- read.csv(in2_csv)

scales <- c(1) # unique(sensdata$ToxinDiffusionScale)
colors = c('firebrick') # c('blue4') # c('firebrick','blue4')

# Additional mortality rate tests
extra_data <- read.csv("sensitivity-3.csv")
extra_data$tissue.fraction <- extra_data$final.tissue/extra_data$init.tissue
extra_data$log.dose <- log(extra_data$init.bacteria, 10)
extra_data_summary <- filter(extra_data, ToxinDiffusionScale %in% scales) %>%
                      group_by(log.dose, ToxinDiffusionScale, ToxinDeathRate) %>%
                      summarise(tissue.avg = mean(tissue.fraction))

                  
# Additional IE parameter tests
ie_data <- read.csv("sensitivity-4.csv.gz")
ie_data$tissue.fraction <- ie_data$final.tissue/ie_data$init.tissue
ie_data$log.dose <- log(ie_data$init.bacteria, 10)

print(summary(ie_data))

add_style <- function(p) {
 p +  xlab(expression('Initial dose (log'[10]*')')) + 
      ylim(0,1) + xlim(0,5) +
      scale_color_manual(values=colors, name="Toxin scale") +
      scale_fill_manual(values=colors, name="Toxin scale") +
      theme_classic() +
      theme(legend.position='none',#legend.position=c(0.4,0.9), #legend.direction="horizontal", legend.box = "vertical",
              legend.background=element_rect(fill = "transparent", colour = "transparent"),
              legend.key=element_rect(fill = "transparent", colour = "transparent"),
              legend.key.height = unit(0,'lines'),
              plot.title = element_text(size=12,hjust=0.5,face="bold"),
              axis.title=element_blank(),
              axis.text=element_text(size=12)) +
      guides(color=guide_legend(override.aes=list(fill=NA))) 

}

data$tissue.fraction <- data$final.tissue/data$init.tissue
data$log.dose <- log(data$init.bacteria,10)
sensdata$tissue.fraction <- sensdata$final.tissue/sensdata$init.tissue
sensdata$log.dose <- log(sensdata$init.bacteria,10)

# --- Mean plot
d <- group_by(filter(data, ToxinDiffusionScale %in% scales), log.dose, ToxinDiffusionScale)
s <- summarise(d, tissue.avg = mean(tissue.fraction))

params <- c("BacteriaConsumptionRate", 
            "InhibitionRate", 
#            "ToxinDeathRate", 
            "ToxinSecretionRate", 
            "InitialTissueDensity", 
            "InitialSeekerDensity", 
            "KillerConsumptionRate", 
            "DisabledRecoveryRate", 
            "KillerToSeekerRate",
            "BacteriaConsumptionScale", 
            "InhibitionScale",
            "KillerConsumptionScale", 
            "BacteriaJumpScale", 
            "SeekerJumpScale", 
            "KillerJumpScale", 
            "KillerActivationScale",
            "KillerActivationRate")


ie_params <- c(
            "KillerConsumptionRate", 
            "DisabledRecoveryRate", 
            "KillerToSeekerRate",
            "BacteriaConsumptionScale", 
            "InhibitionScale",
            "KillerConsumptionScale", 
            "SeekerJumpScale", 
            "KillerJumpScale", 
            "KillerActivationScale")

varnames <- c(expression(paste('Tissue consumption rate, ',italic('b'))),
              expression(paste('Toxin effectivity rate, ',italic('e'))),
#              expression(paste('Toxin removal rate, ',italic('m'))),
              expression(paste('Toxin secretion rate, ',italic('s'))),
              expression(paste('Initial tissue density, ',italic(rho)['H'])),
              expression(paste('Initial seeker density, ',italic(rho)['IS'])),
              expression(paste('IE (kill) elimination rate, ',italic('k'))),
              expression(paste('IE recovery rate, ',italic('r'))),
              expression(paste('Transition from kill to seek ',italic('q'))),
              'Tissue consumption scale',
              'Toxin effectivity scale',
              'IE (kill) elimination scale',
              'Pathogen movement scale',
              'IE (seek) movement scale',
              'IE (kill) movement scale',
              'IE (seek) activation scale',
              expression(paste('IE (seek) activation rate ',italic('a'))))


filter_data <- function(d) {
    filter(d, ToxinDiffusionScale %in% scales) %>%
    group_by_("log.dose", "ToxinDiffusionScale", "dvar"=pname) %>%
    summarise(tissue.avg = mean(tissue.fraction))
}

make_rdf <- function(sensdata, pname) {
    sd <- subset(sensdata, complete.cases(sensdata[pname]))
    mins = sd[sd[pname] == min(sd[pname]),]
    maxs = sd[sd[pname] == max(sd[pname]),] 
    smin <- filter_data(mins)
    smax <- filter_data(maxs)

    r <- data.frame(upper=smax$tissue.avg, 
                    lower=smin$tissue.avg, 
                    log.dose=smin$log.dose, 
                    tissue.avg=smin$tissue.avg, 
                    tox.scale=smin$ToxinDiffusionScale, 
                    neglabels = factor(smin$ToxinDiffusionScale, labels=rep('-',length(scales))), 
                    poslabels = factor(smax$ToxinDiffusionScale, labels=rep('+',length(scales))))
}

get_label_locs <- function(r, delta) {
    # Find locations for annotation labels
    lowys <- 1-filter(r, log.dose==0)$lower
    upys <- 1-filter(r, log.dose==0)$upper
    if (any(upys < lowys)) {
        delta <- -delta
    }
    low_capped <- mapply(function(x) max(x,0), lowys - delta) # don't let small values go outside plotting area
    list("low"=c(low_capped), "up"=c(upys + delta))
}

mean_plots <- list()
for (pname in params) {
    r <- make_rdf(sensdata, pname)
    l <- get_label_locs(r, 0.05)
    low = l$low
    up = l$up

    # Plot
    mean_plot <- ggplot() + 
                 ylab('Fraction of tissue consumed') + 
                 geom_ribbon(data=r, aes(x=log.dose, ymin = 1-lower, ymax = 1-upper, fill=as.factor(tox.scale)), alpha=0.05) +
                 geom_line(data=r, aes(x=log.dose, y=1-lower, colour=as.factor(tox.scale)),size=0.5) + 
                 geom_line(data=r, aes(x=log.dose, y=1-upper, colour=as.factor(tox.scale)),size=0.5) +
                 geom_line(data=s, aes(x=log.dose, y=1-tissue.avg, colour=as.factor(ToxinDiffusionScale)), size=1) +  
                 scale_linetype_manual(values=c("dotted", "dashed", "longdash"), name="Value") +
                 annotate("text", x = rep(0,length(scales)), y = low, label = rep("-",length(scales)), colour=colors) +
                 annotate("text", x = rep(0,length(scales)), y = up, label = rep("+",length(scales)), colour=colors) 

#    if (all(is.na(ie_data[pname])) == FALSE) {
#        print(pname)
#        r2 <- make_rdf(ie_data, pname)
#        l2 <- get_label_locs(r2, 0.03)
#        mean_plot <- mean_plot + 
#                     geom_ribbon(data=r2, aes(x=log.dose, ymin = 1-lower, ymax = 1-upper, fill=as.factor(tox.scale)), alpha=0.025) +
#                     geom_line(data=r2, aes(x=log.dose, y=1-lower, colour=as.factor(tox.scale)),size=0.5,linetype="dotted") + 
#                     geom_line(data=r2, aes(x=log.dose, y=1-upper, colour=as.factor(tox.scale)),size=0.5,linetype="dotted")
#    }   

    # Handle toxin mortality tests separately
    if (pname == "ToxinDeathRate") {
        mean_plot <- mean_plot + 
                     geom_line(data=subset(extra_data_summary, ToxinDeathRate <= 0.5), 
                               aes(x=log.dose, y=1-tissue.avg, colour=as.factor(ToxinDiffusionScale), linetype=as.factor(ToxinDeathRate)))
    }

    titlestr = mapvalues(pname, params, varnames, warn_missing=FALSE)
    mean_plot <- add_style(mean_plot) + labs(x='',y='', title=titlestr) 
    mean_plots <- c(mean_plots, list(mean_plot))
}

print(length(mean_plots))
pdf(file=out_pdf,width=12,height=12)

do.call(grid.arrange, c(mean_plots, nrow = 4, ncol=4, left='Fraction of tissue consumed', bottom='Log initial dose'))
dev.off()

warnings()



