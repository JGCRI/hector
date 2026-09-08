# Identifiers for quantities in the SO2 component

These identifiers correspond to variables that can be read and/or set in
the SO2 component

## Usage

``` r
EMISSIONS_SO2()

VOLCANIC_SO2()
```

## Output variables

These variables can be read using the
[`GETDATA`](https://jgcri.github.io/hector/reference/msgtype.md) message
type.

- NATURAL_SO2:

  Natural SO2 emissions

- Y2000_SO2:

  Year 2000 SO2 emissions

- EMISIONS_SO2:

  Anthropogenic SO2 emissions

- VOLCANIC_SO2:

  Forcing due to volcanic SO2 emissions

## Input variables

These variables can be set using the
[`SETDATA`](https://jgcri.github.io/hector/reference/msgtype.md) message
type.

- EMISSIONS_SO2:

  Anthropogenic SO2 emissions (`"Gg S"`)

- VOLCANIC_SO2:

  Forcing due to volcanic SO2 emissions (`"W/m2"`)

## Note

Because these identifiers are provided as `#define` macros in the hector
code, these identifiers are provided in the R interface as functions.
Therefore, these objects must be called to use them; *e.g.*,
[`GETDATA()`](https://jgcri.github.io/hector/reference/msgtype.md)
instead of the more natural looking `GETDATA`.

## See also

Other capability identifiers:
[`CF4_CONSTRAIN()`](https://jgcri.github.io/hector/reference/haloconstrain.md),
[`CONCENTRATIONS_CH4()`](https://jgcri.github.io/hector/reference/methane.md),
[`CONCENTRATIONS_N2O()`](https://jgcri.github.io/hector/reference/concentrations.md),
[`DELTA_CO2()`](https://jgcri.github.io/hector/reference/delta.md),
[`EMISSIONS_BC()`](https://jgcri.github.io/hector/reference/emissions.md),
[`EMISSIONS_CF4()`](https://jgcri.github.io/hector/reference/haloemiss.md),
[`FTOT_CONSTRAIN()`](https://jgcri.github.io/hector/reference/constraints.md),
[`GLOBAL_TAS()`](https://jgcri.github.io/hector/reference/temperature.md),
[`NBP()`](https://jgcri.github.io/hector/reference/carboncycle.md),
[`OCEAN_UPTAKE()`](https://jgcri.github.io/hector/reference/ocean.md),
[`RF_CF4()`](https://jgcri.github.io/hector/reference/haloforcings.md),
[`RF_TOTAL()`](https://jgcri.github.io/hector/reference/forcings.md),
[`RHO_BC()`](https://jgcri.github.io/hector/reference/rho.md),
[`TRACKING_DATE()`](https://jgcri.github.io/hector/reference/parameters.md)
