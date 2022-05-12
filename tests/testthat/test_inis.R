context("Test ini files")

# Make sure that all of the ini files can generate an actual hector core.
test_that("All ini files can be used to set up a hector core", {
  ini_list <- list.files(system.file(package = "hector", "input"), pattern = "ini")

  for (ini in ini_list) {
    hc <- isactive(newcore(system.file(package = "hector", file.path("input", ini))))
    expect_true(hc)
  }
})

# Check that all parameters found in an ini file match the input_params csv...
test_that("All ini params are in the input csv", {
  # Read in ini files
  ini_list <- list.files(system.file(package = "hector", "input"), pattern = "ini")
  ini_file <- readLines(system.file("input", ini_list[[1]], package = "hector"))

  # Break up file line-by-line
  split_ini <- strsplit(ini_file, ";")

  # Read in comparison csv, identify unique parameter names
  input_table <- hector:::input_csv
  params_to_match <- unique(input_table$parameter)

  # For each parameter, loop over the line-by-line file to check for a match
  # Store unique results - if variable is present somewhere, we expect c(FALSE, TRUE)
  # If the variable is not present, we expect just FALSE
  test <- list()
  for(n in seq_len(length(params_to_match))){
    test[[n]] <- unique(grepl(params_to_match[[n]], split_ini))
  }

  # Extract results, store as data frame
  # Create a column to check if the parameter row contains TRUE, then filter out those columns
  results <- as.data.frame(t(rbind(test)))
  results$t <- grepl(TRUE, results$test)
  results_true <- subset(results, t == TRUE)

  # Then, we expect that the number of rows containing TRUE would equal the number
  # of parameters we are checking against
  expect_equal(nrow(results_true), length(params_to_match))


})

# ...and that all parameters in the csv match an ini file
