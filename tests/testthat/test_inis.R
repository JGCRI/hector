context("Test ini files")

# Make sure that all of the ini files can generate an actual hector core.
test_that("All ini files can be used to set up a hector core", {
  ini_list <- list.files(system.file(package = "hector", "input"), pattern = "ini")

  for (ini in ini_list) {
    hc <- isactive(newcore(system.file(package = "hector", file.path("input", ini))))
    expect_true(hc)
  }
})

# Check that all parameters found the input_params csv are in the ini files...
test_that("All csv params are in an ini file...", {
  # Read in ini files
  ini_list <- list.files(system.file(package = "hector", "input"), pattern = "ini")

  expect_warning(
    for(ini in ini_list){
      ini_file <- readLines(system.file("input", ini, package = "hector"))

      # Read in comparison csv, identify unique parameter names
      input_table <- hector:::input_csv
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
      missing_param <- setdiff(input_params, ini_params)

      # If there are extra or missing parameters, issue a warning
      if(length(missing_param) > 0) warning(paste0("\nParameter ", missing_param, " is failing in ", ini, "."))


    }, regexp = NA)

})

# ...and that all parameters in the ini files are in the csv
test_that("All ini parameters are in the input csv", {

  # Read in ini files
  ini_list <- list.files(system.file(package = "hector", "input"), pattern = "ini")

  expect_warning(
    for(ini in ini_list){
      ini_file <- readLines(system.file("input", ini, package = "hector"))

      # Read in comparison csv, identify unique parameter names
      input_table <- hector:::input_csv
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
      if(length(missing_param) > 0) warning(paste0("\nParameter ", missing_param, " is failing in ", ini, "."))

    }, regexp = NA)


})
