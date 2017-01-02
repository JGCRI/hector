# 1-generate_paramdata.R
# Generate samples for each Hector parameter that's going to be sampled
# The resulting `vardata` and `rundata` files are applied to EACH SCENARIO
# Ben Bond-Lamberty January 2016

source("0-functions.R")  # support functions

SCRIPTNAME  	<- "1-generate_paramdata.R"
PROBLEM       <- FALSE

# The S (climate sensitivity) distribution we generate will be 
# made using the following possible values (using `seq`)
SVALS_MIN <- 0.001 # i.e. don't generate any S smaller than this
SVALS_MAX <- 15    # i.e. don't generate any S larger than this
SVALS_N <- 10000


# -----------------------------------------------------------------------------
# Equation 3 from Roe and Baker (2007) in Science -prob. dens. of S @ given T
hT_roebaker <- function(DT, DT0, fbar, sigma) { 
  (1 / (sigma * sqrt(2 * pi)) * DT0 / DT ^ 2 * 
     exp(-1/2 * ((1 - fbar - DT0 / DT) / sigma) ^ 2))
} # hT_roebaker


# ==============================================================================
# Main 

openlog(file.path(outputdir(), paste0(SCRIPTNAME, ".log.txt")), sink = TRUE)

printlog("Welcome to", SCRIPTNAME)
printlog("Random seed =", RANDOM_SEED)
set.seed(RANDOM_SEED)

S_params <- read_csv("Climate_sensitivity_distributions.csv", skip = 3)

n_samples <- read_csv(RUNSET_FILE)$n_samples %>% mean()

if(file.exists(file.path(VARDATA_FILE))) {
  printlog("Removing existing", VARDATA_FILE)
  file.remove(VARDATA_FILE)
}
if(file.exists(file.path(RUNDATA_FILE))) {
  printlog("Removing existing", RUNDATA_FILE)
  file.remove(RUNDATA_FILE)
}

vardata <- list()

# Generate as many distributions for S as we have parameters, make `n_samples`
# random draws from that distribution, and write output files
for(i in seq_len(nrow(S_params))) {
  printlog(SEPARATOR, i)
  
  s_mean <- S_params$mean[i]
  s_sd <- S_params$SD[i]
 
  S_vals <- seq(SVALS_MIN, SVALS_MAX, length.out = SVALS_N)
  S_pdf <- hT_roebaker(S_vals, DT0 = 1.2, s_mean, s_sd)  # 1.2 C, constant from paper
  S_vals <- sample(x = S_vals, n_samples, replace = TRUE, prob = S_pdf)  %>%
    round(3)
  vardata[[i]] <- data.frame(temperature.S2005 = S_vals, param_set = i)
}


# ------------ Make final data sets, write out, and visualize

printlog(SEPARATOR)
printlog("Collapsing vardata into a single data frame...")
vardata <- rbind_all(vardata)
print_dims(vardata)

printlog("Writing", VARDATA_FILE)
vardata$run_number <- seq_len(nrow(vardata))
select(vardata, -param_set, -run_number) %>%
  write_csv(VARDATA_FILE)

printlog("Making rundata...")
rundata <- S_params %>%
  mutate(param_set = seq_len(nrow(S_params))) %>%
  select(mean, SD, param_set) %>%
  left_join(vardata, by = "param_set")
printlog("Writing", RUNDATA_FILE)
write_csv(rundata, RUNDATA_FILE)

rundata$set <- paste0("(µ=", rundata$mean, ", sd=", rundata$SD, ")")
p <- ggplot(rundata, aes(temperature.S2005, color = set)) + geom_density()
p <- p + ggtitle(RUNSET_NAME) + xlab("S")
p <- p + scale_color_discrete("")
print(p)
save_plot(paste0(RUNSET_NAME, "_S_value_densities"))

# ------------ Done

printlog(SEPARATOR)
printlog("All done with", SCRIPTNAME)
closelog()

if(PROBLEM) warning("There was a problem - see log")
