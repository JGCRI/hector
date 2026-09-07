# Generate the strings that are used in the ALL_OUTPUT function.
# This script does require that an output stream has already been generated
# from the command line.


file <- list.files(path = "output", pattern = "outputstream_", full.names = TRUE)
stopifnot("need to generate outputstream file" = length(file) == 1)

dat <- read.csv(file = file[[1]], comment.char = "#")

# Extract all of the halocarbon base names.
componet_names <- unique(dat$component)
halo_base_names <- gsub(x = componet_names[grepl(pattern = "halocarbon", x = componet_names)],
                        pattern = "_halocarbon", replacement = "")

# Format the halocarbon emissions, constraints, concentrations, and forcings
# helper vectors.
halo_emiss <- paste0(halo_base_names, "_emissions")
halo_constraints <- paste0(halo_base_names, "_constrain")
halo_conc <- paste0(halo_base_names, "_concentration")
halo_rf <- paste0("RF_", halo_base_names)

all_vars <- unique(c(dat$variable, halo_conc, halo_rf))
all_vars <- all_vars[!all_vars %in% c(
  "hc_concentration", "HL_downwelling", "HL_OmegaAr", "LL_OmegaAr",
  "HL_OmegaCa", "LL_OmegaCa", "atmos_c_residual", "HL_Revelle", "HL_Revelle",
  "LL_Revelle", "slr", "slr_no_ice", "sl_rc", "sl_rc_no_ice"
)]


usethis::use_data(halo_emiss, all_vars, halo_constraints, halo_conc, halo_rf, internal = TRUE, overwrite = TRUE)

