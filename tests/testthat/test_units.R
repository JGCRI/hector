context('Units table')

test_that('Units retrieval works.', {
    vars <- c(ECS(), EMISSIONS_SO2(), BETA(), VOLCANIC_SO2(), FFI_EMISSIONS(),
              LIFETIME_SOIL())
    units_true <- c('degC', 'Gg S', '(unitless)', 'W/m2', 'Pg C/yr', 'Years')

    units <- getunits(vars)

    expect_equal(units, units_true)
})

test_that('Invalid units retrieval warns.', {
    vars <- c(ECS(), EMISSIONS_SO2(), ATMOSPHERIC_C(), BETA())
    units_true <- c('degC', 'Gg S', NA, '(unitless)')
    expect_warning(units <- getunits(vars),
                   'entries for the following variables not found')
    expect_equal(units, units_true)
})
