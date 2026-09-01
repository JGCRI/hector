context("internal pacakge data")

test_that("internal halocarbon data are available", {
    ns <- asNamespace("hector")
    expect_true(exists("halo_conc", envir = ns))
    expect_true(exists("halo_constraints", envir = ns))
    expect_true(exists("halo_emiss", envir = ns))
    expect_true(exists("halo_rf", envir = ns))
})
