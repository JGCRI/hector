# Identifiers for quantities in the methane component

These identifiers correspond to variables that can be read and/or set in
the methane component.

## Usage

``` r
CONCENTRATIONS_CH4()

PREINDUSTRIAL_CH4()

EMISSIONS_CH4()

NATURAL_CH4()

LIFETIME_SOIL()

LIFETIME_STRAT()
```

## Output variables

These variables can be read from the methane component.

- CONCENTRATIONS_CH4:

  Atmospheric methane concentration

- PREINDUSTRIAL_CH4:

  Preindustrial methane concentration

## Input variables

These variables can be set in the methane component. The expected units
string is given after each description.

- EMISSIONS_CH4:

  Methane emissions (`"Tg CH4"`)

- PREINDUSTRIAL_CH4:

  Preindustrial methane concentration (`"ppbv CH4"`)

- CH4_CONSTRAIN:

  N2O concentration constraint (`"ppbv CH4"`)

- NATURAL_CH4:

  Natural methane emissions (`"Tg CH4"`)

- LIFETIME_SOIL:

  Time scale for methane loss into soil (`"Years"`)

- LIFETIME_STRAT:

  Time scale for methane loss into stratosphere (`"Years"`)

## Note

Because these identifiers are provided as `#define` macros in the hector
code, these identifiers are provided in the R interface as functions.
Therefore, these objects must be called to use them; *e.g.*,
[`GETDATA()`](https://jgcri.github.io/hector/reference/msgtype.md)
instead of the more natural looking `GETDATA`.

## See also

Other capability identifiers:
[`CF4_CONSTRAIN()`](https://jgcri.github.io/hector/reference/haloconstrain.md),
[`CONCENTRATIONS_N2O()`](https://jgcri.github.io/hector/reference/concentrations.md),
[`DELTA_CO2()`](https://jgcri.github.io/hector/reference/delta.md),
[`EMISSIONS_BC()`](https://jgcri.github.io/hector/reference/emissions.md),
[`EMISSIONS_CF4()`](https://jgcri.github.io/hector/reference/haloemiss.md),
[`EMISSIONS_SO2()`](https://jgcri.github.io/hector/reference/so2.md),
[`FTOT_CONSTRAIN()`](https://jgcri.github.io/hector/reference/constraints.md),
[`GLOBAL_TAS()`](https://jgcri.github.io/hector/reference/temperature.md),
[`NBP()`](https://jgcri.github.io/hector/reference/carboncycle.md),
[`OCEAN_UPTAKE()`](https://jgcri.github.io/hector/reference/ocean.md),
[`RF_CF4()`](https://jgcri.github.io/hector/reference/haloforcings.md),
[`RF_TOTAL()`](https://jgcri.github.io/hector/reference/forcings.md),
[`RHO_BC()`](https://jgcri.github.io/hector/reference/rho.md),
[`TRACKING_DATE()`](https://jgcri.github.io/hector/reference/parameters.md)
