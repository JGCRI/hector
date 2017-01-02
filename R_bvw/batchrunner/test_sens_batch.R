# Ben V-W, June 2016
# test_sens_batch.R
# Just trying to do a test batch of Hector runs
# - modifying climate sensitivity

source("batchrunner.R")

EXECUTABLE <- "/Users/bvegawe2/Library/Developer/Xcode/DerivedData/hector-deqsagdhytasnoeyvihanucdtxel/Build/Products/Release/hector"

# Hector INI file to modify parameters in and run
INFILE <- "input/hector_rcp85.ini" # relative to run directory
# Where batch ini files are saved for reference
INROOT <- "input/" # relative to run directory

RUN_DIRECTORY <- "../../"
# Data structure holding information about variables to change between runs
# A list of lists, holding var name, INI file section, and values.
VARDATA <- list(
#S=list(section="temperature",values=c(1.0,3.0,5.0,7.0,9.0))
#S=list(section="temperature",values=c(7.0,9.0))
k_max=list(section="ocean", values=c(0.1, 0.3, 0.5, 0.7, 0.9, 1.1))
#slope=list(section="ocean",values=c(0.01, 0.1, 1.0, 10.0))
)

# Settings you will probably won't have to change

OUTPUT_DIR    	<- "outputs/batch/"
OUTFILE         <- "outfile"
SEPARATOR		<- "-------------------"

library(dplyr)
library(ggplot2)
theme_set(theme_bw())
library(reshape2)

# -----------------------------------------------------------------------------
# Substitute a new parameter value into an input file
subparam <- function(flines, section, parameter, newvalue) {
    # Find 'section' and the next one after that
    secline <- grep(paste0("^\\[", section, "\\]$"), flines)
    stopifnot(length(secline) > 0)
    nextsecline <- secline + grep("^\\[[a-zA-Z0-9_]*\\]$", flines[secline+1:length(flines)])[1]
    if(length(nextsecline) == 0) nextsecline <- length(flines)
    
    # Find variable name occuring within the section
    varline <- secline - 1 + grep(paste0("^", parameter), flines[secline:nextsecline])
    stopifnot(length(varline) == 1)
    
    # Overwrite with new value
    flines[varline] <- paste0(parameter, "=", newvalue, " ; [MODIFIED]")
    flines
}

# -----------------------------------------------------------------------------
# Run model with replacement param values, save output to new .csv files
replace_run_hector <- function(infile, replacevars, suffix) {
    
    results <- data.frame()
    f <- paste(RUN_DIRECTORY, infile, sep="/")
    printlog("Working on", f)
    stopifnot(file.exists(f))
    flines <- readLines(f)
    
    # Find run name
    run_name_line <- flines[grep(RUN_NAME_TEXT,flines)]
    run_name <- sub(RUN_NAME_TEXT, "", gsub(" ", "", run_name_line))
    
    for(vn in seq_len(length(replacevars))) {
        v <- names(replacevars)[vn]
        flines <- subparam(flines,
                            section=replacevars[[v]]$section,
                            parameter=v,
                            newvalue=replacevars[[v]]$value)
    }
        
    # At this point we've substituted in all new parameter values
    
    wd<-getwd()
    setwd(RUN_DIRECTORY) # need this for Windows
    
    # Write out new ini file
    new_ini <- paste0(INROOT,"hector_",suffix,".ini")
    file.create(new_ini)
    cat(flines, file=new_ini, sep="\n")
        
    # Run Hector
    cmd <- paste0(normalizePath(EXECUTABLE), " ", new_ini)
    printlog(cmd)
    error <- try(system(cmd, ignore.stdout = TRUE))
    setwd(wd)
    if(error) {
        print("Hector error, on to the next run.")
        #stop("Hector error")
        # TODO return error/MAXINT or something like that
    }
}

# -----------------------------------------------------------------------------
# Here's a sample where we just try a few climate sensitivity values
suffix_root = "rcp85"
for(i in seq_len(length(VARDATA))){
    suffix_root = paste0(suffix_root,strsplit(names(VARDATA)[i],"")[[1]][1])
}
for(j in seq_len(length(VARDATA$k_max$values))){
    suffix = paste0(suffix_root,j)
    replacevars = VARDATA
    replacevars$k_max$value = VARDATA$k_max$values[j]
    replacevars$run_name=list(section="core",value=suffix)
    replace_run_hector(INFILE,replacevars,suffix)
}
