context("Carbon tracking")

inputdir <- system.file("input", package = "hector")
tracking_testfile <- file.path(inputdir, "hector_rcp45_tracking.ini")
notracking_testfile <- file.path(inputdir, "hector_rcp45.ini")

# convenience function
trackingcore <- function() {
    newcore(tracking_testfile, name = "test", suppresslogging = TRUE)
}

error_threshold <- 1e-8

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

    # Error produced if we call get_tracking_data() without a proper core
    expect_error(get_tracking_data("core"))
})

test_that("Tracking run produces correct data", {

    # Test that tracking data (from get_tracking_data()) is a data.frame

    # Run core and get tracking data, then test that the class is data.frame
    core <- trackingcore()
    run(core, 2100)

    df <- get_tracking_data(core)
    expect_s3_class(df, "data.frame")

    expect_identical(names(df), c("year", "pool_name", "pool_value",
                                 "pool_units", "source_name", "source_fraction"))

    # Test that tracking data frame has correct dates
    # (not less than trackingDate, not more than end of run, includes all years)
    track_date <- core$trackdate
    end_date <- core$enddate

    # Find the min and max of the data.frame, which should be the track_date
    # and end_date, respectively. Check that the minimum is at least the
    # track_date and the maximum is no higher than the end_date.
    data_min <- min(df$year)
    data_max <- max(df$year)

    expect_gte(data_min, track_date)
    expect_lte(data_max, end_date)

    # Test that all year/pool combinations sum to ~1

    # Calculate the total source fraction (which should be equal to 1)
    # from the sum of the individual source fractions.
    individual_source <- fetchvars(core, vars = df$source_fraction, dates = "NA")
    sum_source <- aggregate(value ~ year + source_fraction,
                                data = individual_source,
                                FUN = "sum")

    expect_identical(1, sum_source$value, tolerance = error_threshold)

    shutdown(core)
})

test_that("Tracking works with a core reset", {

    # Test that reset() works - no error is produced

    # Run a core, reset, and make sure no error is produced.
    core <- trackingcore()
    run(core, 2100)

    a <- reset(core)
    expect_error(a, NA)

    shutdown(core)

    # Test that after reset to <trackingDate>, tracking data frame is empty

    # Run a core and reset to the start date. Check that the
    # get_tracking_data data.frame is identical to an empty one.
    core <- trackingcore()
    run(core, 2100)

    reset(core, date = core$strtdate)

    df <- get_tracking_data(core)
    expect_identical(df, data.frame())

    shutdown(core)

    # Test that after reset to >=trackingDate, tracking data frame has correct dates

    # Run a core, and assign the start, end, and track dates to varibles.
    core1 <- trackingcore()
    run(core1, 2100)

    start_date <- core1$strtdate
    end_date <- core1$enddate
    track_date <- core1$trackdate

    # Reset the core, then confirm that the new core's start, end,
    # and tracking dates are identical to the original core's.
    core2 <- reset(core1)

    df <- get_tracking_data(core2)
    expect_identical(core2$strtdate, start_date)
    expect_identical(core2$enddate, end_date)
    expect_identical(core2$trackdate, track_date)

    shutdown(core2)

    # Test that for a run-reset-run, tracking data is correct

    # Run a core and assign the tracking date to a variable.
    # Reset the core, run a new core, and check that the tracking
    # date is the same as the original core's.
    core1 <- trackingcore()
    run(core1, 2100)
    track_date <- core1$trackdate

    reset(core1)

    core2 <- trackingcore()
    expect_identical(core$trackdate, track_date)

    shutdown(core2)

})

test_that("Turning tracking on and off from R works", {
    # can't do this yet
})
