## test basic hector functionality

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


test_that('RCP scenarios are correct', {
    for (rcp in c('26', '45', '60', '85')) {
        infile <- sprintf('hector_rcp%s.ini', rcp)
        hc <- newcore(file.path(inputdir,infile), suppresslogging = TRUE)
        expect_true(inherits(hc, 'hcore'))
        run(hc)
        outdata <- fetchvars(hc, dates, testvars)

        ## Get the comparison data
        sampleoutfile <- sprintf('sample_outputstream_rcp%s.csv', rcp)
        sampledata <- read.csv(file.path(sampledir, sampleoutfile), comment.char = '#')
        samplekeep <- sampledata$variable %in% testvars & sampledata$year %in% dates
        sampledata <- sampledata[samplekeep, c('year','variable','value', 'units')]
        sampledata <- sampledata[order(sampledata$variable, sampledata$year),]

        ## Ensure output data has the same row ordering
        outdata <- outdata[order(outdata$variable, outdata$year),]
        ## comparison data were only written out with 4 significant figures
        outdata$value <- signif(outdata$value, 4)

        expect_equivalent(outdata, sampledata, info=sprintf("Output doesn't match for scenario rcp%s", rcp))
        hc <- shutdown(hc)
    }
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
