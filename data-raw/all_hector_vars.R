# Generate the strings that are used in the ALL_OUTPUT function.
# This script does require that an output stream has already been generated
# from the command line.


file <- list.files(path = "output", pattern = "outputstream_", full.names = TRUE)
stopifnot("need to generate outputstream file" = length(file) > 0)

dat <- read.csv(file = file[[1]], comment.char = "#")
all_vars <- unique(dat$variable)
all_vars <- all_vars[!all_vars %in% c(
  "hc_concentration", "HL_downwelling", "HL_OmegaAr", "LL_OmegaAr",
  "HL_OmegaCa", "LL_OmegaCa", "atmos_c_residual", "HL_Revelle", "HL_Revelle",
  "LL_Revelle", "slr", "slr_no_ice", "sl_rc", "sl_rc_no_ice"
)]

usethis::use_data(all_vars, internal = FALSE, overwrite = TRUE)
