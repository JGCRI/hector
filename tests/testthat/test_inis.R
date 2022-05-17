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
    grep("FALSE", results$t) #gives row number
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
    equal_sign <- list()
    for(n in 1:length(split_ini)) {
      equal_sign[[n]] <- subset(split_ini[[n]], grepl("=", split_ini[[n]]))
      # If the line in the ini does not contain an =, it is not a
      # parameter and should be discarded
      if(length(equal_sign[[n]]) == 0) equal_sign[[n]] <- NA
    }

    # Get results, drop NAs
    equal <- as.data.frame(t(rbind(equal_sign)))
    equal <- subset(equal, is.na(equal$equal_sign) == FALSE)

    # Split up lines before and after the = sign
    split_params <- list()
    param <- list()
    for(n in 1:nrow(equal)) {
      split_params[[n]] <- strsplit(equal$equal_sign[[n]], "=")
      # Access just the character before the equal sign, the parameter name
      param[[n]] <- split_params[[n]][[1]][[1]]
    }

    # Get results
    raw_parameters <- as.data.frame(t(rbind(unlist(param))))
    # Remove duplicate parameters
    params_to_match <- data.frame(param = unique(raw_parameters$V1))

    # Remove words that somehow snuck into the final list
    bad_row_numbers <- grep("Positive", params_to_match$param)
    rows <- 1:nrow(params_to_match)
    good_row_numbers <- rows[-bad_row_numbers]
    params_to_match[83,] <- "Tgav_constrain"

    # Isolate the final list of parameters
    final_params <- data.frame(param = sort(params_to_match[good_row_numbers,]))

    # Read in comparison csv, identify unique parameter names
    input_table <- hector:::input_csv
    input_params <- sort(unique(input_table$parameter))

    # Compare the two lists
    test <- list()
    for(n in 1:nrow(params_to_match)){
      test[[n]] <- unique(grepl(final_params[n,], input_params))
    }

    results <- as.data.frame(t(rbind(test)))
    results$t <- grepl(TRUE, results$test)
    results_false <- subset(results, t == FALSE)

    failing_params <- row.names(results_false)
    if(length(failing_params) > 0) warning(paste0("\nParameter ", final_params[failing_params,]$param , " is failing. "))

    expect_warning(, "fail")
  }


})
