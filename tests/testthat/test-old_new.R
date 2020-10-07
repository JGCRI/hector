context('Old new tests')
# Verify that hector output for the canonical rcps matches output from older version. This "bit for bit"
# comparison of the output it intended to help preserve Hector data during minor devlopment.
# For behavior changing devlopment the comparison data will need to be updated with
# data-raw/generate-comp-data.R

test_that("Hector output passes old new test", {

    # In order to pass the old new test the mean aboslute difference between
    # the old and new  hector output must be less than the defined threshold.
    error_threshold <- 1e-10

    # Read in the comparison data and extract the information to save.
    comp_data <- read.csv(list.files('compdata', '.csv',  full.names = TRUE), stringsAsFactors = FALSE)
    vars <- unique(comp_data$variable)
    time <- unique(comp_data$year)
    scenarios <- unique(comp_data$scenario)

    # Run Hector
    new_data <- do.call(rbind, lapply(scenarios, function(scn){

        ini <- system.file(file.path('input', scn), package = 'hector')
        hc  <- newcore(ini, name = scn)
        run(hc, runtodate = max(time))
        fetchvars(core = hc, dates = time, vars = vars)

    }))

    # Hector data should return the same amount of data and the values returned from
    # the data should be identical (if there is a difference it should be less than the
    # error_threshold.)
    expect_equivalent(nrow(comp_data), nrow(new_data))
    expect_equal(comp_data$value, new_data$value, tolerance = error_threshold)

})
