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

  for(ini in ini_list){
    ini_file <- readLines(system.file("input", ini, package = "hector"))

    # Break up file line-by-line
    split_ini <- strsplit(ini_file, ";")

    # Read in comparison csv, identify unique parameter names
    input_table <- hector:::input_csv
    input_params <- unique(input_table$parameter)

    # For each parameter, loop over the line-by-line file to check for a match
    # Store unique results - if variable is present somewhere, we expect c(FALSE, TRUE)
    # If the variable is not present, we expect just FALSE
    test <- list()
    for(n in seq_len(length(input_params))){
      test[[n]] <- unique(grepl(input_params[[n]], split_ini))
    }

    # Extract results, store as data frame
    # Create a column to check if the parameter row contains TRUE, then filter out those columns
    results <- as.data.frame(t(rbind(test)))
    results$t <- grepl(TRUE, results$test)
    results_true <- subset(results, t == TRUE)

    # Then, we expect that the number of rows containing TRUE would equal the number
    # of parameters we are checking against
    expect_equal(nrow(results_true), length(input_params))

  }

})

# ...and that all parameters in the ini files are in the csv
test_that("All ini parameters are in the input csv", {

  # Read in ini files
  ini_list <- list.files(system.file(package = "hector", "input"), pattern = "ini")

  for(ini in ini_list){
    ini_file <- readLines(system.file("input", ini, package = "hector"))

    # Break up file line-by-line
    split_ini <- strsplit(ini_file, ";")

    # Subset lines with equal sign (identify line as parameter)
    test <- list()
    for(n in 1:length(split_ini)) {
      test[[n]] <- subset(split_ini[[n]], grepl("=", split_ini[[n]]))
      # If the line in the ini does not contain an = or if it contains more than
      # one word, it is not a parameter and should be discarded
      if(length(test[[n]]) == 0 | length(test[[n]]) > 1) test[[n]] <- NA
    }

    # Get results, drop NAs
    results <- as.data.frame(t(rbind(test)))
    results <- subset(results, is.na(results$test) == FALSE)

    # Split up lines before and after the = sign
    split_params <- list()
    for(n in 1:nrow(results)) {
      split_params[[n]] <- strsplit(results$test[[n]], "=")
    }

    # Get results
    split_param <- as.data.frame(t(rbind(unlist(split_params))))


  }


})
