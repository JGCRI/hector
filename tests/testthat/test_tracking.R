# Tracking tests
# Leeya Pressburger 2021

context("Carbon tracking")

inputdir <- system.file("input", package = "hector")
inifile <- file.path(inputdir, "hector_ssp245.ini")

error_threshold <- 1e-6
tunits <- getunits(TRACKING_DATE())

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
    setvar(core, NA, TRACKING_DATE(), 1900, tunits)
    reset(core, core$reset_date)
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
    reset(core, core$reset_date)
    setvar(core, NA, TRACKING_DATE(), 1770, tunits)
    reset(core, core$reset_date)
    run(core)

    df <- get_tracking_data(core)
    expect_s3_class(df, "data.frame")

    expect_identical(names(df), c("year", "component", "pool_name", "pool_value",
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
    setvar(core, NA, TRACKING_DATE(), 1760, tunits)
    reset(core, core$reset_date)
    run(core, 1770)

    reset(core, core$reset_date)

    df <- get_tracking_data(core)
    expect_identical(df, data.frame())

    # Test that after reset to >=trackingDate, tracking data frame has correct dates

    # Run a core, and assign the start, end, and track dates to variables.
    core1 <- newcore(inifile)
    setvar(core1, NA, TRACKING_DATE(), 1760, tunits)
    reset(core1, core1$reset_date)
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
    setvar(core1, NA, TRACKING_DATE(), 1760, tunits)
    reset(core1, core1$reset_date)
    run(core1, 1770)
    track_date <- fetchvars(core1, NA, TRACKING_DATE())$value

    core2 <- reset(core1)
    track_date2 <- fetchvars(core2, NA, TRACKING_DATE())$value
    expect_identical(track_date2, track_date)

    shutdown(core2)
})

test_that("Pool names are valid", {
    error_threshold <- 1e-3
    core <- newcore(inifile)
    setvar(core, NA, TRACKING_DATE(), 1760, tunits)
    reset(core, core$reset_date)
    run(core, 1770)

    # Check that pool_name and corresponding pool_units are the same values
    # as what is returned in the tracking data in the first year of the run
    tdata <- get_tracking_data(core)
    track_date <- fetchvars(core, NA, TRACKING_DATE())$value
    tdata <- subset(tdata, tdata$year == track_date)

    # HL
    HL <- subset(tdata, tdata$pool_name == "HL")
    HL_value <- unique(HL$pool_value)
    HL_units <- unique(HL$pool_units)

    h <- fetchvars(core, 1760, OCEAN_C_HL())
    expect_equal(h$value, HL_value, tolerance = error_threshold)
    expect_identical(h$units, HL_units)

    # LL
    LL <- subset(tdata, tdata$pool_name == "LL")
    LL_value <- unique(LL$pool_value)
    LL_units <- unique(LL$pool_units)

    l <- fetchvars(core, 1760, OCEAN_C_LL())
    expect_equal(l$value, LL_value, tolerance = error_threshold)
    expect_identical(l$units, LL_units)

    # Intermediate
    int <- subset(tdata, tdata$pool_name == "intermediate")
    int_value <- unique(int$pool_value)
    int_units <- unique(int$pool_units)

    i <- fetchvars(core, 1760, OCEAN_C_IO())
    expect_equal(i$value, int_value, tolerance = error_threshold)
    expect_identical(i$units, int_units)

    # Deep
    deep <- subset(tdata, tdata$pool_name == "deep")
    deep_value <- unique(deep$pool_value)
    deep_units <- unique(deep$pool_units)

    d <- fetchvars(core, 1760, OCEAN_C_DO())
    expect_equal(d$value, deep_value, tolerance = error_threshold)
    expect_identical(d$units, deep_units)

    # atmos_c
    atm <- subset(tdata, tdata$pool_name == "atmos_c")
    atm_value <- unique(atm$pool_value)
    atm_units <- unique(atm$pool_units)

    a <- fetchvars(core, 1760, ATMOSPHERIC_C())
    expect_equal(a$value, atm_value, tolerance = error_threshold)
    expect_identical(a$units, atm_units)

    # earth_c
    ear <- subset(tdata, tdata$pool_name == "earth_c")
    ear_value <- unique(ear$pool_value)
    ear_units <- unique(ear$pool_units)

    e <- fetchvars(core, 1760, EARTH_C())
    expect_equal(e$value, ear_value, tolerance = error_threshold)
    expect_identical(e$units, ear_units)

    # veg_c
    veg <- subset(tdata, tdata$pool_name == "veg_c")
    veg_value <- unique(veg$pool_value)
    veg_units <- unique(veg$pool_units)

    v <- fetchvars(core, 1760, VEG_C())
    expect_equal(v$value, veg_value, tolerance = error_threshold)
    expect_identical(v$units, veg_units)

    # detritus_c
    det <- subset(tdata, tdata$pool_name == "detritus_c")
    det_value <- unique(det$pool_value)
    det_units <- unique(det$pool_units)

    d <- fetchvars(core, 1760, DETRITUS_C())
    expect_equal(d$value, det_value, tolerance = error_threshold)
    expect_identical(d$units, det_units)

    # soil_c
    soil <- subset(tdata, tdata$pool_name == "soil_c")
    soil_value <- unique(soil$pool_value)
    soil_units <- unique(soil$pool_units)

    s <- fetchvars(core, 1760, SOIL_C())
    expect_equal(s$value, soil_value, tolerance = error_threshold)
    expect_identical(s$units, soil_units)

    # Check that source_names are present in pool_name column
    source_names <- sort(unique(tdata$source_name))
    pool_names <- sort(unique(tdata$pool_name))

    expect_identical(source_names, pool_names)

})
