## This script generates the comparison data used in "test-old_new" unit testing.
## The objective of the test is to do a bit for bit comparison, to make sure that "
## patch" and "minor" Hector PRs do not impact Hector results. However, when there
## is an expected "major" Hector development there may be an expected change in Hector behavior.
## If/when this is case use this script to generate new comparison data.

# Since this script will only be run occasionally while actively developing Hector, use
# devtools::load_all() instead of loading the built Hector package.
devtools::load_all()

# Create the output directory.
out_dir <- here::here("tests", "testthat", "compdata")
dir.create(out_dir, showWarnings = FALSE)

# Determine the version of Hector and the git commit, that generates the comparison data.
# This will be saved to help keep track of the last time the comparison data
# was actually updated.
hector_version <- packageVersion("hector")
hector_commit <- system("git rev-parse --short HEAD", intern = TRUE)


vars_to_save <- c(ATMOSPHERIC_CO2(), GLOBAL_TEMP(), RF_TOTAL(), RF_CO2(), HEAT_FLUX())
dates_to_save <- 1750:2300


# During this old new test we will look at results for two scenarios, a concentration and
# emission driven runs.

# The first scenario to run is the constrained RCP 4.5
ini <- here::here("inst", "input", "hector_rcp45_constrained.ini")
hc <- newcore(ini)
run(hc, max(dates_to_save))
hector_rcp45_constrained <- fetchvars(hc, scenario = basename(ini), dates = dates_to_save, vars = vars_to_save)
hector_rcp45_constrained$version <- hector_version # Add the Hector version.
hector_rcp45_constrained$commit <- hector_commit

# Second run.
ini <- here::here("inst", "input", "hector_rcp45.ini")
hc <- newcore(ini)
run(hc, max(dates_to_save))
hector_rcp45 <- fetchvars(hc, scenario = basename(ini), dates = dates_to_save, vars = vars_to_save)
hector_rcp45$version <- hector_version # Add the Hector version.
hector_rcp45$commit <- hector_commit

# Save the comparison data for the unit tests.
comp_data <- rbind(hector_rcp45_constrained, hector_rcp45)
out_file <- file.path(out_dir, "hector_comp.csv")
write.csv(comp_data, file = out_file, row.names = FALSE)
