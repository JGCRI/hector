context("fxns table")

test_that("fxn retrieval works.", {
    strs <- c("beta", "S", "q10_rh")
    fxns <- getfxn(strs)
    expected_fxns <- c("BETA()", "ECS()", "Q10_RH()")

    expect_equal(fxns, expected_fxns)
})

test_that("Invalid string retrieval warns.", {
    vars <- c("fake")
    expect_warning(getfxn(vars), "Functions for the following are not found: fake")
})
