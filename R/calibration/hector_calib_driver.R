##==============================================================================
## hector_calib_driver.R
## 
## Original code for BRICK written by Tony Wong and Alexander Bakker
##   (https://github.com/scrim-network/BRICK, DOI:10.5194/gmd-10-2741-2017)
## Modified for Hector by Ben Vega-Westhoff
##==============================================================================
## Implementation of MCMC (RAM) calibration of Hector simple climate model
## (https://github.com/JGCRI/hector, DOI:10.5194/gmd-8-939-2015)
##  
## Can add parameters to calibration in hector_calib_params.R.
## Can add/switch out observational constraints in obs_readData.R. 
##
## Robust adaptive MCMC (RAM) calibration for model parameters (Vihola, 2001)
## Differential evolution (DE) optimization to find suitable initial parameter guesses
## (Storn and Price, 1997; Price et al, 2005)
##
## Typical default use: $ Rscript hector_calib_driver.R -f default_calib
## See the input options: $ Rscript hector_calib_driver.R -h
##==============================================================================

rm(list=ls())	        # Clear all previous variables

## Required packages/libraries
library(optparse)       # Option parsing
library(DEoptim)        # Use differential evolution for initial parameter guesses
library(adaptMCMC)	# Use robust adaptive Metropolis MCMC method

## Inputs
option_list = list(
    make_option( c("-f", "--folder"), type="character", default=NULL       ,
		 help = "folder for this calibration", metavar="character"),
    make_option( c("-n", "--niter") , type="integer"  , default=5E4        ,
		 help = "length of each chain (default= %default)",
	 	 metavar= "integer"),
    make_option( c("-g", "--gr")    , type="logical"  , default=TRUE       ,
		 help = "Use Gelman Rubin burn-in diagnostic? (default= %default)",
		 metavar= "logical"),
    make_option( c("--forcing")     , type="logical"  , default=FALSE      ,
		 help = "Do forcing-based runs, largely ignoring Hector's carbon cycle? (default= %default)",
		 metavar= "logical"), 
    make_option( c("--model_set")   , type="character", default="all_model",
		 help = "Which set of free model parameters for calibration? (default = %default, see hector_calib_params.R for options)",
                 metavar="character" ), 
    make_option( c("--obs_set")     , type="character", default="all_obs"  ,
                 help = "Which set of observation data sets against which to calibrate? (default = %default, see hector_calib_params.R for options)",
		 metavar="character" ), 
    make_option( c("--continue")    , type="character", default=NULL       ,
		 help = "MCMC list to continue, saved as a .RData file (default = %default)",
		 metavar="character" ) )

opt_parser = OptionParser(option_list=option_list);
opt = parse_args(opt_parser);

if (is.null(opt$folder)){
    print_help(opt_parser)
    stop("At least one argument must be supplied (calibration folder).",
    call.=FALSE)
}

## Set input variables
calib.folder       = opt$folder 	# Folder for temporary files, diagnostic plots, and posterior parameters
forcing.based      = opt$forcing 	# If FALSE, use emissions-based Hector runs     
niter.mcmc         = opt$niter 		# Iterations per MCMC chain, 50k has been ample for DOECLIM parameters (based on GR diagnostic)
gr.mcmc            = opt$gr 		# If TRUE, perform Gelman Rubin burn-in diagnostic (a second chain is run for this if we only have one). If FALSE, assume the second half of the chain has converged (not necessarily a great assumption!).
model.set          = opt$model_set 	# Set of free parameters for the calibration. Set names are found in hector_calib_params.R.
obs.set            = opt$obs_set 	# Set of observation data sets against which to calibrate. Set names are found in hector_calib_params.R.
continue.mcmc      = opt$continue	# RData file containing MCMC chain list to continue (we will set p0 from here)

## Set the seed (for reproducibility)
set.seed(1234)

## Make projections? Only needed if calibrating against other projections (for emulation)
l.project=FALSE

## Skip DEoptim? Can skip if we have presaved DEoptim output from previous similar calibration
## DEoptim takes ~10k runs for the DOECLIM parameters + offsets (S, diff, alpha, temp offset, ocheat offset)
skip.DEoptim = FALSE

## Set up MCMC stuff here
parallel.mcmc = FALSE   # If true, run chains in parallel. Not yet implemented!
nparallel.mcmc = 4      # If parallel.mcmc, how many parallel chains?

## Create calibration folder
if( strsplit( calib.folder,"/" )[[1]][1] != "" ) {
    calib.folder = paste0( getwd(), "/", calib.folder ) # Convert to absolute path
}
if( !file.exists( calib.folder ) ) { system( paste0( "mkdir ", calib.folder ) ) }

## Get the forcing data (before possible aerosol scaling)
if( forcing.based ) {
    forcing.root = "obs_constraints/forcing/"
    file = ifelse( !l.project, "forcing_hindcast.csv",
           ifelse( scenario == 2.6, "forcing_rcp26.csv",
  	   ifelse( scenario == 4.5, "forcing_rcp45.csv",
  	   ifelse( scenario == 6.0, "forcing_rcp6.csv" ,
  	   ifelse( scenario == 8.5, "forcing_rcp85.csv")))))
    forcing.file = paste0( getwd(), "/", forcing.root, file )
    forcing = read.csv( forcing.file, header=TRUE )
    begyear = forcing$year[1]
    endyear = forcing$year[length(forcing$year)]
    print( "Doing calibration of forcing-based Hector" ) 
} else { forcing = NULL; begyear = endyear = NULL }

## Get emissions file
emissions.root = "obs_constraints/emissions/"
file = ifelse( !l.project, "hindcast_emissions.csv",
       ifelse( scenario == 2.6, "RCP26_emissions.csv",
       ifelse( scenario == 4.5, "RCP45_emissions.csv",
       ifelse( scenario == 6.0, "RCP6_emissions.csv" ,
       ifelse( scenario == 8.5, "RCP85_emissions.csv")))))
emissions.file = paste0( getwd(), "/", emissions.root, file )
volcanic.file = paste0( getwd(), "/", emissions.root, "volcanic_RF.csv" )
emissions = read.csv( emissions.file, header=TRUE, skip=3 )

## Set mod.time to include the forcing and/or emissions data needed to run Hector
if(is.null(begyear)){ begyear = emissions$Date[1] 
} else{ begyear = max(forcing$year[1],emissions$Date[1]) }
if(is.null(endyear)){ endyear = emissions$Date[length(emissions$Date)] 
} else{ endyear = min(forcing$year[length(forcing$year)],emissions$Date[length(emissions$Date)])}
mod.time = begyear:endyear
print( paste0( "Calibration will include Hector runs from ", begyear, " to ", endyear ) )

## Write a template .ini file for this calibration. 
## Individual parameters are updated in hectorwrapper().
## Here we just change things that are the same for the entire calibration.
source("subparam.R") # Useful function to replace param values in .ini file
if(forcing.based){ template.default = "input_templates/default_forcing_nowrite.ini"
} else { template.default = "input_templates/default_nowrite.ini" }
lines = readLines(template.default)
lines = gsub("obs_constraints/emissions/RCP45_emissions.csv", emissions.file, lines) #Replace default emissions file location
lines = gsub("obs_constraints/emissions/volcanic_RF.csv",volcanic.file, lines) #Replace default volcanic emissions file location
lines = subparam( lines, "core", "startDate", begyear - 1 ) #Hector's first calcs are for startDate+1
lines = subparam( lines, "core", "endDate", endyear )
lines = subparam( lines, "forcing", "baseyear", begyear )
ini.template = paste0( calib.folder, "/template.ini" )
cat( lines, file=ini.template, sep="\n" )

## Get the details for all of the free parameters in this calibration
source('hector_calib_params.R')
param.list   = param_calib_details( model_set = model.set, obs_set = obs.set )
params       = param.list$params      ; parnames      = param.list$parnames   
sections     = param.list$sections    ; in.hector     = param.list$in.hector   
in.DEoptim   = param.list$in.DEoptim  ;
p0           = param.list$p0          ; bound.lower   = param.list$bound.lower         
bound.upper  = param.list$bound.upper ; step.mcmc     = param.list$step.mcmc
obs.ts       = param.list$obs.ts      ; output.vars   = param.list$compare.var 
output.components = param.list$compare.component
index.DEoptim = param.list$index.DEoptim

print( "Calibrating these parameters: " )
for( sec in unique(sections) ){ 
    print( paste( params[sections == sec], collapse = ", " ) ) 
}
print( "Using these observational constraints: " )
print( paste( obs.ts, collapse = ", " ) )

## Read in useful functions
source('hectorwrapper.R')	# Runs Hector
source('compute_indices.R')	# Computes the overlap indices of two time series
source('forcing_total.R')	# Adjusts total forcing to account for any aerosol scaling
source('convertVars.R')         # Function to convert model output to match observational output variables/units

## Read in all data sets against which we will calibrate
source('obs_readData.R')
## Then gather up all the data/model indices for comparisons. use lists to avoid
## enormous amounts of input to the MCMC functions.
## Also gather actual observation/error values.
midx = list(); oidx = list(); obs = list(); obs.err = list()
norm.lower = list(); norm.upper = list()
for ( i in 1:length(obs.ts) ) {
    ts = obs.ts[[i]]
    midx[[ts]]       = obs.all[[ts]]$midx
    oidx[[ts]]       = obs.all[[ts]]$oidx
    obs[[ts]]        = obs.all[[ts]]$obs
    obs.err[[ts]]    = obs.all[[ts]]$obs.err
    norm.lower[[ts]] = obs.all[[ts]]$norm.lower
    norm.upper[[ts]] = obs.all[[ts]]$norm.upper
}

## Which model indices should be used to normalize in same way as data?
l.idx = c(); u.idx = c()
for( i in 1:length(obs.ts) ) {
    l = norm.lower[[obs.ts[[i]]]]
    u = norm.upper[[obs.ts[[i]]]]
    if( !is.na(l) ) { l.idx = append( l.idx, which( mod.time == l ) ) 
    } else { l.idx = append( l.idx, NA ) }
    if( !is.na(u) ) { u.idx = append( u.idx, which( mod.time == u ) )
    } else { u.idx = append( u.idx, NA ) }
}
ind.norm.data = data.frame( obs.ts, l.idx, u.idx) 		

##==============================================================================

## Use 'DEoptim' (differential evolution optimization) to find better initial parameters
##   p0 initial parameter guesses based on Urban and Keller (2010)
##   These are okay, and work, but can improve using differential evolution optimization
##   (as long as you use a large enough vector population (at least 10*[# parameters]))

#Can skip to go straight to mcmc, instead using the best values from a previous DEoptim run (see commented example below)
if(is.null(continue.mcmc)){
    print( "Starting DEoptim (differential evolution optimization) to find initial parameters for the MCMC chain" )
    source('hector_DEoptim.R')

    niter.deoptim=200          # number of iterations for DE optimization
    NP.deoptim=10*length(p0[index.DEoptim])   # population size for DEoptim (do at least 10*[N parameters])
    F.deoptim=0.8              # as suggested by Storn et al (2006)
    CR.deoptim=0.9             # as suggested by Storn et al (2006)

    t.beg=proc.time()          # save timing
    outDEoptim <- DEoptim( minimize_residuals_hector            , 
			   bound.lower[index.DEoptim]           , bound.upper[index.DEoptim],
   		           DEoptim.control(NP = NP.deoptim, itermax = niter.deoptim, F = F.deoptim, 
		                           CR = CR.deoptim, trace = FALSE)                               , 
			   parnames.in = parnames[index.DEoptim], in.hector.in = in.hector[index.DEoptim],
			   sections.in = sections[index.DEoptim], calib.folder = calib.folder            ,
			   forcing.in = forcing                 , ini.template = ini.template            ,
			   output.vars = output.vars            , output.components = output.components  ,
			   mod.time = mod.time                  , l.project = l.project                  , 
		           oidx = oidx, midx = midx             , obs = obs, obs.err = obs.err           ,
                           ind.norm.data = ind.norm.data        )

    p0[index.DEoptim] = outDEoptim$optim$bestmem
    print("Finished DEoptim")
    t.end=proc.time()          # save timing
    print(paste0("DEoptim took: ",(t.end[3]-t.beg[3])/60.," min"))
    print(paste0(params," ",p0))
}

if(!is.null(continue.mcmc)){

    # 12/27/17 Forcing-based Hector. Took 170 minutes. DOECLIM DEoptim output.
#   p0[index.DEoptim] = c( 1.91804058912437, 1.53993386594774, 0.520910696383523, 
#                          -0.00949539681064582, -20.1757086003454 )

    # 12/28/17 Emissions-based Hector. Took 170 minutes. DOECLIM DEoptim output.
#    DEoptim.output.str = "Emissions-based Hector, DOECLIM parameters and observation constraints"
#    p0[index.DEoptim] = c( 2.01823906164744, 2.99881682965184, 0.50315986159485, 
#		         0.0289778671684981, -27.8713241773148 )

    print(paste0("Skipping DEoptim, continuing first chain from ",continue.mcmc) )
    load(continue.mcmc)
    p0 = unconverged_chain_list[[1]][length(unconverged_chain_list[[1]][,1]),]
    print(paste0(params," ",p0))
}
##==============================================================================

## MCMC calibration
## + Cite Metropolis et al (1953) and Hasting (1973) for any Metropolis-Hastings
## + Also cite Vihola (2012) if you use "adaptMCMC" library's "MCMC" with adapt=TRUE
## + log.post is in the 'hector_assimLikelihood.R' module, and defines
##   the statistical model for calibration

## Source the statistical model
source('hector_assimLikelihood.R')

accept.mcmc = 0.234   		     # Optimal acceptance rate as # parameters->infinity (Gelman et al, 1996; Roberts et al, 1997)
gamma.mcmc = 0.5	             # rate of adaptation (between 0.5 and 1, lower is faster adaptation)
stopadapt.mcmc = round(niter.mcmc*1) # stop adapting after how long (if ever)?

if( !parallel.mcmc ){
  print("Starting MCMC chain.")
  t.beg=proc.time()
  amcmc.out1 = MCMC( log.post                     , niter.mcmc                  , 
		     p0                           , scale = step.mcmc           , 
		     adapt = TRUE                 , acc.rate = accept.mcmc      ,
  		     gamma = gamma.mcmc           , list = TRUE                 , 
		     n.start = round(0.01*niter.mcmc),
		     parnames.in = parnames       , in.hector.in = in.hector    , 
		     sections.in = sections       , chain.str = "chain1"        , 
		     calib.folder = calib.folder  , forcing.in = forcing        ,
		     ini.template = ini.template  , output.vars = output.vars   ,
		     output.components = output.components,
		     mod.time = mod.time          , l.project = l.project       ,
		     bound.lower.in = bound.lower , bound.upper.in = bound.upper,
	             oidx = oidx, midx = midx     , obs = obs, obs.err = obs.err,
		     ind.norm.data = ind.norm.data)
  t.end=proc.time()											# save timing
  chain1 = amcmc.out1$samples
  print(paste0(niter.mcmc," runs took: ",(t.end[3]-t.beg[3])/60.," min"))
}

## If you want to run 2 (or more) chains in parallel (save time, more sampling)
if( parallel.mcmc ){	  #Not sure how to do this for Hector, not implemented for now
  t.beg=proc.time() # save timing (running millions of iterations so best to have SOME idea...)
  print( paste0( "Starting ", nparallel.mcmc, " parallel MCMC chains." ) )
  amcmc.par1 = MCMC.parallel( log.post                     , niter.mcmc                      , 
			      p0                           , n.chain = nparallel.mcmc        , 
			      n.cpu = nparallel.mcmc       , dyn.libs='hectorwrapper.R'      ,
  			      scale = step.mcmc            , adapt = TRUE                    , 
			      acc.rate = accept.mcmc       , gamma = gamma.mcmc              , 
			      list = TRUE                  , n.start = round(0.01*niter.mcmc),
                              parnames.in = parnames       , in.hector.in = in.hector        ,
                              sections.in = sections       , chain.str = "chainparallel"     ,
                              calib.folder = calib.folder  , forcing.in = forcing            ,
                              ini.template = ini.template  , output.vars = output.vars       ,
			      output.components = output.components, 
                              mod.time = mod.time          , l.project = l.project           ,
                              bound.lower.in = bound.lower , bound.upper.in = bound.upper    ,
                              oidx = oidx, midx = midx     , obs = obs, obs.err = obs.err    ,
                              ind.norm.data = ind.norm.data)
  t.end=proc.time() # save timing
  print(paste0(nparallel.mcmc, "parallel chains of ", niter.mcmc," runs took: ",(t.end[3]-t.beg[3])/60.," min"))
  if( nparallel.mcmc == 1 ){
    chain1 = amcmc.par1[[1]]$samples
  }
}

##==============================================================================
## Check for convergence
## Check #1: History plots
pdf( paste0( calib.folder, "/chain1_hector.pdf" ) )
par(mfrow=c(3,3))
for ( pp in 1:length(parnames) ) {
	plot( chain1[,pp], type = "l", ylab = parnames[pp], xlab = "Number of Runs", main = "")
}
dev.off()

## Check #2: Heidelberger and Welch's convergence diagnostic:
heidel.diag(chain1, eps=0.1, pvalue=0.05)

if( ( !parallel.mcmc | (nparallel.mcmc == 1) ) & gr.mcmc ){ #this section needed if we only did one chain earlier and want to calc GR
## Check #3: Gelman and Rubin's convergence diagnostic:
# Converged when the potental scale reduction factor is less than 1.1
  print("Starting 2nd MCMC chain (needed for Gelman Rubin burn-in diagnostic calculation).")
  set.seed(222)
  t.beg2 = proc.time()
  amcmc.out2 = MCMC( log.post                     , niter.mcmc                  ,
                     p0                           , scale = step.mcmc           ,
                     adapt = TRUE                 , acc.rate = accept.mcmc      ,
                     gamma = gamma.mcmc           , list = TRUE                 ,
                     n.start = round(0.01*niter.mcmc),
                     parnames.in = parnames       , in.hector.in = in.hector    ,
                     sections.in = sections       , chain.str = "chain2"        ,
                     calib.folder = calib.folder  , forcing.in = forcing        ,
                     ini.template = ini.template  , output.vars = output.vars   ,
		     output.components = output.components,
                     mod.time = mod.time          , l.project = l.project       ,
                     bound.lower.in = bound.lower , bound.upper.in = bound.upper,
                     oidx = oidx, midx = midx     , obs = obs, obs.err = obs.err,
                     ind.norm.data = ind.norm.data)
  t.end2 = proc.time()  # save timing
  print( paste0( niter.mcmc, " runs took: ", (t.end2[3]-t.beg2[3])/60., " min"))
  chain2 = amcmc.out2$samples

  # Reset seed back to original
  set.seed(1234)
}

##==============================================================================

## Determine when (in increments of ??,000 iterations, using Gelman and Rubin
## diagnostic) the two parallel chains (assumed to be
## chain1=amcmc.par1[[1]]$samples and chain2=amcmc.par1[[2]]$samples
## as defined above in the MCMC.parallel(...) command) converge.
## Initialize the testing of the Gelman and Rubin diagnostics
gr.step = floor( niter.mcmc / 100 )
niter.test = seq( from = max(gr.step, 100), to = niter.mcmc, by = gr.step) #start w/ too short a sample and we get an error
gr.test = rep( NA, length(niter.test) )
gr.max <- 1.1 #max statistic value for the rest of the chain to be considered converged

## Calculate the statistic at a few spots throughout the chain. Once it is
## close to 1 (people often use GR<1.1 or 1.05), the between-chain variability
## is indistinguishable from the within-chain variability, and they are
## converged. It is only after the chains are converged that you should use the
## parameter values as posterior draws, for analysis.
if( !gr.mcmc ) {
  print('Not using GR; cutting off first half for burn-in instead of doing GR diagnostics')
} else if( parallel.mcmc & (nparallel.mcmc > 1) ) {
  # these cases are FAR more fun
  # accumulate the names of the soon-to-be mcmc objects
  string.mcmc.list <- 'mcmc1'
  for (m in 2:nparallel.mcmc) {
    string.mcmc.list <- paste(string.mcmc.list, ', mcmc', m, sep='')
  }
  for (i in 1:length(niter.test)) {
    for (m in 1:nparallel.mcmc) {
      # convert each of the chains into mcmc object
      eval(parse(text=paste('mcmc',m,' <- as.mcmc(amcmc.par1[[m]]$samples[1:niter.test[i],])', sep='')))
    }
    eval(parse(text=paste('mcmc_chain_list = mcmc.list(list(', string.mcmc.list , '))', sep='')))
    gr.test[i] <- as.numeric(gelman.diag(mcmc_chain_list)[2])
  }
} else if( !parallel.mcmc | (nparallel.mcmc == 1) ) {
  for (i in 1:length(niter.test)) {
    mcmc1 <- as.mcmc( chain1[1:niter.test[i],] )
    mcmc2 <- as.mcmc( chain2[1:niter.test[i],] )
    mcmc_chain_list = mcmc.list( list(mcmc1, mcmc2) )
    gr.test[i] <- as.numeric(gelman.diag(mcmc_chain_list)[2])
  }
} else {print('error - nparallel.mcmc < 1 makes no sense')}


## Plot GR statistics as a function of iterations
if(gr.mcmc){
    pdf( paste0( calib.folder, "/gr_stat_hector.pdf" ) )
    plot(niter.test,gr.test)
    abline(h=1.1,lty=2)
    dev.off()
}

#===============================================================================
# Chop off burn-in
#===============================================================================
#

# Note: here, we are only using the Gelman and Rubin diagnostic. But this is
# only after looking at the quantile stability as iterations increase, as well
# as the Heidelberger and Welch diagnostics, which suggest the chains are okay.
# 'ifirst' is the first spot where the GR stat gets to and stays below gr.max
# for all of the models.
# save a separate ifirst for each experiment
ifirst <- NULL
if( !gr.mcmc ) {
  ifirst <- round(0.5*niter.mcmc)
} else {
  lgr <- rep(NA, length(niter.test))
  for (i in 1:length(niter.test)) {lgr[i] <- gr.test[i] < gr.max}
  for (i in seq(from=length(niter.test), to=1, by=-1)) {
    if( all(lgr[i:length(lgr)]) ) {ifirst <- niter.test[i]}
  }
  if ( is.null(ifirst) ) {
    print( "Unable to find convergence w/ GR diagnostic." )
    chains_file = paste0(calib.folder, "/unconverged_chain_list.RData")
    unconverged_chain_list = mcmc_chain_list
    save(unconverged_chain_list, file = chains_file)
    print(paste0("MCMC list saved to ", chains_file))
    stop(paste0("Could increase niter.mcmc and/or try continuing from the end of the first chain with --continue ",chains_file))
  }
}

if( parallel.mcmc & (nparallel.mcmc > 1) ) {
  chains_burned <- vector( 'list', nparallel.mcmc )
  for (m in 1:nparallel.mcmc) {
    chains_burned[[m]] <- amcmc.par1[[m]]$samples[(ifirst+1):niter.mcmc,]
  }
} else if( gr.mcmc ) {
  chains_burned <- vector( 'list', 2 )
  chains_burned[[1]] <- chain1[(ifirst+1):niter.mcmc,]
  chains_burned[[2]] <- chain2[(ifirst+1):niter.mcmc,]
} else {
  chains_burned <- vector( 'list', 1 )
  chains_burned[[1]] <- chain1[(ifirst+1):niter.mcmc,]
}
  
# Combine all of the chains from 'ifirst' to 'niter_mcmc' into a potpourri of
# [alleged] samples from the posterior.
parameters.posterior <- chains_burned[[1]]
if( length( chains_burned ) > 1 ) {
  for (m in 2:length ( chains_burned ) ) {
    parameters.posterior <- rbind(parameters.posterior, chains_burned[[m]])
  }
}
  
# Only saving the transition covariance matrix for one of the chains (if in parallel).
if( parallel.mcmc ) {
  covjump.posterior <- amcmc.par1[[1]]$cov.jump
} else {
  covjump.posterior <- amcmc.out1$cov.jump
}
  
n.parameters = ncol(parameters.posterior)

## Histograms
pdf( paste0( calib.folder, "/posterior_hist_hector.pdf" ) )
par(mfrow=c(3,3))
for ( pp in 1:length(parnames) ) {
	hist( parameters.posterior[,pp], xlab = params[pp], main='')
}
dev.off()

## Fit PDFs to the parameter distributions
pdf.all=vector('list',n.parameters)
n.node=200
for (pp in 1:n.parameters){
  tmp = density(parameters.posterior[,pp],kernel='gaussian',
                n=n.node,from=bound.lower[pp],to=bound.upper[pp])
  pdf.all[[pp]] = tmp; names(pdf.all)[pp]=params[pp]
}

##==============================================================================
## Plot the PDFs
pdf( paste0( calib.folder, "/posterior_pdf_hector.pdf" ) )
par(mfrow=c(3,3))
for ( pp in 1:n.parameters ){
  plot( pdf.all[[pp]]$x, pdf.all[[pp]]$y, type='l', xlab=params[pp], ylab="Density" );
}
dev.off()
##==============================================================================

## Write a CSV file with the successful parameter combinations
## Structure of the CSV file is as follows. n.parameters columns, 
##   (# of chains) x (# of burnt-in parameter sets / chain ) rows.
##     First row: Parameter names.
##     Rows 2-??: The calibrated parameter values.

to.file = parameters.posterior
rownames(to.file) = NULL
colnames(to.file) = parnames
filename = paste0( calib.folder, "/hector_calibrated_MCMC_parameters.csv")
write.table( to.file, file = filename, sep = ",", qmethod = "double", row.names = FALSE )

print( paste0( "Calibration completed. ", length(parameters.posterior[,1]), 
               " posterior parameter sets saved to ", filename ) )
post_medians = rep( NA, n.parameters )
for (pp in 1:n.parameters){
  post_medians[pp] = median( parameters.posterior[,pp] )
}
print( "Hector parameter posterior medians: " )
print( params[in.hector] )
print( post_medians[in.hector] ) 
##==============================================================================
## End
##==============================================================================
