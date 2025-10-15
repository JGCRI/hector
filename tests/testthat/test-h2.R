# Probably temporary testing or may be moved to other unit tests.


ini <- system.file("input/hector_ssp245.ini", package = "hector")
dates <- 1750:2100

test_that("tau ", {

    # Limited testing for now check to see tau oh can be read out...
    # in the future might want to test the actual values but for now
    # we are happy if the values range from 6.6 to 8
    hc <- newcore(ini)
    run(hc)
    out <- fetchvars(hc, dates = 1750:2100, vars = CH4_LIFETIME_OH())

    head(out)


    expect_equal(CH4_LIFETIME_OH(), "TAU_OH")
    expect_lte(mean(out$value), 8)
    expect_gte(mean(out$value), 6.6)

})


test_that("H2 emissions ", {

    # Check to make sure that can fetch and set the H2 emissions and
    # that the effects of adding some H2 emissions change things
    # now that we have the CH4 indirect effects implement.

    hc <- newcore(ini)
    run(hc)
    d <- 1750:2100
    out <- fetchvars(hc, dates = d,
                     vars = c(EMISSIONS_H2(), CONCENTRATIONS_CH4(), CH4_LIFETIME_OH()))

    new_val <- 10
    setvar(core = hc, dates = d, var = c(EMISSIONS_H2()), values = rep(new_val, length(d)),
           unit = getunits(EMISSIONS_H2()))
    reset(hc)
    run(hc)
    out2 <- fetchvars(hc, dates = d,
                     vars = c(EMISSIONS_H2(), CONCENTRATIONS_CH4(), CH4_LIFETIME_OH()))

    diff <- abs(out$value - out2$value)

    # Now we do expect to see changes in CH4 concentrations in response
    # to H2 emissions
    expect_gt(mean(diff[out$variable == CONCENTRATIONS_CH4()]), 0)
    expect_gt(mean(diff[out$variable == CH4_LIFETIME_OH()]), 0)

    # Since we are working off of some default H2 emissions = 0, then
    # changing the H2 emissions we should see a difference.
    expect_equal(mean(diff[out$variable == EMISSIONS_H2()]), new_val)

    })


