
if (requireNamespace("lintr", quietly = TRUE)) {
    context("Package style")
    test_that("No lints found", {
        lintr::expect_lint_free(exclusions = list("R/RcppExports.R", "vignettes/"))
    })
}
