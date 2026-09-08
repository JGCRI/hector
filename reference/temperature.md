# Identifiers for variables associated with the temperature component

All of these variables can be read using the
[`GETDATA`](https://jgcri.github.io/hector/reference/msgtype.md) message
type.

## Usage

``` r
GLOBAL_TAS()

GMST()

SST()

OCEAN_TAS()

LAND_TAS()

FLUX_MIXED()

FLUX_INTERIOR()

HEAT_FLUX()
```

## Functions

- `GLOBAL_TAS()`: Global mean air temperature anomaly

- `GMST()`: Global mean surface temperature anomaly

- `SST()`: Average sea surface temperature anomaly

- `OCEAN_TAS()`: Average air temperature anomaly over the ocean

- `LAND_TAS()`: Average air temperature anomaly over land, land surface
  temperature and air temperature over land are assumed to be
  equivalent.

- `FLUX_MIXED()`: Heat flux into the mixed layer of the ocean

- `FLUX_INTERIOR()`: Heat flux into the interior layer of the ocean

- `HEAT_FLUX()`: Total heat flux into the ocean

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
[`EMISSIONS_SO2()`](https://jgcri.github.io/hector/reference/so2.md),
[`FTOT_CONSTRAIN()`](https://jgcri.github.io/hector/reference/constraints.md),
[`NBP()`](https://jgcri.github.io/hector/reference/carboncycle.md),
[`OCEAN_UPTAKE()`](https://jgcri.github.io/hector/reference/ocean.md),
[`RF_CF4()`](https://jgcri.github.io/hector/reference/haloforcings.md),
[`RF_TOTAL()`](https://jgcri.github.io/hector/reference/forcings.md),
[`RHO_BC()`](https://jgcri.github.io/hector/reference/rho.md),
[`TRACKING_DATE()`](https://jgcri.github.io/hector/reference/parameters.md)
