context('Verify correctness of hector scenarios')

inputdir <- system.file('input', package='hector')
sampledir <- system.file('output', package='hector')
testvars <- c(ATMOSPHERIC_CO2(), RF_TOTAL(), GLOBAL_TEMP(), NPP())
tempvars <- c(GLOBAL_TEMP(), OCEAN_AIR_TEMP(), LAND_AIR_TEMP())
fland <- 0.29        # Global land area fraction
dates <- 2000:2300


test_that('RCP scenarios are correct', {
    for (rcp in c('26', '45', '60', '85')) {
        infile <- sprintf('hector_rcp%s.ini', rcp)
        scen <- sprintf('rcp%s', rcp)
        hc <- newcore(file.path(inputdir,infile), suppresslogging = TRUE)
        expect_true(inherits(hc, 'hcore'))
        run(hc)
        outdata <- fetchvars(hc, dates, testvars, scen)
        tempdata <- fetchvars(hc, dates, tempvars, scen)

        ## Get the comparison data
        sampleoutfile <- sprintf('sample_outputstream_rcp%s.csv', rcp)
        sampledata <- read.csv(file.path(sampledir, sampleoutfile), comment.char = '#', stringsAsFactors = FALSE)
        sampledata$scenario <- as.character(sampledata$run_name)
        samplekeep <- sampledata$variable %in% testvars & sampledata$year %in% dates
        sampledata <- sampledata[samplekeep, c('scenario', 'year','variable','value', 'units')]
        sampledata <- sampledata[order(sampledata$variable, sampledata$year),]

        ## Ensure output data has the same row ordering
        outdata <- outdata[order(outdata$variable, outdata$year),]
        ## comparison data were only written out with 4 significant figures
        outdata$value <- signif(outdata$value, 4)

        expect_equivalent(outdata, sampledata, info=sprintf("Output doesn't match for scenario rcp%s", rcp))

        td <- dplyr::select(tempdata, year, variable, value) %>% tidyr::spread(variable, value)
        tgcomp <- fland*td$Tgav_land + (1.0-fland)*td$Tgav_ocean_air
        expect_equal(tgcomp, td$Tgav, info=sprintf("Global temperature doesn't add up for scenario rcp%s", rcp))

        hc <- shutdown(hc)
    }
})


test_that('Atmospheric CO2 constraint works', {
    hc45 <- newcore(file.path(inputdir, 'hector_rcp45.ini'),
                              suppresslogging=TRUE)
    hc45const <- newcore(file.path(inputdir, 'hector_rcp45_constrained.ini'),
                                   suppresslogging=TRUE)

    run(hc45)
    run(hc45const)

    ## Check co2 concentration against the input constraint file.  Check that
    ## the constraints are actually doing something by checking both that we get
    ## the constrained co2 when we run with them, and that we *don't* get the
    ## constraint values when we run without them.
    co2const <- read.csv(file.path(inputdir, 'constraints', 'rcp45_co2ppm.csv'),
                         skip = 1)
    co2const <- co2const[co2const$Date >= min(dates) & co2const$Date <=
                           max(dates),]

    constvals <- signif(co2const$Ca_constrain, 5)

    rcp45co2 <- fetchvars(hc45, dates, ATMOSPHERIC_CO2())
    rcp45co2vals <- signif(rcp45co2$value, 5)

    rcp45constco2 <- fetchvars(hc45const, dates, ATMOSPHERIC_CO2())
    rcp45constco2vals <- signif(rcp45constco2$value, 5)

    expect_equal(rcp45constco2vals, constvals)
    expect_false(any(rcp45co2vals == constvals))

    shutdown(hc45)
    shutdown(hc45const)

})
