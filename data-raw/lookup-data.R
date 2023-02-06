# Generate the table of Hector strings and functions for the look up table.
# Load the hector package
devtools::load_all()

# Helper function to evaluate an export R function
# Args
#   n: name of the function
# Returns: string output from the function
hector_eval_func <- function(n) {
    print(n)
    assertthat::assert_that(is.character(n))
    x <- match.fun(gsub(x = n, pattern = "\\()", replacement = ""))
    assertthat::assert_that(is.function(x))
    x()
}

# Read ini the contents of the rcpp file, this is written with the working
# directory set to the root directory of the Hector repository/package.
lines <- readLines("./src/rcpp_constants.cpp")

# Specific functions to exclude
# TODO is there a way to make this not hard coded?
lines <- lines[!grepl(
    pattern = "GETDATA|SETDATA|BIOME_SPLIT_CHAR|RFADJ_PREFIX|RF_PREFIX",
    x = lines)]

# Identify the lines of the code where the R function names are defined.
conditions <- (grepl("^String ", x = lines) & !grepl("^ ", x = lines))
string_names <- lines[conditions]
string_names <- gsub(pattern = "\\(String", replacement = "()", x = string_names)

# Parse out the function names, evaluate the function then store results in
# a table.
fxn_names <- unlist(lapply(string_names, function(s) {
    unlist(strsplit(s, split = " "))[2]}))
fxn_vals <- unlist(lapply(fxn_names, hector_eval_func))
fxntable <- data.frame(fxn = fxn_names, string = fxn_vals)

usethis::use_data(fxntable, internal = FALSE, overwrite = TRUE)
