#' Read Hector INI file into list
#'
#' @param file Path to INI file
#' @return
#' @author Alexey Shiklomanov
#' @examples
#' rcp45_ini <- read_ini(
#'   system.file("input", "hector_rcp45.ini", package = "hector")
#' )
#' rcp45_ini$core$run_name
#' rcp45_ini$onelineocean$enabled
#' @export
read_ini <- function(file) {
  full_file <- readLines(file)

  # Remove comment lines, which start with `;`
  no_comments <- grep("^\\s*;", full_file, value = TRUE, invert = TRUE)

  # Remove empty lines
  no_empty <- grep("^\\s*$", no_comments, value = TRUE, invert = TRUE)

  # Remove trailing whitespace and comments
  no_trailing <- gsub("\\s*;.*$", "", no_empty)

  # Trim whitespace
  trimmed <- gsub("^\\s*(.*?)\\s*$", "\\1", no_trailing)

  i_headers <- grep("^\\[.*\\]", trimmed)
  n_headers <- diff(c(i_headers, length(trimmed) + 1)) - 1
  header_tags <- gsub("\\[|\\]", "", trimmed[i_headers])

  f_values <- rep(seq_along(n_headers), n_headers)
  stopifnot(length(f_values) == length(trimmed) - length(i_headers))

  raw_values <- split(trimmed[-i_headers], f_values)
  value_list <- lapply(raw_values, keyvalue)
  names(value_list) <- header_tags

  value_list
}

#' Write R list to Hector INI file
#'
#' @param ini_list Hector INI named, nested list, such as that
#'   returned by [read_ini()].
#' @param file Output file, as character string or connection. See
#'   [base::writeLines()].
#' @return `ini_list`, invisibly
#' @author Alexey Shiklomanov
#' @examples
#' l <- list(a = list(a1 = 5, a2 = 3.5),
#'           b = list(some_file = "hello", `b3[50]` = 5))
#' tmp <- tempfile()
#' write_ini(l, tmp)
#' # Write to `stdout()`
#' write_ini(l, "")
#' @export
write_ini <- function(ini_list, file) {
  stopifnot(!is.null(names(ini_list)))
  write("; Config file for hector model", file)
  for (tag in names(ini_list)) {
    write(sprintf("[%s]", tag), file, append = TRUE)
    sub_list <- ini_list[[tag]]
    for (key in names(sub_list)) {
      write(sprintf("%s = %s", key, sub_list[[key]]), file, append = TRUE)
    }
  }
  invisible(ini_list)
}

#' Convert a vector of strings of the form `"name=value"` to a named R
#' list
#'
#' @param x Character vector of the form `"name=value"`
#' @return Named list
#' @author Alexey Shiklomanov
keyvalue <- function(x) {
  stopifnot(is.character(x))
  xsplit <- strsplit(x, "\\s*=\\s*")
  values <- lapply(xsplit, function(x) try_numeric(x[[2]]))
  names(values) <- lapply(xsplit, function(x) x[[1]])
  values
}

#' Try to convert a value to numeric, and if it throws a warning,
#' return the original value.
#'
#' @param x Value to be converted to numeric
#' @return `as.numeric(x)` if successful, or `x` if not
#' @author Alexey Shiklomanov
try_numeric <- function(x) {
  stopifnot(is.character(x))
  tryCatch(
    as.numeric(x),
    warning = function(e) x
  )
}
