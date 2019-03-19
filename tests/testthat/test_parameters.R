context('Test changes to Hector parameters')

inputdir <- system.file('input', package='hector')
sampledir <- system.file('output', package='hector')
testvars <- c(ATMOSPHERIC_CO2(), RF_TOTAL(), GLOBAL_TEMP())
dates <- 1750:2100

test_that('Rerunning spinup produces minimal change', {
    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), suppresslogging = TRUE)
    run(hc, 2100)
    dd1 <- fetchvars(hc, dates, testvars)

    ## reset to before start date, but do not change anything
    reset(hc, 0.0)
    run(hc, 2100)
    dd2 <- fetchvars(hc, dates, testvars)

    diff <- abs((dd2$value - dd1$value) / (dd1$value + 1.0e-6))
    expect_lt(max(diff), 1.0e-6)

    shutdown(hc)
})

test_that('Lowering initial CO2 lowers output CO2', {
    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), suppresslogging = TRUE)
    run(hc, 2100)
    dd1 <- fetchvars(hc, dates, ATMOSPHERIC_CO2())

    ## Change the preindustrial CO2
    setvar(hc, NA, PREINDUSTRIAL_CO2(), 250, "ppmv CO2")
    reset(hc, 0.0)
    run(hc, 2100)
    dd2 <- fetchvars(hc, dates, ATMOSPHERIC_CO2())

    ## The concentrations should start off lower than the reference run by
    ## approximately the change in initial concentration, and the deficit
    ## increases with time.  We'll test for this approximately
    diff <- dd2$value - dd1$value       # should all be negative
    expect_lt(max(diff), -26.0)

    shutdown(hc)
})


test_that('Lowering ECS lowers output Temperature', {
    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), suppresslogging = TRUE)
    run(hc, 2100)
    ## temperature bounces around a bit in the early years; limit test to after
    ## 2000, when the signal is clear
    tdates <- 2000:2100
    dd1 <- fetchvars(hc, tdates, GLOBAL_TEMP())

    setvar(hc, NA, ECS(), 2.5, 'degC')
    ## make sure this still works with automatic reset.
    run(hc, 2100)
    dd2 <- fetchvars(hc, tdates, GLOBAL_TEMP())

    ## Check that temperature is lower
    diff <- dd2$value - dd1$value
    expect_lt(max(diff), 0.0)

    shutdown(hc)
})

test_that('Raising Q10 increases CO2 concentration', {
    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), suppresslogging = TRUE)
    run(hc, 2100)
    qdates <- 2000:2100                 # limit to years where temperature
                                        # increase is smooth.
    dd1 <- fetchvars(hc, qdates, ATMOSPHERIC_CO2())

    ## Change the preindustrial CO2
    setvar(hc, NA, Q10_RH(), 2.5, NA)
    reset(hc, 0.0)
    run(hc, 2100)
    dd2 <- fetchvars(hc, qdates, ATMOSPHERIC_CO2())

    ## Check that concentration is higher across the board
    diff <- dd2$value - dd1$value
    expect_gt(min(diff), 0.0)

    shutdown(hc)
})

test_that('Lowering diffusivity increases temperature', {
    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), suppresslogging = TRUE)
    run(hc, 2100)
    qdates <- 2000:2100                 # limit to years where temperature
                                        # increase is smooth.
    dd1 <- fetchvars(hc, qdates, GLOBAL_TEMP())

    ## Change the diffusivity
    setvar(hc, NA, DIFFUSIVITY(), 2.0, "cm2/s")
    reset(hc, 0.0)
    run(hc, 2100)
    dd2 <- fetchvars(hc, qdates, GLOBAL_TEMP())

    ## Check that concentration is higher across the board
    diff <- dd2$value - dd1$value
    expect_gt(min(diff), 0.0)

    shutdown(hc)
})

test_that('Lowering aerosol forcing scaling factor increases temperature', {
    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), suppresslogging =
                    TRUE)
    run(hc, 2100)
    qdates <- 2000:2100

    dd1 <- fetchvars(hc, qdates, GLOBAL_TEMP())

    ## Change the aerosol scaling factor
    setvar(hc, NA, AERO_SCALE(), 0.5, NA)
    reset(hc, 0.0)
    run(hc, 2100)
    dd2 <- fetchvars(hc, qdates, GLOBAL_TEMP())

    ## Check that temperatures are higher
    diff <- dd2$value - dd1$value
    expect_gt(min(diff), 0.0)

    shutdown(hc)
})

test_that('Increasing volcanic forcing scaling factor increases the effect of volcanism', {
    ## Use the difference between 1960 temperature and 1965 temperature as a
    ## measure of the volcanic effect.
    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), suppresslogging=TRUE)
    run(hc, 1971)
    dates <- c(1960, 1965)
    tbase <- fetchvars(hc, dates, GLOBAL_TEMP())
    vsibase <- tbase$value[1] - tbase$value[2]

    setvar(hc, NA, VOLCANIC_SCALE(), 1.5, getunits(VOLCANIC_SCALE()))
    reset(hc)
    run(hc, 1971)
    tscl <- fetchvars(hc, dates, GLOBAL_TEMP())
    vsiscl <- tscl$value[1] - tscl$value[2]

    expect_gt(vsiscl, vsibase)
})
