# Tracking tests
# Leeyah Pressburger 2021

context("Carbon tracking")

inputdir <- system.file("input", package = "hector")
inifile <- file.path(inputdir, "hector_rcp45.ini")

error_threshold <- 1e-6


test_that("No-tracking run produces empty data frame", {

    # A run without tracking should return an empty data frame
    hc <- newcore(inifile, name = "test", suppresslogging = TRUE)
    run(hc, 1760)
    x <- get_tracking_data(hc)
    expect_identical(x, data.frame())

    shutdown(hc)
})


test_that("get_tracking_data handles bad inputs", {

    # Error produced if we call get_tracking_data() without a proper core
    expect_error(get_tracking_data("core"))

})

test_that("Changing a parameter changes tracking data", {

    # Run core, get tracking data
    core <- newcore(inifile)
    setvar(core, NA, TRACKING_DATE(), 1900, "")
    run(core, runtodate = 2000)
    df1 <- get_tracking_data(core)

    # ...change a parameter and re-run
    default_Q10 <- fetchvars(core, NA, Q10_RH())
    new_Q10 <- default_Q10$value * 0.9
    setvar(core, NA, Q10_RH(), new_Q10, getunits(Q10_RH()))

    reset(core)
    run(core, runtodate = 2000)
    df2 <- get_tracking_data(core)

    expect_true(nrow(df1) == nrow(df2))
    expect_false(all(df1$source_fraction == df2$source_fraction))

    shutdown(core)
})

test_that("Tracking run produces correct data", {

    # Test that tracking data (from get_tracking_data()) is a data.frame

    # Run core and get tracking data, then test that the class is data.frame
    core <- newcore(inifile)
    setvar(core, NA, TRACKING_DATE(), 1770, "")
    run(core)

    df <- get_tracking_data(core)
    expect_s3_class(df, "data.frame")

    expect_identical(names(df), c("year", "pool_name", "pool_value",
                                 "pool_units", "source_name", "source_fraction"))

    # Test that tracking data frame has correct dates
    # (not less than trackingDate, not more than end of run, includes all years)
    track_date <- fetchvars(core, NA, TRACKING_DATE())$value
    end_date <- core$enddate
    years <- c(track_date:end_date)

    # Find the min and max of the data.frame, which should be the track_date
    # and end_date, respectively. Check that the minimum is at least the
    # track_date and the maximum is no higher than the end_date.
    # Find the non-repeating years within the data.frame, then check
    # that all years defined above are present.
    data_min <- min(df$year)
    data_max <- max(df$year)
    data_years <- unique(df$year)

    expect_gte(data_min, track_date)
    expect_lte(data_max, end_date)
    expect_identical(data_years, years)

    # Test that all year/pool combinations sum to ~1

    # Calculate the total source fraction (which should be equal to 1)
    # from the sum of the individual source fractions.
    df <- subset(df, pool_name != "Diff")
    ag <- aggregate(source_fraction ~ year + pool_name, data = df, FUN = sum)

    ones <- rep(1, times = length(ag$source_fraction))
    expect_equal(ones, ag$source_fraction, tolerance = error_threshold)

    shutdown(core)
})

test_that("Tracking works with a core reset", {

    # Test that reset() works

    # Run a core, reset, and make sure the start date has reset.
    core <- newcore(inifile)
    run(core, 1760)
    start_date <- core$strtdate

    a <- reset(core)
    expect_identical(a$strtdate, start_date)

    # After reset to <trackingDate>, tracking data frame should be empty

    # Run a core and reset to the start date. Check that the
    # get_tracking_data data.frame is identical to an empty one.
    core <- newcore(inifile)
    setvar(core, NA, TRACKING_DATE(), 1760, "")
    run(core, 1770)

    reset(core)

    df <- get_tracking_data(core)
    expect_identical(df, data.frame())

    # Test that after reset to >=trackingDate, tracking data frame has correct dates

    # Run a core, and assign the start, end, and track dates to variables.
    core1 <- newcore(inifile)
    setvar(core1, NA, TRACKING_DATE(), 1760, "")
    run(core1, 1770)

    start_date <- core1$strtdate
    end_date <- core1$enddate
    track_date <- fetchvars(core1, NA, TRACKING_DATE())$value

    # Reset the core, then confirm that the new core's start, end,
    # and tracking dates are identical to the original core's.
    core2 <- reset(core1, date = track_date)
    track_date2 <- fetchvars(core2, NA, TRACKING_DATE())$value

    expect_identical(core2$strtdate, start_date)
    expect_identical(core2$enddate, end_date)
    expect_identical(track_date2, track_date)

    # Test that for a run-reset-run, tracking data is correct

    # Run a core and assign the tracking date to a variable.
    # Reset the core, run a new core, and check that the tracking
    # date is the same as the original core's.
    core1 <- newcore(inifile)
    setvar(core1, NA, TRACKING_DATE(), 1760, "")
    run(core1, 1770)
    track_date <- fetchvars(core1, NA, TRACKING_DATE())$value

    core2 <- reset(core1)
    track_date2 <- fetchvars(core2, NA, TRACKING_DATE())$value
    expect_identical(track_date2, track_date)

    shutdown(core2)
})
