context("Carbon tracking")

inputdir <- system.file("input", package = "hector")
testvars <- c(ATMOSPHERIC_CO2(), RF_TOTAL(), GLOBAL_TEMP())


test_that("No-tracking run produces empty data frame", {

    # First Hector core run
    hc <- newcore(file.path(inputdir, "hector_rcp60.ini"), name = "RCP45", suppresslogging = TRUE)
# TODO: ensure no tracking

    # A run without tracking should return an empty data frame
    run(hc, 2100)
    x <- get_tracking_data(hc)
    expect_identical(x, data.frame())

    shutdown(hc)
})

# tracking data frame has correct dates
# all year/pool combinations sum to ~1
# after reset, tracking data frame has correct dates

# OK, how do we handle tracking date changes in R?
# a set_tracking_date function
# as a parameter to run() (which then uses setData)
# as a parameter to core::run()
# via setvar()   <- this seems the best way?
