# 2-montecarlo.R
# Run Hector Monte Carlo for ocean acidification paper
# Ben Bond-Lamberty January 2016

source("0-functions.R")  # support functions

SCRIPTNAME  	<- "2-montecarlo.R"
PROBLEM       <- FALSE
TESTING       <- FALSE   # if TRUE, don't actually run model
TESTING_DELAY <- 0      # "model run time" (with no real model)

# Hector-related settings (constant)
HECTOR_INPUTS     <- "samples/hector"  # base input files
HECTOR_OUTPUT_FILE <- "output/outputstream.csv" # pattern (relative to TEMPDIR)
HECTOR_EXE        <- "/people/d3x290/hector/source/hector" # (absolute path) PIC.  BVW-dunno what this is
HECTOR_EXE <- "/Users/bvegawe2/Library/Developer/Xcode/DerivedData/hector-deqsagdhytasnoeyvihanucdtxel/Build/Products/Debug/hector"

# Monte Carlo settings - all runs
MAX_N_MC     <- 1e6  # overrides vardata length
TEMPDIR      <- "tempdir/"
PARALLELIZE  <- FALSE

# Output processing settings
OUTPUT_MC                 <- "output/"
HECTOR_OUTPUT_VARIABLES   <- c("Tgav", 
                               "Ca", 
                               "pH_LL", "pH_HL", 
                               "Temp_LL", "Temp_HL",
                               "OmegaAr_HL", "OmegaAr_LL",
                               "OmegaCa_HL", "OmegaCa_LL",
                               "CO3_HL", "CO3_LL")
HECTOR_OUTPUT_YEARRANGE   <- 1850:2300


# -----------------------------------------------------------------------------
# Insert run number into a filename, e.g. path/file.ext -> path/file_x.ext
makename <- function(fqfn, x, sep = "_") {
  fn <- basename(fqfn) %>%
    strsplit(split = ".", fixed = TRUE) %>%
    .[[1]]
  if(is.na(fn[2])) { # no extension
    newfn <- paste0(fn[1], sep, x)
  } else {
    newfn <- paste0(fn[1], sep, x, ".", fn[2])
  }
  file.path(dirname(fqfn), newfn)
}

# -----------------------------------------------------------------------------
# Substitute a new parameter value into an in-memory input file
subparam <- function(flines, section, parameter, newvalue) {
  printlog("Looking for", parameter, "in [", section, "]")
  # Find 'section' and the next one after that
  secline <- grep(paste0("^\\[", section, "\\]$"), flines) 
  stopifnot(length(secline) > 0)
  nextsecline <- secline + grep("^\\[[a-zA-Z0-9_]*\\]$", flines[secline+1:length(flines)])[1]
  if(length(nextsecline) == 0) {
    nextsecline <- length(flines)
  }
  #  printlog("[", section, "] lines", secline, "-", nextsecline)
  
  # Find variable name occuring within the section
  varline <- secline - 1 + grep(paste0("^", parameter), flines[secline:nextsecline])
  stopifnot(length(varline) == 1)
  
  # Overwrite with new value
  flines[varline] <- paste0(parameter, "=", newvalue, " ; [MODIFIED ", date(), "]")
  printlog("New line", varline, ":", flines[varline]) 
  flines
}

# -----------------------------------------------------------------------------
# run an entire Monte Carlo set
run_mc <- function(runname_mc, vardata_file, hector_ini_file) {
  
  starttime <- Sys.time()
  
  logfile <- paste0(SCRIPTNAME, "_", RUNSET_NAME, "_", runname_mc, ".log.txt") %>%
    file.path(outputdir(), .)
  openlog(logfile, sink = TRUE)
  printlog(SEPARATOR)
  printlog("run_mc: starting new run", runname_mc)
  printlog("Writing to", logfile)
  
  # Copy input data to a new temporary directory
  if(dir.exists(TEMPDIR)) {
    printlog("Removing TEMPDIR...")
    system2("rm", args = c("-rf", TEMPDIR))  # rm -rf
  }
  hectorinputs <- paste0(HECTOR_INPUTS, "/")  # has to end with slash
  if(system2("cp", args = c("-R", hectorinputs, TEMPDIR))) {  # cp -R
    stop("Couldn't create temp directory")
  }
  printlog("Copying", hectorinputs, "to", TEMPDIR)
  
  #   Load pristine input files and variable value data
  vardata <- read_csv(vardata_file)
  printlog("Reading", hector_ini_file)
  inifile_original <- readLines(file.path(TEMPDIR, hector_ini_file))
  
  # Loop (possibly in parallel) through all runs
  N <- min(nrow(vardata), MAX_N_MC)
  problemlist <- c()
  printlog("Starting", N, "simulations")
  
  if(PARALLELIZE) {
    library(doParallel)  # doParallel_1.0.10
    cl <- makeCluster(2)
    registerDoParallel(cl)
    printlog("Starting parallel backend:", getDoParName(), getDoParVersion())
    printlog(capture.output(cl))
    
    foreach(i = seq_len(N), 
            .packages = c("luzlogr", "dplyr", "readr"),
            .combine = 'unlist',
            .export = c("single_hector_run", "SEPARATOR", "subparam", "makename",
                        "TEMPDIR", "TESTING", "TESTING_DELAY",
                        "RUNSET_NAME",
                        "HECTOR_OUTPUT_FILE", "HECTOR_EXE", 
                        "HECTOR_OUTPUT_YEARRANGE", "HECTOR_OUTPUT_VARIABLES"),
            .inorder = FALSE) %dopar% {
              single_hector_run(i, inifile_original, vardata, runname_mc, hector_ini_file)
            } -> problemlist
    
    stopCluster(cl)
  } else {   # Serial
    for(i in seq_len(N)) {
      prob <- single_hector_run(i, inifile_original, vardata, runname_mc, hector_ini_file)
      problemlist <- c(problemlist, prob)  # prob is NULL if no problem
    } # for
  }
  
  # ------------ Almost done! Copy and gzip results
  
  # Copy results into a single file and write to output directory
  printlog("Copying and assembling output files...")
  ofile <- paste0(RUNSET_NAME, "_", runname_mc, "_", format(Sys.time(), "%Y%m%d%H%M"), ".csv") %>%
    file.path(RUNSET_OUTPUT_DIR, .)
  files <- list.files(TEMPDIR, pattern = paste0(runname_mc, "_[0-9]+.csv"))
  for(f in files) {
    readr::read_csv(file.path(TEMPDIR, f)) %>%
      write_csv(ofile, append = file.exists(ofile))
  }
  if(require(R.utils)) {  # R.utils not automatically available on Linux R
    printlog("gzipping", ofile, "...")
    R.utils::gzip(ofile, overwrite = TRUE)
    ofile_gzip <- paste0(ofile, ".gz")
    ofile_general <- file.path(RUNSET_OUTPUT_DIR, 
                               paste0(RUNSET_NAME, "_", runname_mc, ".csv.gz"))
    printlog("Creating", ofile_general)
    file.copy(ofile_gzip, ofile_general, overwrite = TRUE)
  }
  
  # ------------ Done
  
  printlog("All done with", SCRIPTNAME, "run", runname_mc)
  printlog(length(problemlist), "problem(s):", paste(problemlist, collapse = " "))
  save_data(problemlist, fname = paste0("problems_", runname_mc, ".csv"))
  printlog("Elapsed time:", round(difftime(Sys.time(), starttime, units = "mins"), 1), "minutes")
  closelog()
  
  if(PROBLEM) warning("There was a problem - see log")
  ofile_gzip
} # run_mc

# -----------------------------------------------------------------------------
# Set up and run Hector for a single MC instance
single_hector_run <- function(i, inifile_original, vardata, runname_mc, hector_ini_file) {
  problem_encountered <- NULL
  # Substitute a new value for each variable
  # Data frame headers are of form {INI section}.{variable}
  printlog(SEPARATOR, RUNSET_NAME, runname_mc, i)
  inifile <- inifile_original
  for(v in names(vardata)) {
    vn <- strsplit(v, ".", fixed = TRUE)[[1]]
    inifile <- subparam(inifile, vn[1], vn[2], vardata[i, v])  
  }
  
  # Substitute new run name
  printlog("Making new run name...")
  run_name <- paste(runname_mc, i, sep="_")
  inifile <- subparam(inifile, "core", "run_name", run_name)
  
  #   Write out new INI file
  inifile_name <- makename(hector_ini_file, run_name)
  fq_inifile_name <- file.path(TEMPDIR, inifile_name)
  printlog("Writing", inifile_name)
  writeLines(inifile, fq_inifile_name)
  
  #   Run Hector
  hector_ofile <- makename(HECTOR_OUTPUT_FILE, run_name) %>% file.path(TEMPDIR, .)
  if(TESTING) {  # copy output file, as if model ran
    printlog("Pretending to run model...")
    file.copy(file.path(TEMPDIR, "output/sample_outputstream_rcp45.csv"), hector_ofile)
    Sys.sleep(TESTING_DELAY)
  } else {
    printlog("Running model...")
    olddir <- getwd()
    setwd(TEMPDIR)
    result <- system2(HECTOR_EXE, inifile_name, stdout = FALSE)
    setwd(olddir)
    
    if(result) {
      flaglog("Uh oh, Hector didn't exit normally:", result)
      printlog(runname_mc, i)
      problem_encountered <- i
    }
  }
  
  #   Load results, filter, write to temporary file
  if(file.exists(hector_ofile)) {
    printlog("Loading and filtering Hector output...")
    ofile <- paste0(run_name, ".csv") %>% file.path(TEMPDIR, .)
    readr::read_csv(hector_ofile, skip = 1) %>%
      filter(variable %in% HECTOR_OUTPUT_VARIABLES, 
             !spinup, 
             year %in% HECTOR_OUTPUT_YEARRANGE) %>%
      select(-spinup, -run_name, -component, -units) %>%   # reduce file size
      mutate(run_number = i) %>%
      write_csv(ofile)
    printlog("Written to", ofile)
  } else {
    flaglog("No output file generated!")
  }
  problem_encountered   # return NULL (no problem) or i (if yes)
}


# ==============================================================================
# Main 

# Load the runset file, and do all the runs listed in it
runset <- read_csv(RUNSET_FILE)

for(i in seq_len(nrow(runset))) {
  run_mc(runset$runname_mc[i], VARDATA_FILE, runset$hector_ini_file[i])
}

