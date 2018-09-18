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
    }
})
