context("Test ini files")

ini_list <- list.files(system.file(package = "hector", "input"), pattern = "ini")
input_table <- hector::inputstable

# Make sure that all of the ini files can generate an actual hector core.
test_that("All ini files can be used to set up a hector core", {
  for (ini in ini_list) {
    hc <- isactive(newcore(system.file(package = "hector", file.path("input", ini))))
    expect_true(hc)
  }
})



# Check that all *required* parameters in the input_params csv are in all ini files
# except for idealized runs.
scns_inis_list <- ini_list[grepl(pattern = "ssp", x = ini_list)]
test_that("All required csv params are in all ini files...", {
  # Identify required unique parameter names
  required <- subset(input_table, required == "y")
  input_params <- sort(unique(required$parameter))

  expect_warning(
    for (ini in scns_inis_list) {
      ini_file <- readLines(system.file("input", ini, package = "hector"))

      # Pull out lines with = sign, meaning parameters and definitions
      eq <- ini_file[grepl("=", ini_file)]

      # Function to extract the string before the equal sign, the parameter name
      split_equal <- function(str) {
        x <- strsplit(str, "=")
        unlist(x)[[1]]
      }

      param <- unlist(lapply(eq, split_equal))

      # Remove symbols
      ini_params <- gsub(";|\\[1750]| ", "", x = param)

      # Compare the two lists of parameters
      missing_param <- setdiff(input_params, ini_params)

      # If there are extra or missing parameters, issue a warning
      if (length(missing_param) > 0) {
        warning(paste0(
          "\nRequired parameter ",
          missing_param, " is not present in ", ini, "."
        ))
      }
    },
    regexp = NA
  )
})

# ...and that all parameters in the ini files are in the input_params csv
test_that("All ini parameters are in the input csv", {
  expect_warning(
    for (ini in scns_inis_list) {
      ini_file <- readLines(system.file("input", ini, package = "hector"))

      # Read in comparison csv, identify unique parameter names
      input_params <- sort(unique(input_table$parameter))

      # Pull out lines with = sign, meaning parameters and definitions
      eq <- ini_file[grepl("=", ini_file)]

      # Function to extract the string before the equal sign, the parameter name
      split_equal <- function(str) {
        x <- strsplit(str, "=")
        unlist(x)[[1]]
      }

      param <- unlist(lapply(eq, split_equal))

      # Remove symbols
      ini_params <- gsub(";|\\[1750]| ", "", x = param)

      # Compare the two lists of parameters
      missing_param <- setdiff(ini_params, input_params)

      # If there are extra or missing parameters, issue a warning
      if (length(missing_param) > 0) {
        warning(paste0(
          "\nParameter ",
          missing_param, " is failing in ", ini, "."
        ))
      }
    },
    regexp = NA
  )
})



# Check that there is no signal in the pi control
test_that("picontrol",{

    pi_ini <- ini_list[grepl(pattern = "picontrol", x = tolower(ini_list))]
    hc <- newcore(system.file(package = "hector", file.path("input", pi_ini)))

    run(hc)

    # Fetch variables that should be constant over the course of the run
    dates <- 1750:2100
    temp <- fetchvars(hc, dates, vars = GLOBAL_TAS())
    total_rf <- fetchvars(hc, dates, vars = RF_TOTAL())
    ch4_conc <- fetchvars(hc, dates, vars = CONCENTRATIONS_CH4())

    expect_lte(object = sd(temp$value), expected = 1e-4)
    expect_lte(object = sd(total_rf$value), expected = 1e-4)
    expect_lte(object = sd(ch4_conc$value), expected = 1e-4)

})


