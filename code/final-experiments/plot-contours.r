library(viridisLite)
library(reshape2)

## Read command line arguments
args <- commandArgs(trailingOnly = TRUE)

if (length(args) < 2) {
    print("Usage: Rscript plot-contours.r [inoculation radius] [data] [output] ")
    quit()
}

radius <- args[1]
in_csv <- args[2]
out_fname <- args[3]

out_pdf <- paste(out_fname, ".pdf", sep="")
out_tiff <- paste(out_fname, ".tiff", sep="")

colors = c('firebrick',
           'orange',
           'olivedrab',
           'mediumpurple',
           'dodgerblue', 
           'blue4',
           'darkgray',
           'black')

threshold <- 0.75

## Hill dose-response function
hillfun <- function(x,base,maxv,half,steep){
    res <- base + maxv*(x/half)^steep/(1 + (x/half)^steep)
    return(res)
}

## read in file and add very important columns
d <- read.csv(in_csv)
d <- subset(d,BacteriaEntryRadius==radius)
d$prop.tissue <- 1.0 - d$final.tissue/d$init.tissue
d$loginitb <- log10(d$init.bacteria)

## leave only the necessary columns
d <- data.frame(ToxinDiffusionScale = d$ToxinDiffusionScale,
                LogInitBacteria = d$loginitb,
                PropTissueLeft = d$prop.tissue)

## get a list of scales
scales <- unique(d$ToxinDiffusionScale)
scales <- sort(scales)
labels <- c("A", "B", "C", "D", "E", "F")
initb <- unique(d$LogInitBacteria)
initb <- sort(initb)

## we have 21 initial doses (columns)
## try 32 for rows
rows <- 32

## get normalised kernel density estimates
kdes <- list()
meanvalues <- list()
for(i in 1:length(scales)){
    cd <- subset(d,d$ToxinDiffusionScale==scales[i])
    m <- matrix(0.0,nrow=rows,ncol=length(initb))
    for(dose in 1:length(initb)){
        scd <- subset(cd,cd$LogInitBacteria==initb[dose])
        kde <- density(scd$PropTissueLeft,
                       bw=0.25,
                       adjust=1,
                       kernel='gaussian',
                       from=0.0,
                       to=1.0,
                       n=rows)
        m[,dose] <- kde$y
        m[,dose] <- m[,dose]/sum(m[,dose]) ## normalisation!
    }
    kdes[[i]] <- m
    mv <- numeric(length(initb))
    for(j in 1:length(initb)){
        mv[j] <- mean(cd$PropTissueLeft[cd$LogInitBacteria == initb[j]])
    }
    meanvalues[[i]] <- data.frame(LogInitBacteria=initb,meanvalue=mv)
}

## make a long (melted) list of kernel densities
## NOTE: this exists for ggplot2 (I think) and is not necessary for the baseplot
kdem <- list()
yvals <- seq(from=0.0,to=1.0,length=rows)
for(i in 1:length(scales)){
    kdem[[i]] <- melt(kdes[[i]])
    colnames(kdem[[i]]) <- c('y','x','z')
    for(j in 1:nrow(kdem[[i]])){
        kdem[[i]][j,'y'] <- yvals[kdem[[i]][j,'y']]
        kdem[[i]][j,'x'] <- initb[kdem[[i]][j,'x']]
    }
}

## initial guesses for the hill fitting to means
startlists <- list()
startlists[[1]] <- list(base = 0.72,
                        maxv = 0.26,
                        half = 3.3,
                        steep = 3.1)
startlists[[2]] <- list(base = 0.68,
                        maxv = 0.28,
                        half = 2.7,
                        steep = 4.5)
startlists[[3]] <- list(base = 0.58,
                        maxv = 0.38,
                        half = 2.8,
                        steep = 5.8)
startlists[[4]] <- list(base = 0.48,
                        maxv = 0.50,
                        half = 3.0,
                        steep = 7.5)
startlists[[5]] <- list(base = 0.35,
                        maxv = 0.63,
                        half = 3.2,
                        steep = 9.6)
startlists[[6]] <- list(base = 0.23,
                        maxv = 0.76,
                        half = 3.6,
                        steep = 14.5)

## PDF

## oversized grid for panels, remember to pdfcrop afterwards
lmat <- matrix(c(7,7,7,1,4,7,2,5,7,3,6,7),nr=3,nc=4)
korkeus <- 5
pdf(file=out_pdf,width=1.25*korkeus,height=korkeus)
colpad <- 5
nlev <- 10
layout(lmat,widths=c(0.5,1,1,1),heights=c(1,1,0.5))
par(mar=c(0.2,0.2,2.0,0.2),mgp=c(0.8,0.8,0.0),xpd='NA')
for(k in 1:6){
    hf <- nls(meanvalue ~ base + maxv*(initb/half)^steep/(1 + (initb/half)^steep),
              algorithm="port",
              data=meanvalues[[k]],
              start=startlists[[k]],
              lower=c(0.0,0.05,0.1,0.5),
              upper=c(1.0,1.0,20.0,20.0))
    maxlev <- 0.0
    minlev <- 1.0
    for(i in 1:6){
        if(max(kdes[[k]]) > maxlev){
            maxlev <- max(kdes[[k]])
        }
        if(min(kdes[[k]]) < minlev){
            minlev <- min(kdes[[k]])
        }
    }
    levs <- seq(from=minlev,to=maxlev,length.out=nlev)
    plot(0,0,type='n',xlim=range(initb),ylim=range(yvals),
         main='',
         xaxt='n',yaxt='n',xlab='',ylab='')
    title(main=bquote(paste('Movement scale: ', .(scales[k]))))
    .filled.contour(x=initb,y=yvals,z=t(kdes[[k]]),
                    levels=levs,
                    col=rev(magma(nlev+1+colpad)[colpad:(nlev+1+colpad)]))
    if((k == 1) | (k == 4)){
        axis(2)
    }
    if((k == 4) | (k == 5) | (k == 6)){
        axis(1)
    }
    contour(x=initb,y=yvals,z=t(kdes[[k]]),levels=levs,add=TRUE,lwd=0.5,
            drawlabels=FALSE,
            plot.axes={axis(1);axis(2);points(3.5,0.9,pch=16,col='white');})

    curve(hillfun(x,coef(hf)['base'],coef(hf)['maxv'],
                  coef(hf)['half'],coef(hf)['steep']),add=TRUE,lwd=2.5)


    #plot(meanvalues[k]$initb, meanvalues[k]$meanvalue)

    if(k == 4){
        text(-1.16,1.145,'Fraction of tissue consumed',srt=90)
        text(8.1,-0.27,expression('log'[10]*'(dose)'))
    }
}
dev.off()

##TIFFI

lmat <- matrix(c(7,7,7,1,4,7,2,5,7,3,6,7),nr=3,nc=4)
dpi = 300
w = 11.4
tiff(file=out_tiff,width=w,height=w*0.75,units='cm',pointsize=8,res=dpi,compression='lzw')
colpad <- 5
nlev <- 10
layout(lmat,widths=c(0.5,1,1,1),heights=c(1,1,0.5))
par(mar=c(0.2,0.2,2.0,0.2),mgp=c(0.8,0.8,0.0),xpd='NA')
for(k in 1:6){
    hf <- nls(meanvalue ~ base + maxv*(initb/half)^steep/(1 + (initb/half)^steep),
              algorithm="port",
              data=meanvalues[[k]],
              start=startlists[[k]],
              lower=c(0.0,0.05,0.1,0.5),
              upper=c(1.0,1.0,20.0,20.0))
    maxlev <- 0.0
    minlev <- 1.0
    for(i in 1:6){
        if(max(kdes[[k]]) > maxlev){
            maxlev <- max(kdes[[k]])
        }
        if(min(kdes[[k]]) < minlev){
            minlev <- min(kdes[[k]])
        }
    }
    levs <- seq(from=minlev,to=maxlev,length.out=nlev)
    plot(0,0,type='n',xlim=range(initb),ylim=range(yvals),
         main='',
         xaxt='n',yaxt='n',xlab='',ylab='')
    title(main=bquote(paste(.(labels[k]), '. Toxin scale ', .(scales[k]))),cex.main=2,adj=0)
    .filled.contour(x=initb,y=yvals,z=t(kdes[[k]]),
                    levels=levs,
                    col=rev(gray.colors(nlev,start=0.3,end=1.0)))
    if((k == 1) | (k == 4)){
        axis(2)
    }
    if((k == 4) | (k == 5) | (k == 6)){
        axis(1)
    }
    contour(x=initb,y=yvals,z=t(kdes[[k]]),levels=levs,add=TRUE,lwd=0.5,
            drawlabels=FALSE,
            plot.axes={axis(1);axis(2);points(3.5,0.9,pch=16,col='white');})
    curve(hillfun(x,coef(hf)['base'],coef(hf)['maxv'],
                  coef(hf)['half'],coef(hf)['steep']),add=TRUE,lwd=3,col='black') 
    curve(hillfun(x,coef(hf)['base'],coef(hf)['maxv'],
                  coef(hf)['half'],coef(hf)['steep']),add=TRUE,lwd=2,col=colors[k])

    z = (threshold-coef(hf)['base'])/coef(hf)['maxv']
    x = (z/(1-z))^(1/coef(hf)['steep'])*coef(hf)['half']
    segments(x,0,x,1,col='black',lwd=1, lty=2)

    if(k == 4){
        text(-1.16,1.145,'Fraction of tissue consumed',srt=90, cex=1)
        text(8.1,-0.27,expression('Initial dose (log'[10]*')'), cex=1)
    }
}
dev.off()
