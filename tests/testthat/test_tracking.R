context("Carbon tracking")


inputdir <- system.file("input", package = "hector")
tracking_testfile <- file.path(inputdir, "hector_rcp45_tracking.ini")
notracking_testfile <- file.path(inputdir, "hector_rcp45.ini")

# THINGS WE WANT TO TEST

test_that("No-tracking run produces empty data frame", {

    # First Hector core run
    hc <- newcore(notracking_testfile, name = "test", suppresslogging = TRUE)

    # A run without tracking should return an empty data frame
    run(hc, 2100)
    x <- get_tracking_data(hc)
    expect_identical(x, data.frame())

    shutdown(hc)
})


test_that("get_tracking_data handles bad inputs", {
    # error produced if we call it without a proper core
})

test_that("Tracking run produces correct data", {
    # tracking data (from get_tracking_data()) is a data.frame
    # tracking d.f. has particular columns (date, pool, etc)
    # expect_identical(names(x), c("...", "..."))
    # tracking data frame has correct dates (not less than trackingDate, not more than end of run, includes all years)
    # all year/pool combinations sum to ~1
})

test_that("Tracking works with a core reset", {
    # (reset works - no error is produced)
    # after reset to <trackingDate, tracking data frame is empty
    # after reset to >=trackingDate, tracking data frame has correct dates
    # run-reset-run, tracking data is correct
})

test_that("Turning tracking on and off from R works", {
    # can't do this yet
})
