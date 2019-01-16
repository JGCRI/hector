#' Read Hector INI file into list
#'
#' @param file Path to INI file
#' @param parse_timeseries Logical. If `TRUE` (default), use
#'   [parse_timeseries()] to convert time series value names (e.g.
#'   `"Ftalbedo[1980]"`) to a `data.frame`. If `FALSE`, use the full
#'   string (with year included) as the key name.
#' @param normalize_paths Logical. If `TRUE` (default), convert any
#'   relative paths in the INI file to full, normalized paths,
#'   following the same logic as the Hector internals (if the file
#'   exists relative to the working directory, use that file.
#'   Otherwise, look for the path in `base_dir`). If `FALSE`, leave
#'   the paths exactly as written.
#' @param base_dir Path to directory to search for any relative paths,
#'   if `normalize_paths` is `TRUE`. Defaults to the directory in
#'   which the INI file is stored.
#' @return Nested list of the target object
#' @author Alexey Shiklomanov
#' @examples
#' rcp45_ini <- read_ini(
#'   system.file("input", "hector_rcp45.ini", package = "hector")
#' )
#' rcp45_ini$core$run_name
#' rcp45_ini$onelineocean$enabled
#' # This one will be a `data.frame`
#' rcp45_ini$simpleNbox$Ftalbedo
#' @export
read_ini <- function(file,
                     parse_timeseries = TRUE,
                     normalize_paths = TRUE,
                     base_dir = dirname(normalizePath(file))) {
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

  # Parse time series and return
  if (parse_timeseries) {
    value_list <- lapply(value_list, parse_timeseries)
  }

  if (normalize_paths) {
    value_list <- lapply(value_list, normalize_paths, base_dir = base_dir)
  }

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
  string <- "; Config file for hector model"
  for (tag in names(ini_list)) {
    string <- c(string, sprintf("[%s]", tag))
    sub_list <- ini_list[[tag]]
    for (key in names(sub_list)) {
      value <- sub_list[[key]]
      if (is.data.frame(value)) {
        string <- c(string,
                    sprintf("%s[%d] = %s", key, value[["date"]], value[[key]]))
      } else {
        string <- c(string, sprintf("%s = %s", key, value))
      }
    }
  }
  writeLines(string, file)
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

#' From a list of values, convert the ones that are time series
#' (e.g.`"Ftalbedo[1980]"`) to a `data.frame`. Leave scalar and
#' character values (including _paths_ to time series data) as is.
#'
#' @param value_list Named list of values from a particular section
#' @return `value_list`, but with all time series combined into
#'   `data.frame`s and moved to the end of the list.
#' @author Alexey Shiklomanov
parse_timeseries <- function(value_list) {
  vnames <- names(value_list)
  is_ts <- grepl("\\[.*\\]", vnames)
  scalars <- value_list[!is_ts]
  tss <- value_list[is_ts]
  ts_names <- gsub("\\[[[:digit:]]+\\]", "", names(tss))
  ts_years <- as.numeric(gsub(".*\\[([[:digit:]]+)\\.*]", "\\1", names(tss)))
  ts_groups <- split(tss, ts_names)
  ts_years_g <- split(ts_years, ts_names)

  ts <- mapply(
    function(values, years, value_col) {
      dat <- data.frame(x = years, y = unlist(values), row.names = NULL)
      colnames(dat) <- c("date", value_col)
      dat
    },
    ts_groups, ts_years_g, as.list(names(ts_groups)),
    SIMPLIFY = FALSE
  )

  c(scalars, ts)
}

#' Apply Hector internal logic to convert relative paths to absolute paths
#'
#' @inheritParams parse_timeseries
#' @param base_dir Base directory to use if files are not found
#' @return `value_list`, with all path variables replaced with
#'   normalized, absolute paths.
normalize_paths <- function(value_list, base_dir = NULL) {
  is_path <- vapply(
    value_list,
    function(x) is.character(x) && grepl("csv:", x),
    logical(1)
  )
  paths <- lapply(
    value_list[is_path],
    function(x) tryCatch({
      x <- gsub("csv:", "", x)
      paste0("csv:", normalizePath(x, mustWork = TRUE))
    },
    error = function(e) {
      if (is.null(base_dir)) stop("`base_dir` is NULL.",
                                  "I don't know where else to look for files.")
      paste0("csv:", normalizePath(file.path(base_dir, x), mustWork = TRUE))
    })
  )
  modifyList(value_list, paths)
}
