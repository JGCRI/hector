context('Test basic hector functionality')

inputdir <- system.file('input', package='hector')
sampledir <- system.file('output', package='hector')
testvars <- c(ATMOSPHERIC_CO2(), RF_TOTAL(), GLOBAL_TEMP())
dates <- 2000:2300

test_that('Basic hcore functionality works', {
    hc <- newcore(file.path(inputdir, 'hector_rcp45.ini'), suppresslogging = TRUE)
    run(hc, 2100)
    expect_true(inherits(hc, 'hcore'))
    expect_true(isactive(hc))
    expect_equal(startdate(hc), 1745)
    expect_equal(enddate(hc), 2300)
    expect_equal(getdate(hc), 2100)

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
