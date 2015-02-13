# batchrunner.R
# Ben Bond-Lamberty January 2015
#
# R script to batch-run Hector, for various uses:
# - run all four RCPs (or any number of cases) at once
# - do a brute-force run varying parameters
# - do a optimizer run looking for best set of parameters

# ----------------------------------------------------------------
# Settings you will definitely need to overwrite in your code

# Location of the Hector executable to run
EXECUTABLE <- "/Users/ben/Library/Developer/Xcode/DerivedData/hector-cglywsekpzlzoxaqbdkpxegrhaba/Build/Products/Debug/hector"

# ----------------------------------------------------------------
# Settings you will probably need to overwrite

# Location of the main run (working) directory
RUN_DIRECTORY <- "../../"

# Hector INI files to (possibly modify parameters in and) run
INFILES <- c("input/hector_rcp26.ini",
             "input/hector_rcp45.ini", 
             "input/hector_rcp60.ini",
             "input/hector_rcp85.ini")   # relative to run directory

# Data to score the resulting runs against. These data should include run_name, 
# year, component, variable, and value. Also (optionally) a 'weight' field if 
# you want to weight some observations more than others.
REFDATA <- read.csv("CMIP5_Tgav.csv")
library(caTools)
smoothDATA <- runmean(REFDATA$value, 15, alg=c("C"), endrule=c("mean"), align=c("center"))
REFDATA<-cbind(smoothDATA = smoothDATA, REFDATA)
REFDATA$value<-NULL
colnames(REFDATA)[1] <- "value"

#REFDATA$value<-runmean(REFDATA$value, 15, alg=c("C"), endrule=c("mean"), align=c("center"))

# Data structure holding information about variables to change between runs
# A list of lists, holding var name, INI file section, and values.
# If optimizing, the FIRST value is used as the starting point.
VARDATA <- list(
    k_max=list(section="ocean", values=c(0.8, 0.6, 1.0, 1.2)),
    t_mid=list(section="ocean", values=c(2.5, 2.0, 3.0, 4.0, 5.0)),
    slope=list(section="ocean", values=c(0, -0.5, -1.5, -2))
    #bc_b=list(section="temperature", values=c(0.1)),
    #oc_b=list(section="temperature", values=c(0.1)),
    #so2i_b=list(section="temperature", values=c(0.1)),
    #so2d_b=list(section="temperature", values=c(0.1))
)

# ----------------------------------------------------------------
# Settings you will probably won't have to change

RUN_NAME_TEXT   <- "run_name="       # search for this in INI file
SCRIPTNAME      <- "batchrunner.R"
OUTPUT_DIR    	<- "outputs/"
LOG_DIR			<- "logs/"
LOGFILE         <- "logfile"
OUTFILE         <- "outfile"
SEPARATOR		<- "-------------------"

library(dplyr)
library(ggplot2)
theme_set(theme_bw())
library(reshape2)

# -----------------------------------------------------------------------------
# Time-stamped output function
printlog <- function(msg="", ..., ts=TRUE, cr=TRUE) {
    if(ts) cat(date(), " ")
    cat(msg, ...)
    if(cr) cat("\n")
} # printlog

# -----------------------------------------------------------------------------
# write out log information
savelogdata <- function(run_name, score, vardata, vals, logfile) {
    logdata <- data.frame(date=date(), run_name=run_name)
    logdata$score <- score
    for(vn in 1:length(vardata))
        logdata[names(vardata)[vn]] <- vals[vn]
    printlog("Writing to", logfile)
    write.table(logdata, file=logfile, col.names=!file.exists(logfile), 
                append=file.exists(logfile), row.names=F, sep=",")
} # savelogdata

# -----------------------------------------------------------------------------
# Substitute a new parameter value into an input file
subparam <- function(flines, section, parameter, newvalue) {
    # Find 'section' and the next one after that
    secline <- grep(paste0("^\\[", section, "\\]$"), flines) 
    stopifnot(length(secline) > 0)
    nextsecline <- secline + grep("^\\[[a-zA-Z0-9_]*\\]$", flines[secline+1:length(flines)])[1]
    if(length(nextsecline) == 0) nextsecline <- length(flines)
    printlog("[", section, "] lines", secline, "-", nextsecline)
    
    # Find variable name occuring within the section
    varline <- secline - 1 + grep(paste0("^", parameter), flines[secline:nextsecline])
    stopifnot(length(varline) == 1)
    
    # Overwrite with new value
    flines[varline] <- paste0(parameter, "=", newvalue, " ; [MODIFIED]")
    printlog("Line", varline, ":", flines[varline]) 
    flines
}

# -----------------------------------------------------------------------------
# Substitute a new parameter value into an input file
score <- function(scoredata) {
    with(scoredata, sum((value.hector - value.ref) ^ 2 * weight, na.rm=T))
}

# -----------------------------------------------------------------------------
# Run model with a set of parameter values, log results, and score vs. reference data
run_hector <- function(vals, infiles, vardata, refdata=NULL, logfile=NULL, outfile=NULL) {
    
    printlog(vals)
    if(!is.null(refdata) & !"weight" %in% names(refdata)) 
        refdata$weight <- 1  # default: no weighting
    
    results <- data.frame()
    for(f in infiles) {
        f <- paste(RUN_DIRECTORY, f, sep="/")
        printlog("Working on", f)
        stopifnot(file.exists(f))
        flines <- readLines(f)
        printlog("Read", length(flines), "lines")
        
        # Find run name
        run_name_line <- flines[grep(RUN_NAME_TEXT,flines)]
        run_name <- sub(RUN_NAME_TEXT, "", gsub(" ", "", run_name_line))
        printlog("run_name =", run_name)
        
        for(vn in seq_len(length(vardata))) {  
            v <- names(vardata)[vn]
            flines <- subparam(flines,
                               section=vardata[[v]]$section,
                               parameter=v,
                               newvalue=vals[vn])
        }
        
        # At this point we've substituted in all new parameter values
        # Write out new ini file
        tf <- tempfile()
        printlog("Writing to", tf)
        cat(flines, file=tf, sep="\n")
        
        # Run Hector
        wd<-getwd()
        setwd(RUN_DIRECTORY) # need this for Windows
        cmd <- paste0(normalizePath(EXECUTABLE), " ", tf)
        printlog(cmd)
        error <- try(system(cmd, ignore.stdout = TRUE))
        setwd(wd)
        if(error) {
            stop("Hector error")
            # TODO return error/MAXINT or something like that
        }
        
        # Read outputstream
        osf <- paste0(RUN_DIRECTORY, "/output/outputstream_", run_name, ".csv")
        printlog("Reading", osf)
        d <- read.csv(osf, comment.char="#")
        #        print(head(d))
        
        # Subset for vars in refdata
        if(!is.null(refdata))
            d <- d[d$variable %in% unique(refdata$variable),]
        # no spinup
        d <- d[d$spinup==0,]
        # get rid of extra fields
        d$spinup <- d$units <- NULL
        
        # Save log if requested
        if(!is.null(logfile)) {
            if(!is.null(refdata)) {
                printlog("Merging with refdata")
                print(head(refdata))
                print(head(results))
                scoredata <- merge(refdata, d, 
                                   by=c("year", "run_name", "component", "variable"), 
                                   suffixes=c(".ref",".hector"))
                thisscore <- score(scoredata)
            } else
                thisscore <- NA
            
            savelogdata(run_name, thisscore, vardata, vals, logfile)
        }
        results <- rbind(results, d)
    }
    
    # At this point we've run all INI files requested
    
    if(!is.null(refdata)) {
        printlog("Merging with refdata")
        results_mrg <- merge(refdata, results, 
                             by=c("year", "run_name", "component", "variable"), 
                             suffixes=c(".ref",".hector"))
        
        # Compute overall match score
        finalscore <- score(results_mrg)
    } else
        finalscore <- NA

    # Save data if requested
    if(!is.null(outfile)) {
        for(vn in seq_len(length(vardata)))
            results[names(vardata)[vn]] <- vals[vn]
        results$score <- finalscore
        printlog("Writing to", outfile)
        write.table(results, file=outfile, col.names=!file.exists(outfile), 
                    append=file.exists(outfile), row.names=F, sep=",")
    }
    
    # Save log if requested
    if(!is.null(logfile))
        savelogdata(run_name="Combined", finalscore, vardata, vals, logfile)
    
    finalscore
} # run_hector


# -----------------------------------------------------------------------------
# Brute-force example driving run_hector above
bruteforce <- function(infiles, vardata=NULL, refdata=NULL, suffix="") {
    
    logfile <- paste0(LOGFILE, suffix, ".csv")
    outfile <- paste0(OUTFILE, suffix, ".csv")
    
    if(is.null(vardata)) {  # no parameter changing; just run infiles 
        return(invisible(
            run_hector(vals=NULL, infiles=infiles,
                       vardata=vardata, refdata=refdata,
                       logfile=logfile, outfile=outfile)))
    }
    
    for(i in seq_len(nrow(runlist))) {
        printlog(SEPARATOR)
        printlog(i)
        runlist[i, "score"] <- run_hector(vals=as.numeric(runlist[i,]),
                                          infiles=infiles,
                                          vardata=vardata,
                                          refdata=refdata,
                                          logfile=logfile,
                                          outfile=outfile)
    } # for
    invisible(runlist)
} # bruteforce


# -----------------------------------------------------------------------------
# Optimizing function using run_hector above
optimize <- function(infiles, vardata, refdata, suffix="", graph=TRUE) {
    
    logfile <- paste0(LOGFILE, suffix, ".csv")
    outfile <- paste0(OUTFILE, suffix, ".csv")
    
    # Get parameter starting values (the first ones in the list)    
    parlist <- unlist(lapply(vardata, FUN=function(x) { x=x$values[1] }))
    
    # Optimize!
    results <- optim(par=parlist,
                     fn=run_hector,
                     gr=NULL,
                     control=list(maxit=200,
                                  reltol=1e-5), # don't need default 1e-8
                     # following parameters get passed to run_hector
                     infiles, vardata, refdata, logfile, outfile)
    
    if(graph) graph_optimized(results, vardata, refdata, suffix)
    
    invisible(results)
}

# -----------------------------------------------------------------------------
# Graph results of optimize() above
graph_optimized <- function(results, vardata, refdata, suffix="") {
    logfile <- paste0(LOGFILE, suffix, ".csv")
    outfile <- paste0(OUTFILE, suffix, ".csv")
    
    printlog("Optimization message:", results$message)
    printlog("Final score:", results$value)
    printlog("Final parameter values:", results$par)
    finalscore <- results$value
    title <- paste(names(vardata), round(results$par, 3), collapse=",")
    
    printlog("Reading logfile...")
    logdata <- read.csv(logfile)
    logdata$step <- 1:nrow(logdata)
    p1 <- ggplot(logdata, aes(step, score, color=run_name)) + geom_line()   
    p1 <- p1 + ggtitle(title)
    print(p1)
    ggsave(paste0("optimize-scores", suffix, ".pdf"))
    
    logmelt <- melt(logdata, id.vars=c("date", "run_name", "score", "step"))
    p2 <- ggplot(logmelt[logmelt$run_name=="Combined",], aes(step, value)) 
    p2 <- p2 + geom_point() + geom_line()
    p2 <- p2 + facet_grid(variable~., scales="free") 
    p2 <- p2 + ggtitle(title)
    print(p2)
    ggsave(paste0("optimize-values", suffix, ".pdf"))
    
    printlog("Reading outfile...")
    outdata <- read.csv(outfile)
    
    # Assign group numbers
    # The group_by solution below (using arbitrary column names) from
    # http://stackoverflow.com/questions/21208801/group-by-multiple-columns-in-dplyr-using-string-vector-input
    outdata$group <- outdata %>%
        group_by_(.dots=lapply(names(VARDATA), as.symbol)) %>%
        group_indices()
    
    p3 <- ggplot(outdata, aes(year, value)) 
    p3 <- p3 + geom_line(aes(group=group), color="grey")  # other runs
    p3 <- p3 + geom_point(data=refdata)   # reference data
    p3 <- p3 + geom_line(data=subset(outdata, score==min(score)), color="red", size=2)
    p3 <- p3 + facet_wrap(~run_name, scales="free")
    p3 <- p3 + ggtitle(title)
    print(p3)
    ggsave(paste0("optimize-runs", suffix, ".pdf"))
    
    invisible(results)
} # graph_optimized

# Examples for how to use this code

# Run the four RCP cases (no multiple runs or comparison to data)
#bruteforce(INFILES)

# An optimization exercise
# Weight 1950-2050 observations heavily
#REFDATA$weight <- 1
#REFDATA$weight[REFDATA$year > 1960 & REFDATA$year <= 2150] <- 2

#optimize(INFILES[1], VARDATA, REFDATA, suffix="-new-26") %>%
#    graph_optimized(VARDATA, REFDATA, suffix="-new-26")
#optimize(INFILES[2], VARDATA, REFDATA, suffix="-new-45")
#optimize(INFILES[3], VARDATA, REFDATA, suffix="-new-60")
#optimize(INFILES[4], VARDATA, REFDATA, suffix="-new-85")

# optimize(INFILES, VARDATA, REFDATA, suffix="-new-combined") %>%
#     graph_optimized(VARDATA, REFDATA, suffix="-new-combined")
# REFDATA$weight[REFDATA$run_name=="rcp26"] <- 0.0
# optimize(INFILES, VARDATA, REFDATA, suffix="-new-not26") %>%
#     graph_optimized(VARDATA, REFDATA, suffix="-new-not26")
