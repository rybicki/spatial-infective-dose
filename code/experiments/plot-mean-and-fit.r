library(ggplot2)
library(gridExtra)
library(dplyr)

args <- commandArgs(trailingOnly = TRUE)

if (length(args) < 4) {
    print("Usage: Rscript plot-mean-and-fit.r [data] [output 1] [output 2] [stat csv]")
    quit()
}

in_csv <- args[1]
out_pdf <- args[2]
out2_pdf <- args[3]
stat_csv <- args[4]

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

data$tissue.fraction <- data$final.tissue/data$init.tissue
data$log.dose <- log(data$init.bacteria,10)

# --- Mean plot
d <- group_by(filter(data, ToxinDiffusionScale %in% scales), log.dose, ToxinDiffusionScale)
s <- summarise(d, tissue.avg = mean(tissue.fraction))

mean_plot <- ggplot(s, aes(x=log.dose,y=tissue.avg,colour=as.factor(ToxinDiffusionScale))) +
      ylab('Fraction of remaining tissue') + 
      #stat_smooth(method='loess') + 
      geom_smooth(method="nls", 
                  formula=y ~ SSlogis(x, Asym, xmid, scal),
                  se=FALSE) +
      geom_point(shape=1,size=0.5) 
mean_plot <- add_style(mean_plot) + ggtitle("(b)")

S = length(unique(d$ToxinDiffusionScale))
fit_df <- data.frame(toxin.diffusion.scale = rep(NA,S), 
                       asymptote = rep(NA,S), 
                       inflection.point= rep(NA,S),
                       scale = rep(NA,S),
                       residual.error = rep(NA,S))

i = 1
for(scale in sort(unique(d$ToxinDiffusionScale))) {
    dd <- subset(s, ToxinDiffusionScale == scale)
    fit <- nls(tissue.avg ~ SSlogis(log.dose, Asym, xmid, scal), data = dd)
    z <- summary(fit)
    row <- list(scale, z$coefficients[1], z$coefficients[2], z$coefficients[3], z$sigma)
    fit_df[i,] <- row
    i <- i+1
}

# ---- Regression plot

classify_infection <- function(x) {
  as.integer(x >= 0.1) 
}

regression_plot <- ggplot(data, mapping = aes(x = log.dose, y = classify_infection(tissue.fraction), colour=factor(ToxinDiffusionScale))) +
    stat_smooth(method=glm,method.args=list(family='binomial'),se=) +
    ylab('Probability of clearance') +
    ggtitle("(a)")

regression_plot <- add_style(regression_plot) + theme(legend.position='none')
warnings()
pdf(file=out_pdf,width=7,height=3.5)
grid.arrange(regression_plot,mean_plot,ncol=2,nrow=1)
dev.off()

write.csv(fit_df,stat_csv)

# ----  Time plot

time_plot <- function(data, label) {
p <- ggplot(data, mapping = aes(x = log.dose, y = time.avg, colour=factor(ToxinDiffusionScale))) 
p <- add_style(p) + 
              ylab(label) + ylim(75,300) + 
              geom_smooth(method="nls", formula=y ~ SSlogis(x, Asym, xmid, scal), se=FALSE, size=0.5) +
              #geom_smooth(method="loess", size=0.5, se=FALSE) +
              #geom_line(size=0.5) + 
              geom_point(shape=1,size=0.5) 
}

s0 <- summarise(filter(d, final.tissue < 0.01), time.avg = mean(time))
s10 <- summarise(filter(d, !is.na(time.tissue.at.0.1)), time.avg = mean(time.tissue.at.0.1))
s25 <- summarise(filter(d, !is.na(time.tissue.at.0.25)), time.avg = mean(time.tissue.at.0.25))
s50 <- summarise(filter(d, !is.na(time.tissue.at.0.5)), time.avg = mean(time.tissue.at.0.5))

summary(s0)

tp0 <- time_plot(s0, "Time until no tissue remains")
tp10 <- time_plot(s10, "Time until 10% tissue remains") + ggtitle("(b)")
tp25 <- time_plot(s25, "Time until 25% tissue remains")
tp50 <- time_plot(s50, "Time until 50% tissue remains") + ggtitle("(a)")

pdf(file=out2_pdf, width=7, height=3.5)
grid.arrange(tp50,tp10+theme(legend.position='none'),ncol=2,nrow=1)
dev.off()
