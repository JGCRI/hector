context('Test basic hector functionality')

inputdir <- system.file('input', package='hector')
sampledir <- system.file('output', package='hector')
testvars <- c(ATMOSPHERIC_CO2(), RF_TOTAL(), GLOBAL_TEMP())
dates <- 2000:2300

test_that('Basic hcore functionality works', {
    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), name='RCP45', suppresslogging = TRUE)
    run(hc, 2100)
    expect_true(inherits(hc, 'hcore'))
    expect_true(isactive(hc))
    expect_equal(startdate(hc), 1745)
    expect_equal(enddate(hc), 2300)
    expect_equal(getdate(hc), 2100)
    expect_equal(getname(hc), 'RCP45')

    expect_error(run(hc, 2050), "is prior to the current date")
    expect_silent(run(hc, 2100))
    expect_silent(run(hc))

    hc <- shutdown(hc)
    expect_false(isactive(hc))

    ## Check that errors on shutdown cores get caught
    expect_error(getdate(hc), "Invalid or inactive")
    expect_error(run(hc), "Invalid or inactive")
    expect_error(fetchvars(hc), "Invalid or inactive")
})

test_that('Garbage collection shuts down hector cores', {
    ## This test makes use of some knowledge about the structure of the hector
    ## core objects that no user should ever assume.
    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), suppresslogging = TRUE, name='core1')
    expect_true(isactive(hc))
    coreidx1 <- hc$coreidx

    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), suppresslogging = TRUE, name='core2')
    expect_true(isactive(hc))
    coreidx2 <- hc$coreidx
    expect_equal(coreidx2, 1+coreidx1)
    gc(verbose=FALSE)

    ## make a deep copy of the structure
    oldhc <- as.environment(as.list(hc))
    oldhc$coreidx <- coreidx1
    class(oldhc) <- c('hcore','environment')
    expect_false(isactive(oldhc))
    expect_true(isactive(hc))
    shutdown(hc)
})

test_that('Scenario column is created in output', {
    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), suppresslogging = TRUE, name='scenario1')
    run(hc)

    outdata1 <- fetchvars(hc, dates, testvars)
    expect_equal(outdata1$scenario, rep('scenario1', nrow(outdata1)))

    outdata2 <- fetchvars(hc, dates, testvars, 'scenario2')
    expect_equal(outdata2$scenario, rep('scenario2', nrow(outdata2)))

    shutdown(hc)
})


test_that("Reset produces identical results",{
    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), suppresslogging = TRUE)
    run(hc)
    expect_equal(getdate(hc), enddate(hc))
    outdata1 <- fetchvars(hc, dates, testvars)

    reset(hc,2000)
    expect_equal(getdate(hc), 2000)
    run(hc)
    outdata2 <- fetchvars(hc, dates, testvars)
    expect_equal(outdata1, outdata2)
    hc <- shutdown(hc)

})

test_that("Setting emissions changes results", {
    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), suppresslogging = TRUE)
    run(hc, 2100)

    outdata1a <- fetchvars(hc, 2000:2089, c(ATMOSPHERIC_CO2(), RF_TOTAL()))
    outdata1b <- fetchvars(hc, 2091:2100, c(ATMOSPHERIC_CO2(), RF_TOTAL()))
    reset(hc, 2000)

    ## crank up the emissions dramatically in 2090
    setvar(hc, 2090, FFI_EMISSIONS(), 10.0, "Pg C/yr")
    run(hc, 2100)
    outdata2a <- fetchvars(hc, 2000:2089, c(ATMOSPHERIC_CO2(), RF_TOTAL()))
    outdata2b <- fetchvars(hc, 2091:2100, c(ATMOSPHERIC_CO2(), RF_TOTAL()))

    ## Prior to the change results should be the same
    expect_equal(outdata1a, outdata2a)

    ## After the change, concentration and forcing should be higher than they were in the original run
    expect_true(all(outdata2b$value > outdata1b$value))

    hc <- shutdown(hc)
})


test_that("Automatic reset is performed if and only if core is not marked 'clean'.", {
    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), suppresslogging = TRUE)

    expect_true(hc$clean)
    run(hc, 2100)
    ## We can test whether reset actually runs by noting that trying to run to
    ## an earlier date is an error, but an auto-reset will prevent the error
    expect_error(run(hc, 2050), "is prior") # No reset
    hc$clean <- FALSE
    hc$reset_date <- 0
    expect_silent(run(hc, 2050))        # reset performed
    expect_true(hc$clean)

    hc$clean <- FALSE
    reset(hc)                           # explicit reset
    expect_true(hc$clean)

    hc$clean <- FALSE
    reset(hc, startdate(hc))            # doesn't rerun spinup
    expect_false(hc$clean)
    hc$reset_date <- 2000               # spinup no longer required
    reset(hc, startdate(hc))
    expect_true(hc$clean)

    shutdown(hc)
})


test_that("Setting future values does not trigger a reset.", {
    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), suppresslogging = TRUE)

    run(hc, 2100)
    setvar(hc, 2101:2300, FFI_EMISSIONS(), 0.0, "Pg C/yr")
    expect_true(hc$clean)

    shutdown(hc)
})


test_that("Setting past or parameter values does trigger a reset.", {
    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), suppresslogging = TRUE)
    run(hc, 2100)

    setvar(hc, 2050:2150, FFI_EMISSIONS(), 0.0, "Pg C/yr")
    expect_false(hc$clean)
    expect_equal(hc$reset_date, 2049)
    expect_error(run(hc, 2048), "is prior")
    expect_true(hc$clean)               # reset still gets run!
    expect_silent(run(hc, 2050))
    expect_true(hc$clean)

    setvar(hc, 2050:2150, FFI_EMISSIONS(), 0.0, "Pg C/yr") # edge case
    expect_false(hc$clean)
    expect_equal(hc$reset_date, 2049)
    expect_error(run(hc, 2048), "is prior")
    expect_silent(run(hc, 2050))
    expect_true(hc$clean)

    ## Setting two sets of values should reset to the lower one
    setvar(hc, 2000, FFI_EMISSIONS(), 0.0, "Pg C/yr")
    setvar(hc, 2010, FFI_EMISSIONS(), 0.0, "Pg C/yr")
    expect_equal(hc$reset_date, 1999)
    expect_false(hc$clean)
    setvar(hc, 1972, FFI_EMISSIONS(), 0.0, "Pg C/yr")
    expect_equal(hc$reset_date, 1971)
    expect_false(hc$clean)

    ## Setting parameter values should trigger a reset
    setvar(hc, NA, ECS(), 2.5, 'degC')
    expect_false(hc$clean)
    expect_equal(hc$reset_date, 0)
    reset(hc)
    expect_true(hc$clean)
    setvar(hc, NA, ECS(), 3, 'degC')
    expect_false(hc$clean)
    setvar(hc, 1800, FFI_EMISSIONS(), 0.0, "Pg C/yr") # shouldn't change the
                                        # reset date
    expect_equal(hc$reset_date, 0)


    shutdown(hc)
})
