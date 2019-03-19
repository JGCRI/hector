library(hector)

hvars <- c(ECS(), PREINDUSTRIAL_CO2(), Q10_RH(), BETA(), AERO_SCALE(), VOLCANIC_SCALE(), DIFFUSIVITY(),
           FFI_EMISSIONS(), LUC_EMISSIONS(),
           EMISSIONS_BC(), EMISSIONS_N2O(), EMISSIONS_NOX(), EMISSIONS_CO(),
           EMISSIONS_NMVOC(), EMISSIONS_OC(),
           EMISSIONS_CH4(), PREINDUSTRIAL_CH4(), NATURAL_CH4(), LIFETIME_SOIL(), LIFETIME_STRAT(),
           EMISSIONS_SO2(), VOLCANIC_SO2())


hunits <- c('degC', 'ppmv CO2', '(unitless)', '(unitless)', '(unitless)', '(unitless)', 'cm2/s',
            'Pg C/yr', 'Pg C/yr',
            'Tg', 'Tg N', 'Tg N', 'Tg CO',
            'Tg NMVOC', 'Tg',
            'Tg CH4', 'ppbv CH4', 'Tg CH4', 'Years', 'Years',
            'Gg S', 'W/m2')


haloemis <- c(EMISSIONS_CF4(), EMISSIONS_C2F6(), EMISSIONS_HFC23(),
              EMISSIONS_HFC32(), EMISSIONS_HFC4310(), EMISSIONS_HFC125(),
              EMISSIONS_HFC134A(), EMISSIONS_HFC143A(), EMISSIONS_HFC227EA(),
              EMISSIONS_HFC245FA(), EMISSIONS_SF6(), EMISSIONS_CFC11(),
              EMISSIONS_CFC12(), EMISSIONS_CFC113(), EMISSIONS_CFC114(),
              EMISSIONS_CFC115(), EMISSIONS_CCL4(), EMISSIONS_CH3CCL3(),
              EMISSIONS_HCF22(), EMISSIONS_HCF141B(), EMISSIONS_HCF142B(),
              EMISSIONS_HALON1211(), EMISSIONS_HALON1301(),
              EMISSIONS_HALON2402(), EMISSIONS_CH3CL(), EMISSIONS_CH3BR())
halounits <- 'Gg'


unitstable <- rbind(data.frame(variable=hvars, units=hunits),
                    data.frame(variable=haloemis, units=halounits))

devtools::use_data(unitstable, internal=TRUE, overwrite=TRUE)
