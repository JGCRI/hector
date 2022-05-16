context("Hector core-handling functions")

inputdir <- system.file("input", package = "hector")
inifile <- file.path(inputdir, "hector_ssp245.ini")

test_that("Running a single scenario returns proper outputs", {

    core <- newcore(inifile)
    sce <- hector:::runscenario(inifile)

    # Check that results are a data.frame
    expect_s3_class(sce, "data.frame")

    # Check that all years are included
    start_date <- core$strtdate
    end_date <- core$enddate
    years <- c(start_date:end_date)
    data_years <- unique(sce$year)
    # Equal, not identical, because of differing classes
    expect_equal(data_years, years)

    # Test that all variables are present
    vars <- c("CO2_conc", "RF_tot", "RF_CO2", "Tgav")
    expect_identical(unique(sce$variable), vars)

    # Test that units are correct
    units <- c("ppmv CO2", "W/m2", "degC")
    expect_identical(unique(sce$units), units)

    shutdown(core)
})

test_that("Object is an hcore instance (for multiple functions)", {

    # Test that if function does not receive an hcore class it returns an error
    expect_error(hector:::isactive("core"),
                 regexp = "Object supplied is not an hcore class instance.")
    expect_error(hector:::startdate("core"),
                 regexp = "Object supplied is not an hcore class instance.")
    expect_error(hector:::enddate("core"),
                 regexp = "Object supplied is not an hcore class instance.")

})

test_that("Inactive core returns error", {

    # Test that if core is inactive, function returns an error
    core <- newcore(inifile)
    run(core, 1760)
    shutdown(core)
    expect_match(hector:::format.hcore(core), regexp = "INACTIVE")

})

test_that("print.hcore works properly", {

    # Test that print.hcore prints correctly
    core <- newcore(inifile)
    expect_output(print(core))

    shutdown(core)

})
