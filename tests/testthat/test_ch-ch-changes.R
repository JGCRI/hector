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
    reset(hc, 0.0)
    run(hc, 2100)
    dd2 <- fetchvars(hc, tdates, GLOBAL_TEMP())

    ## Check that temperature is lower
    diff <- dd2$value - dd1$value
    expect_lt(max(diff), 0.0)

    shutdown(hc)
})
