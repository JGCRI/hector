if (requireNamespace("lintr", quietly = TRUE)) {
  context("Package style")
  if (FALSE) {
    # If this test fails then you need to reformat code! Do so by running the
    # following
    styler::style_pkg(exclude_files = c("R/RcppExports.R", "vignettes/"))
  }
  test_that("No lints found", {
    lintr::expect_lint_free(exclusions = list("R/RcppExports.R", "vignettes/"))
  })
}
