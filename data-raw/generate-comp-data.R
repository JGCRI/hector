## This script generates the comparison data that is used in the old new tests (a bit by bit)
## comparison. Whenever there is devlopment that should change Hector's output this script
## should to generate new comparison data.
## Since this script will only while developing the package devtools::load_all() and
## file paths that are relative to the hector project instead of built package.

# Load the Hector package
devtools::load_all()

# Create the output directory.
out_dir <- here::here('tests', 'testthat', 'compdata')
dir.create(out_dir, showWarnings = FALSE)

# Determine the version of Hector that will generate the comparison data.
# This will be saved to help keep track of the last time the comparison data
# was actually updated.
session_info   <- sessionInfo(package = 'hector')
hector_version <- session_info$otherPkgs$hector$Version


vars_to_save  <- c(ATMOSPHERIC_CO2(), GLOBAL_TEMP(), RF_TOTAL(), RF_CO2(), HEAT_FLUX())
dates_to_save <- 1750:2300


# During this old new test we will look at results for two scenarios, a concentration and
# emission driven runs.

# The first scenario to run is the constrained RCP 4.5
ini <- here::here('inst', 'input', 'hector_rcp45_constrained.ini')
hc  <- newcore(ini)
run(hc, 2300)
hector_rcp45_constrained <- fetchvars(hc, scenario = basename(ini), dates = dates_to_save, vars = vars_to_save)
hector_rcp45_constrained$version <- hector_version  # Add the Hector version.

# Second run.
ini <- here::here('inst', 'input', 'hector_rcp45.ini')
hc  <- newcore(ini)
run(hc, 2300)
hector_rcp45 <- fetchvars(hc, scenario = basename(ini), dates = dates_to_save, vars = vars_to_save)
hector_rcp45$version <- hector_version  # Add the Hector version.


# Save the comparison data for the unit tests.
comp_data <- rbind(hector_rcp45_constrained, hector_rcp45)
out_file  <- file.path(out_dir, 'hector_comp.csv')
write.csv(comp_data, file = out_file, row.names = FALSE)


