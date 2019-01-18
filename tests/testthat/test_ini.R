context("Reading and writing INI files")

tempdir <- tempfile()
dir.create(tempdir, showWarnings = FALSE)
teardown({
  unlink(tempdir, recursive = TRUE)
})

test_that("RCP4.5 is read and written correctly", {
  correct_names <- c(
    "core",
    "onelineocean", "ocean",
    "simpleNbox", "carbon-cycle-solver",
    "so2", "CH4", "OH", "ozone", "N2O",
    "forcing", "temperature",
    "bc", "oc",
    "CF4_halocarbon", "C2F6_halocarbon",
    "HFC23_halocarbon", "HFC32_halocarbon",
    "HFC4310_halocarbon", "HFC125_halocarbon", "HFC134a_halocarbon",
    "HFC143a_halocarbon", "HFC227ea_halocarbon", "HFC245fa_halocarbon",
    "SF6_halocarbon", "CFC11_halocarbon", "CFC12_halocarbon",
    "CFC113_halocarbon", "CFC114_halocarbon", "CFC115_halocarbon",
    "CCl4_halocarbon", "CH3CCl3_halocarbon", "halon1211_halocarbon",
    "halon1301_halocarbon", "halon2402_halocarbon", "HCF22_halocarbon",
    "HCF141b_halocarbon", "HCF142b_halocarbon", "CH3Cl_halocarbon",
    "CH3Br_halocarbon")
  rcp45_ini <- read_ini(
    system.file("input", "hector_rcp45.ini", package = "hector")
  )
  expect_named(rcp45_ini, correct_names)
  invisible(lapply(rcp45_ini, expect_named))
  expect_equal(rcp45_ini[[c("core", "run_name")]], "rcp45")
  expect_equal(rcp45_ini[[c("onelineocean", "enabled")]], 0)

  # Time series should be converted to albedo
  ftalbedo <- rcp45_ini[[c("simpleNbox", "Ftalbedo")]]
  expect_s3_class(ftalbedo, "data.frame")
  expect_named(ftalbedo, c("date", "Ftalbedo"))
  expect_equal(nrow(ftalbedo), 2)

  # CSV path is normalized to a full path
  ffi_emission <- rcp45_ini[[c("simpleNbox", "ffi_emissions")]]
  expect_match(ffi_emission, "csv:/")
  expect_true(file.exists(gsub("csv:", "", ffi_emission)))

  rcp45_testfile <- file.path(tempdir, "rcp45.ini")
  write_ini(rcp45_ini, rcp45_testfile)
  expect_true(file.exists(rcp45_testfile))

  raw_out <- readLines(rcp45_testfile)
  expect_equal(raw_out[2], "[core]")
  expect_equal(raw_out[33], "Ftalbedo[1750] =  0.0")

  # Make sure these are not converted to scientific notation
  expect_equal(raw_out[14], "tt = 72000000")
  expect_equal(raw_out[15], "tu = 49000000")
  expect_equal(raw_out[17], "tid = 200000000")

  rcp45_test <- read_ini(rcp45_testfile)
  expect_identical(rcp45_ini, rcp45_test)
})


test_that("Passing INI as file or list produces same results.", {
  for (rcp in c("26", "45", "60", "85")) {
    ini_file <- system.file("input", sprintf("hector_rcp%s.ini", rcp),
                            package = "hector")
    file_out <- runscenario(ini_file, suppresslogging = TRUE)
    ini <- read_ini(ini_file)
    ini_result <- runscenario(ini, suppresslogging = TRUE)
    expect_identical(file_out, ini_result, info = paste0("RCP", rcp))

    # Make a trivial modification -- change the run name
    ini2 <- modifyList(ini, list(core = list(run_name = "rcp26_mod")))
    ini2_result <- runscenario(ini2, suppresslogging = TRUE)
    expect_identical(ini2_result, file_out, info = paste0("RCP", rcp))
  }
})
