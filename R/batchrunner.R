# batchrunner.R
# Ben Bond-Lamberty January 2015
#
# R script to batch-run Hector:
# - run all four RCPs at once
# - do a brute-force run varying parameters
# - do a optimizer run looking for best set of parameters

# ----------------------------------------------------------------
# Settings you will probably need to change
# Location of the Hector executable to run
EXECUTABLE <- "/Users/ben/Library/Developer/Xcode/DerivedData/hector-cglywsekpzlzoxaqbdkpxegrhaba/Build/Products/Debug/hector"

# Location of the main run (working) directory
RUN_DIRECTORY <- "../"

# Hector INI files to modify parameters in and run
INFILES <- c("input/hector_rcp26.ini", 
             "input/hector_rcp45.ini", 
             "input/hector_rcp60.ini",
             "input/hector_rcp85.ini")   # relative to run directory

# Data to score the resulting runs against
# These data should include run_name, year, component, variable, value
REFDATA <- read.csv("CMIP5_Tgav.csv")

# Data structure holding information about variables to change between runs
# A list of lists, holding var name, INI file section, and values
VARDATA <- list(
    k_max=list(section="ocean", values=c(0.8, 0.6, 1.0, 1.2)),
    t_mid=list(section="ocean", values=c(2.75, 2.0, 3.0, 4.0, 5.0)),
    slope=list(section="ocean", values=c(-1, -0.5, -1.5, -2)),
    k_min=list(section="ocean", values=c(0, 0.1, 0.2, 0.3)),
    S=list(section="temperature", values=c(3, 2, 2.5))
)

# ----------------------------------------------------------------
# Settings you will probably won't have to change

RUN_NAME_TEXT <- "run_name="
SCRIPTNAME      <- "batchrunner.R"
OUTPUT_DIR    	<- "outputs/"
LOG_DIR			<- "logs/"
SEPARATOR		<- "-------------------"


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
    # Find section
    secline <- grep(paste0("^\\[", section, "\\]$"), flines) 
    stopifnot(length(secline) > 0)
    printlog(section, "secline =", secline)
    
    # Find next section
    nextsecline <- secline + grep("^\\[[a-zA-Z0-9_]*\\]$", flines[secline+1:length(flines)])[1]
    if(length(nextsecline) == 0) nextsecline <- length(flines)
    printlog("nextsecline =", nextsecline)
    
    # Find variable name occuring after that point
    varline <- secline - 1 + grep(paste0("^", parameter), flines[secline:nextsecline])
    printlog(parameter, "line =", varline)
    stopifnot(length(varline) == 1)
    
    # Insert new value
    flines[varline] <- paste0(parameter, "=", newvalue, " ; [MODIFIED]")
    printlog(flines[varline])
    
    flines
}

# -----------------------------------------------------------------------------
# Run model with a set of parameter values, log results, and score vs. reference data
run_hector <- function(vals, infiles, vardata, refdata=NULL, logfile=NULL, outfile=NULL) {
    printlog(vals)
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
        
        for(vn in 1:length(vardata)) {  
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
        cmd <- paste0("cd ", RUN_DIRECTORY, "; ", EXECUTABLE, " ", tf)
        printlog(cmd)
        error <- try(system(cmd, ignore.stdout = TRUE))
        if(error) {
            stop("Hector error")
            # TODO return error/MAXINT or something like that
        }
        
        # Read outputstream
        osf <- paste0(RUN_DIRECTORY, "/output/outputstream_", run_name, ".csv")
        printlog("Reading", osf)
        d <- read.csv(osf)
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
                #    print(head(refdata))
                #    print(head(results))
                scoredata <- merge(refdata, d, 
                                   by=c("year", "run_name", "component", "variable"), 
                                   suffixes=c(".ref",".hector"))
                thisscore <- sum((scoredata$value.hector - scoredata$value.ref) ^ 2, na.rm=T)
            } else
                thisscore <- NA
            
            savelogdata(run_name, thisscore, vardata, vals, logfile)
        }
        results <- rbind(results, d)
    }
    
    # At this point we've run all INI files requested
    # Save data if requested
    if(!is.null(outfile)) {
        for(vn in 1:length(vardata))
            results[names(vardata)[vn]] <- vals[vn]
        printlog("Writing to", outfile)
        write.table(results, file=outfile, col.names=!file.exists(outfile), 
                    append=file.exists(outfile), row.names=F, sep=",")
    }
    
    if(!is.null(refdata)) {
        printlog("Merging with refdata")
        #    print(head(refdata))
        #    print(head(results))
        results <- merge(refdata, results, 
                         by=c("year", "run_name", "component", "variable"), 
                         suffixes=c(".ref",".hector"))
        print(head(results))
        
        # Compute overall match score
        score <- sum((results$value.hector - results$value.ref) ^ 2, na.rm=T)        
    } else
        score <- NA
    
    # Save log if requested
    if(!is.null(logfile))
        savelogdata(run_name="Combined", score, vardata, vals, logfile)
    
    score
}


# -----------------------------------------------------------------------------
# Brute-force example driving run_hector above
bruteforce <- function(vardata, infiles, refdata=NULL, logfile=NULL, outfile=NULL) {
    runlist <- expand.grid(lapply(vardata, FUN=function(x) { x=x$values }))
    
    for(i in 1:nrow(runlist)) {
        printlog(SEPARATOR)
        printlog(i)
        runlist[i, "score"] <- run_hector(vals=as.numeric(runlist[i,]),
                                          infiles=infiles,
                                          vardata=vardata,
                                          refdata=refdata,
                                          logfile=logfile,
                                          outfile=outfile)
    }
    invisible(runlist)
}


# -----------------------------------------------------------------------------
# Optimizing example driving run_hector above
optimize <- function(vardata, infiles, refdata, logfile=NULL, outfile=NULL) {
    
    # Haven't tried this yet, but basically
    
    parlist <- unlist(lapply(vardata, FUN=function(x) { x=x$values[1] }))
    
    optim(par=parlist,
          fn=run_hector,
          gr=NULL,
          # following parameters get passed to run_hector
          infiles, vardata, refdata, logfile, outfile)
}

    
#bruteforce(VARDATA, INFILES, REFDATA, logfile="logfile.csv", outfile="outfile.csv")



if(0) {
    library(ggplot2)
    
    # qplot(year,value,data=test,geom="line",color=t_mid,group=t_mid)+facet_wrap(~run_name,nrow=2,ncol=2)
    logresults <- read.csv("logfile.csv")
    p1 <- qplot(k_max, t_mid, geom="tile", fill=score, data=logresults)
    p1 <- p1 + facet_wrap(~run_name)
    print(p1)
    ggsave("results_score.pdf")
    
    
    results <- read.csv("outfile.csv")
    p2 <- qplot(year,value,data=results,geom="line",color=t_mid,linetype=factor(k_max),group=paste(t_mid,k_max))
    p2 <- p2 + facet_wrap(~run_name,nrow=2,ncol=2)
    print(p2)
    ggsave("results_time.pdf")
}
