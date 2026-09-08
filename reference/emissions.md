# Identifiers for miscellaneous emissions not elsewhere described

All of these variables can be set using the
[`SETDATA`](https://jgcri.github.io/hector/reference/msgtype.md) message
type.

## Usage

``` r
EMISSIONS_BC()

EMISSIONS_N2O()

NAT_EMISSIONS_N2O()

EMISSIONS_NOX()

EMISSIONS_CO()

EMISSIONS_NMVOC()

EMISSIONS_OC()

EMISSIONS_NH3()
```

## Functions

- `EMISSIONS_BC()`: Black carbon emissions

- `EMISSIONS_N2O()`: N2O emissions (`"Tg N"`)

- `NAT_EMISSIONS_N2O()`: Natural N2O emissions (`"Tg N"`)

- `EMISSIONS_NOX()`: Emissions for NOx compounds (`"Tg N"`)

- `EMISSIONS_CO()`: Emissions for carbon monoxide (`"Tg CO"`)

- `EMISSIONS_NMVOC()`: Emissions for non-methane volatile organic
  compounds (NMVOC) (`"Tg NMVOC"`)

- `EMISSIONS_OC()`: Emissions for organic carbon (`"Tg"`)

- `EMISSIONS_NH3()`: Emissions for NH3 (`"Tg"`)

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
