# This script parses information from rcpp_constants.cpp file to create a table of the
# R functions that can be used to fetch Hector outputs, their strings names, & units.

# Load the hector package
devtools::load_all()

# Load the inputs_params.csv, it will be used to seperate the inputs from the outputs.
input_params <- read.csv(file.path("data-raw", "input_params.csv"))

# Read ini the contents of the rcpp file.
lines <- readLines(file.path("src", "rcpp_constants.cpp"))

# Identify the lines of the script where the R function names are defined.
conditions <- (grepl("String ", x = lines) & !grepl(";", x = lines) & !grepl("// ", x = lines))
string_names <- lines[conditions]

# Clean up the selected lines of the cpp code so that the vector fxn_names only contains the names of
# the R functions. Evaluate the R functions and save the results.
removed_String <- gsub(pattern = "String |", replacement = "", x  = string_names)
fxn_names <- unlist(lapply(removed_String, function(s){ unlist(strsplit(x = s, split = "\\("))[[1]]}))
fxn_vals <- sapply(fxn_names, function(n){tryCatch({match.fun(n)()}, error = function(e){NA})}, USE.NAMES = FALSE)

# Store the R function names & results in a data frame. Clean up the data frame so that
# it only contains outputs.
func_df <- na.omit(data.frame(R_function = fxn_names, name = fxn_vals))
func_df <- func_df[!(func_df$name %in% input_params$parameter), ]
outputs <- func_df[!grepl(pattern = "constrain|uptake|emissionns|getData|setData", x = func_df$name), ]
outputs <- outputs[!grepl(pattern = "PREFIX|BIOME_SPLIT|GET|SET|WARMINGFACTOR|LO_WARMING_RATIO", x = outputs$R_function), ]

# Format the R function name to look more like the R function.
outputs$R_function <- unlist(lapply(outputs$R_function, function(s){paste0(s, "()")}))

# Use fetchvars to get results for all of the Hector output in order to add the
# unit information to the output table.
path <- file.path("inst", "input", "hector_ssp245.ini")
hc   <- newcore(path)
run(hc, runtodate = 1900)
hector_results <- fetchvars(hc, 1900, vars = outputs$name)

# Format the Hector outputs into a nice kable that to be inserted
# the output article.
cbind(outputs, units = hector_results$units) %>%
    knitr::kable(format = "markdown", row.names = FALSE)
