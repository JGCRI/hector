context("Hector with multiple biomes")

ssp245 <- function() {
    newcore(system.file("input", "hector_ssp245.ini", package = "hector"),
            name = "test core", suppresslogging = TRUE)
}

# Save a copy of the default values
core <- ssp245()
default_beta <- fetchvars(core, NA, BETA())



test_that("Hector runs with multiple biomes created via INI file", {
  string2core <- function(ini_string, name, ini_file = NULL) {
    if (is.null(ini_file)) {
      ini_file <- tempfile()
      on.exit(file.remove(ini_file), add = TRUE)
      writeLines(ini_string, ini_file)
    }
    newcore(ini_file, name = name, suppresslogging = TRUE)
  }

  quickrun <- function(ini_string, name, ini_file = NULL) {
    core <- string2core(ini_string, name, ini_file)
    invisible(run(core))
    on.exit(shutdown(core), add = TRUE)
    dates <- seq(2000, 2100)
    vars <- c(
      CONCENTRATIONS_CO2(),
      RF_TOTAL(),
      GLOBAL_TAS()
    )
    fetchvars(core, dates, vars)
  }

  ssp245_file <- system.file("input", "hector_ssp245.ini", package = "hector")
  raw_ini <- trimws(readLines(ssp245_file))
  new_ini <- raw_ini

  # Remove non-biome-specific variables
  biome_vars <- c(
    "veg_c", "detritus_c", "soil_c", "permafrost_c", "npp_flux0",
    "beta", "q10_rh", "f_nppv", "f_nppd", "f_litterd"
  )
  biome_rxp <- paste(biome_vars, collapse = "|")
  iremove <- grep(sprintf("^(%s) *=", biome_rxp), raw_ini)
  new_ini <- new_ini[-iremove]

  # Add biome-specific versions of above variables at top of
  # simpleNbox block
  isnbox <- grep("^\\[simpleNbox\\]$", new_ini)
  new_ini <- append(new_ini, c(
    "boreal.veg_c = 100",
    "tropical.veg_c = 450",
    "boreal.detritus_c = 15",
    "tropical.detritus_c = 45",
    "boreal.soil_c = 1200",
    "tropical.soil_c = 578",
    "boreal.permafrost_c = 0",
    "tropical.permafrost_c = 0",
    "boreal.npp_flux0 = 5.0",
    "tropical.npp_flux0 = 45.0",
    "boreal.beta = 0.36",
    "tropical.beta = 0.36",
    "boreal.q10_rh = 2.0",
    "tropical.q10_rh = 2.0",
    "boreal.f_nppv = 0.35",
    "tropical.f_nppv = 0.35",
    "boreal.f_nppd = 0.60",
    "tropical.f_nppd = 0.60",
    "boreal.f_litterd = 0.98",
    "tropical.f_litterd = 0.98"
  ), after = isnbox)

  # Make csv paths absolute (otherwise, they search in the tempfile directory)
  icsv <- grep("^ *.*?=csv:", new_ini)
  csv_paths_l <- regmatches(
    new_ini[icsv],
    regexec(".*?=csv:(.*?\\.csv)", new_ini[icsv])
  )
  csv_paths <- vapply(csv_paths_l, `[[`, character(1), 2)
  csv_full_paths <- file.path(dirname(ssp245_file), csv_paths)
  new_ini_l <- Map(
    gsub,
    pattern = csv_paths,
    replacement = csv_full_paths,
    x = new_ini[icsv]
  )
  new_ini[icsv] <- unlist(new_ini_l, use.names = FALSE)

  biome_result <- quickrun(new_ini, "biome")
  ssp245_result <- quickrun(NULL, "default", ini_file = ssp245_file)

  result_diff <- ssp245_result$value - biome_result$value
  diff_summary <- tapply(result_diff, ssp245_result$variable, sum)
  expect_true(all(abs(diff_summary) > 0))

  # Add the warming tag
  warm_biome <- append(new_ini, c(
    "boreal.warmingfactor = 2.5",
    "tropical.warmingfactor = 1.0"
  ), after = isnbox)
  warm_biome_result <- quickrun(warm_biome, "warm_biome")
  default_tas <- ssp245_result[
    ssp245_result[["variable"]] == GLOBAL_TAS(),
    "value"
  ]
  warm_tas <- warm_biome_result[
    warm_biome_result[["variable"]] == GLOBAL_TAS(),
    "value"
  ]
  expect_true(mean(default_tas) != mean(warm_tas))

  # Try to add a fake biome. This should fail because other variables
  # haven't been initialized.
  extra_biome <- append(new_ini, c(
    "extra.veg_c = 1.0"
  ), after = isnbox)
  expect_error(
    quickrun(extra_biome, "extra_biome"),
    "not same size"
  )
})

test_that("Creating new biomes via set/fetchvar is prohibited", {
  core <- ssp245()
  b1 <- fetchvars(core, NA, BETA())
  b2 <- fetchvars(core, NA, BETA("global"))
  expect_equal(b1$value, b2$value)
  expect_error(
    fetchvars(core, NA, BETA("fake")),
    "Biome 'fake' missing from biome list"
  )
  expect_error(
    setvar(core, NA, BETA("permafrost"), 0.5, NA),
    "Biome 'permafrost' missing from biome list"
  )
})

test_that("Low-level biome creation functions work", {
  core <- ssp245()
  default_beta <- fetchvars(core, NA, BETA())
  test_that("Biomes can be created", {
    expect_silent(invisible(hector:::create_biome_impl(core, "testbiome")))
    expect_equal(get_biome_list(core), c("global", "testbiome"))
    expect_equal(fetchvars(core, NA, BETA("testbiome"))[["value"]], default_beta[["value"]])
    expect_equal(fetchvars(core, NA, VEG_C("testbiome"))[["value"]], 0)
  })

  test_that("Biomes can be deleted", {
    expect_silent(invisible(hector:::delete_biome_impl(core, "testbiome")))
    expect_equal(get_biome_list(core), "global")
    expect_error(
      fetchvars(core, NA, BETA("testbiome")),
      "Biome 'testbiome' missing from biome list"
    )
    expect_error(
      fetchvars(core, NA, VEG_C("testbiome")),
      "Biome 'testbiome' missing from biome list"
    )
  })
  # Check that running the core still works after all of this
  expect_true(all(class(run(core)) == c("hcore", "environment")))
})

test_that("Correct way to create new biomes", {
  core <- ssp245()
  gbeta <- fetchvars(core, NA, BETA())
  expect_equal(get_biome_list(core), "global")
  invisible(rename_biome(core, "global", "permafrost"))
  expect_equal(get_biome_list(core), "permafrost")
  expect_error(fetchvars(core, NA, BETA()), "Biome 'global' missing from biome list")
  pbeta <- fetchvars(core, NA, "permafrost.beta")
  expect_equal(pbeta[["variable"]], BETA("permafrost"))
  expect_equal(pbeta[["value"]], gbeta[["value"]])
  expect_true(is.environment(run(core)))

  results_pf <- fetchvars(core, 2000:2100)
  expect_error(create_biome(core, "permafrost"), "Biome 'permafrost' is already in `biome_list`")
  expect_error(invisible(create_biome(core, "empty")),
               'argument "veg_c0" is missing, with no default', fixed = FALSE)
  expect_equal(get_biome_list(core), c("permafrost", "empty"))
  expect_equal(fetchvars(core, NA, BETA("empty"))[["value"]], pbeta[["value"]])
  expect_silent(invisible(run(core)))
  results_pfe <- fetchvars(core, 2000:2100)
  expect_equal(results_pf, results_pfe)
})

test_that("Split biomes, and modify parameters", {
  core <- ssp245()
  invisible(rename_biome(core, "global", "default"))
  expect_equal(get_biome_list(core), "default")
  global_veg <- fetchvars(core, dates = NA, vars = VEG_C("default"))[["value"]]
  invisible(run(core))
  r_global <- fetchvars(core, 2000:2100)
  r_global_pools <- fetchvars(core, 2000:2100, c(VEG_C("default"),
                                                 DETRITUS_C("default"),
                                                 SOIL_C("default")))

  r_global_pools$biome <- gsub("^(.*)\\.(.*)", "\\1", r_global_pools$variable)
  r_global_pools$variable <- gsub("^(.*)\\.(.*)", "\\2", r_global_pools$variable)
  r_global_totals <- aggregate(value ~ year + variable, data = r_global_pools, sum)

  invisible(reset(core))
  fsplit <- 0.1
  split_biome(core, "default", c("non-pf", "permafrost"), fveg_c = c(1 - fsplit, fsplit))
  expect_equal(get_biome_list(core), c("non-pf", "permafrost"))

  # The non-pool settings should be identical for the biomes
  x <- hector:::get_biome_inits(core, "non-pf")
  y <- hector:::get_biome_inits(core, "permafrost")
  expect_identical(x[F_NPPD()], y[F_NPPD()])
  expect_identical(x[F_NPPV()], y[F_NPPV()])
  expect_identical(x[F_LITTERD()], y[F_LITTERD()])
  expect_identical(x[BETA()], y[BETA()])
  expect_identical(x[Q10_RH()], y[Q10_RH()])
  expect_identical(x[WARMINGFACTOR()], y[WARMINGFACTOR()])

  default_veg <- fetchvars(core, dates = NA, vars = VEG_C("non-pf"))[["value"]]
  permafrost_veg <- fetchvars(core, dates = NA, vars = VEG_C("permafrost"))[["value"]]

  # Also check that trying to get global `VEG_C()` will retrieve the total
  global_veg2 <- fetchvars(core, dates = NA, vars = VEG_C())[["value"]]
  global_veg3 <- fetchvars(core, dates = NA, vars = VEG_C("global"))[["value"]]
  expect_equal(global_veg2, global_veg3)
  expect_equivalent(default_veg, (1 - fsplit) * global_veg2)
  expect_equivalent(permafrost_veg, fsplit * global_veg2)
  expect_equivalent(default_veg + permafrost_veg, global_veg2)

  # Sum of biome-specific pools should exactly equal global results at
  # each time step. This is a stringent test!
  reset(core, core$reset_date)
  invisible(run(core))
  r_biome <- fetchvars(core, 2000:2100)
  # Climate should be the same
  expect_equivalent(r_global, r_biome)
  # Summed pools should be the same
  r_biome_data <- fetchvars(core, 2000:2100, c(VEG_C("non-pf"), VEG_C("permafrost"),
                                               DETRITUS_C("non-pf"), DETRITUS_C("permafrost"),
                                               SOIL_C("non-pf"), SOIL_C("permafrost")),
                            scenario = "default pf")
  r_biome_data$biome <- gsub("^(.*)\\.(.*)", "\\1", r_biome_data$variable)
  r_biome_data$variable <- gsub("^(.*)\\.(.*)", "\\2", r_biome_data$variable)
  r_biome_totals <- aggregate(value ~ year + variable, data = r_biome_data, sum)
  expect_equivalent(r_global_totals, r_biome_totals)

  # Certain changes should always produce higher atmospheric CO2

  test_higher_co2 <- function(var_f, value) {
    core <- ssp245()
    orig_val <- fetchvars(core, NA, var_f())[["value"]]
    invisible(run(core))
    basic <- fetchvars(core, 2000:2100, CONCENTRATIONS_CO2())
    # Create two biomes, change one of the parameters
    invisible(split_biome(core, "global", c("a", "b")))
    setvar(core, NA, var_f("b"), value, NA)
    # Check that only the one parameter was changed
    expect_equal(fetchvars(core, NA, var_f("a"))[["value"]], orig_val)
    expect_equal(fetchvars(core, NA, var_f("b"))[["value"]], value)
    reset(core, core$reset_date)
    invisible(run(core))
    # Check that the new result had higher CO2 than original one
    new <- fetchvars(core, 2000:2100, CONCENTRATIONS_CO2())
    expect_true(all(basic[["value"]] < new[["value"]]))
  }

  # Lower beta means higher CO2
  test_higher_co2(BETA, 0.1)
  # Higher Q10 means higher CO2
  test_higher_co2(Q10_RH, 4)
  # Higher f_nppd means higher CO2 (because detritus respires faster than soil)
  test_higher_co2(F_NPPD, 0.64)
  # Higher f_litterd means higher CO2 (same reason)
  test_higher_co2(F_LITTERD, 0.99)
})

test_that("More than 2 biomes", {
    save_this_year <- 2020
    core <- ssp245()
    invisible(run(core))
    global_vegc <- fetchvars(core, dates = save_this_year, VEG_C())

    # Using default arguments
    invisible(reset(core))
    biomes <- paste0("b", 1:5)
    veg_c_biomes <- vapply(biomes, VEG_C, character(1))
    split_biome(core, "global", biomes)
    expect_equal(get_biome_list(core), biomes)
    invisible(reset(core))
    invisible(run(core))

    biome_vegc <- fetchvars(core, save_this_year, veg_c_biomes)
    expect_equivalent(sum(biome_vegc[["value"]]), global_vegc[["value"]])
})
