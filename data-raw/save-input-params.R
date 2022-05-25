# Since this script will only be run occasionally while actively developing Hector, use
# devtools::load_all() instead of loading the built Hector package.
devtools::load_all()

# Read in the input parameter csv file
inputstable <- read.csv("./data-raw/input_params.csv")

# Save as an object in the package
usethis::use_data(inputstable, internal = FALSE, overwrite = TRUE)

