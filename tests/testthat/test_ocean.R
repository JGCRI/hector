context("Check ocean")
# Check some basic science to make sure Hector's ocean behaves as expected.

# Run Hector
inputdir <- system.file("input", package = "hector")
sampledir <- system.file("output", package = "hector")
rcp45 <- file.path(inputdir, "hector_rcp45.ini")

# Dates to extract values and run Hector.
t_dates <- 1850:1900
hc <- newcore(rcp45)
run(hc, max(t_dates))


test_that("Checking carbon pools", {

    # The sum of the high and low latitude, intermediate, and deep ocean pools should
    # equal the size of the ocean pool.
    total_ocean_pool <- fetchvars(core = hc, dates = t_dates, vars = OCEAN_C())

    carbon_pools <- c(OCEAN_C_HL(), OCEAN_C_LL(), OCEAN_C_IO(), OCEAN_C_DO())
    individual_carbon_pools <- fetchvars(core = hc, dates = t_dates, vars = carbon_pools)
    sum_carbon_pools <- aggregate(value ~ year, data = individual_carbon_pools, sum)

    # Define an error threhold that corresponds to less than about .025% of the average
    # total carbon pool.
    allowable_percent <- 0.025
    error_thresh <- (mean(total_ocean_pool$value) * allowable_percent) / 100
    expect_equal(total_ocean_pool$value, sum_carbon_pools$value, tolerance = error_thresh)
    })


test_that("Checking ocean flux", {

    # The total ocean to atmosphere flux should equal the sum of the ocean to atmosphere fluxes
    # at the high and low latitudes.

    # Get the total ocean to atmosphere flux.
    total_flux <- fetchvars(core = hc, dates = t_dates, vars = OCEAN_CFLUX())

    # Parse out the fluxes from the high and low latitude pools and calculate the sum
    # to compare with the total.
    pool_fluxes <- fetchvars(core = hc, dates = t_dates, vars = c(ATM_OCEAN_FLUX_HL(),
                                                                  ATM_OCEAN_FLUX_LL()))
    sum_pool_flux <- aggregate(value ~ year, data = pool_fluxes, sum)

    expect_equal(total_flux$value, sum_pool_flux$value)

})

test_that("Checking high and low latitude difference", {

    # We expect that the high and low latitude ocean boxes produce different
    # results, they should have different temperature, carbon fluxes, co3, and ph values.

    # Calculate the sum of the high latitude variables.
    hl_vars <- c(OCEAN_C_HL(), PH_HL(), ATM_OCEAN_FLUX_HL(), PCO2_HL(), TEMP_HL(), CO3_HL())
    hl_results <- fetchvars(core = hc, dates = t_dates, vars = hl_vars)
    mean_hl <- aggregate(value ~ variable, data = hl_results, mean)

    # Calculate the sum of the low latitude variables.
    ll_vars <- c(OCEAN_C_LL(), PH_LL(), ATM_OCEAN_FLUX_LL(), PCO2_LL(), TEMP_LL(), CO3_LL())
    ll_results <- fetchvars(core = hc, dates = t_dates, vars = ll_vars)
    mean_ll <- aggregate(value ~ variable, data = ll_results, mean)

    # None of the high and low latitude boxes should return the same results.
    expect_true(all(mean_hl$value != mean_ll$value))

})

test_that("Read and writing ocean parameters", {

    # The ocean parameters
    params <- c(TT(), TU(), TWI(), TID())

    # Should throw an error if a date is passed in.
    expect_error(fetchvars(hc, dates = t_dates, vars = params))

    # Save a copy of the default hector parameter values.
    default_params <- fetchvars(hc, dates = NA, vars = params)
    expect_equal(nrow(default_params), length(params))

    # Save a copy of the default Hector ocean variables, only check the values from the total ocean
    # box or one of the surface boxes.
    ocean_vars <- c(OCEAN_CFLUX(), OCEAN_C(), PH_HL(), PCO2_HL(), DIC_HL(), TEMP_HL(), CO3_HL())
    default_ocean_results <- fetchvars(hc, t_dates, vars = ocean_vars)
    default_mean <- aggregate(value ~ variable, data = default_ocean_results, mean)

    # Change the default hector parameters.
    new_params <- default_params$value * 1.1

    # Run hector with the new parameter values and the new output with the old output.
    mapply(function(val, p, u) {

        # Create a new hector core each time so that only one
        # parameter is being changed at a time.
        hc2 <- newcore(rcp45)

        # reset the hector core
        setvar(hc2, dates = NA, var = p, values = val, unit = u)
        reset(hc2)
        run(hc2, runtodate = max(t_dates))
        new_ocecan_out <- fetchvars(hc2, t_dates, vars = ocean_vars)
        new_mean <- aggregate(value ~ variable, data = new_ocecan_out, mean)

        # Make sure that Hector output is changed when ocean component parameters are
        # changed. It is safe to expect that the change in the Hector output should be
        # greater than the error threshold which we've set at 1e-10
        # What is the absolute difference between the mean ocean variables?
        error_threshold <- 1e-10
        diff <- abs(new_mean$value - default_mean$value)
        expect_true(all(diff > error_threshold), info = cat("problem with resetting ", p, "\n"))

    }, val = new_params, p = params, u = default_params$units)



})
