devtools::load_all()

input_csv <- read.csv("./data-raw/input_params.csv")

usethis::use_data(input_csv, internal = TRUE, overwrite = TRUE)
