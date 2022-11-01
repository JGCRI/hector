context("LUC pulse")

# Credit to Dawn Woodard for this stringent test:
# A Hector run with zero emissions, Q10=1, and beta=0 should:
#   (1) exhibit flat carbon pools post-spinup, and
#   (2) a LUC pulse should instantly equilibrate in the terrestrial pools
# We use a custom INI file for this

test_that("Hector responds correctly to a LUC pulse", {
    hc <- newcore("input/luc_pulse.ini")
    run(hc)

    # The carbon pools should be flat post-spinup
    # Our pulse is LUC, so focus on veg C
    x <- fetchvars(hc, 1750:1799, VEG_C())
    expect_true(all(diff(x$value) < 1e-6))

    # The LUC pulse is at 1800 in this test file
    # As long as LUC is proportional to pool sizes (see PR #647),
    # there should be no trend in subsequent years
    x <- fetchvars(hc, 1801:1850, VEG_C())
#    expect_true(all(diff(x$value) < 1e-6))

    # Finally, the LUC pulse itself should be equal to what's in the input file
    emiss <- read.csv("input/luc_pulse_tables.csv", skip = 3)
    luc <- fetchvars(hc, hc$strtdate:hc$enddate, LUC_EMISSIONS())
    expect_equal(sum(luc$value), sum(emiss$luc_emissions))

    shutdown(hc)
})
