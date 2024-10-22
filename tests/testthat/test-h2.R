# Probably temporary testing or may be moved to other unit tests.


ini <- system.file("input/hector_ssp245.ini", package = "hector")
dates <- 1750:2100

test_that("tau ", {

    # Limited testing for now check to see tau oh can be read out...
    # in the future might want to test the actual values but for now
    # we are happy if the values range from 6.6 to 8
    hc <- newcore(ini)
    run(hc)
    out <- fetchvars(hc, dates = 1750:2100, vars = LIFETIME_OH())

    head(out)


    expect_equal(LIFETIME_OH(), "TAU_OH")
    expect_lte(mean(out$value), 8)
    expect_gte(mean(out$value), 6.6)

})


test_that("H2 emissions ", {

    # Check to make sure that can fetch and set the H2 emissions although
    # at this point changing the emissions will have no impact on
    # [ch4] dynamics but in the future it should...

    hc <- newcore(ini)
    run(hc)
    d <- 1750:2100
    out <- fetchvars(hc, dates = d,
                     vars = c(EMISSIONS_H2(), CONCENTRATIONS_CH4(), LIFETIME_OH()))

    new_val <- 10
    setvar(core = hc, dates = d, var = c(EMISSIONS_H2()), values = rep(new_val, length(d)),
           unit = getunits(EMISSIONS_H2()))
    reset(hc)
    run(hc)
    out2 <- fetchvars(hc, dates = d,
                     vars = c(EMISSIONS_H2(), CONCENTRATIONS_CH4(), LIFETIME_OH()))

    diff <- abs(out$value - out2$value)

    # As of now the [CH4] and tau oh should not change
    expect_equal(mean(diff[out$variable == CONCENTRATIONS_CH4()]), 0)
    expect_equal(mean(diff[out$variable == LIFETIME_OH()]), 0)

    # But if we can change the H2 emissions we should see a difference in
    # H2 emissions between the two new runs!
    expect_equal(mean(diff[out$variable == EMISSIONS_H2()]), new_val)

    })


