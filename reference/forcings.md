# Identifiers for capabilities in the Hector forcing component

These identifiers specify forcing values that can be provided by hector
via the forcing component. All of the values corresponding to these
identifiers are read-only (*i.e.*, they can only appear in
[`GETDATA`](https://jgcri.github.io/hector/reference/msgtype.md)
messages.)

## Usage

``` r
RF_TOTAL()

RF_ALBEDO()

RF_CO2()

RF_H2O_STRAT()

RF_O3_TROP()

RF_BC()

RF_OC()

RF_NH3()

RF_SO2()

RF_ACI()

RF_VOL()

RF_MISC()

Q_CO2()

RF_CH4()
```

## Functions

- `RF_TOTAL()`: Total radiative forcing

- `RF_ALBEDO()`: Radiative forcing due to albedo

- `RF_CO2()`: Radiative forcing due to CO2

- `RF_H2O_STRAT()`: Radiative forcing due to stratospheric water vapor

- `RF_O3_TROP()`: Radiative forcing due to tropospheric ozone

- `RF_BC()`: Radiative forcing due to black carbon aerosol-radiation
  interactions

- `RF_OC()`: Radiative forcing due to organic carbon aerosol-radiation
  interactions

- `RF_NH3()`: Radiative forcing due to NH3 aerosol-radiation
  interactions

- `RF_SO2()`: Radiative forcing due to SO2 aerosol-radiation
  interactions

- `RF_ACI()`: aerosol-cloud interactions, includes contributions from
  SO2, BC and OC

- `RF_VOL()`: Radiative forcing due to volcanic activity

- `RF_MISC()`: Radiative miscellaneous forcing read in from ini.

- `Q_CO2()`: Radiative forcing efficiency for doubling of CO2

- `RF_CH4()`: Radiative forcing due to methane

## Note

Because these identifiers are provided as `#define` macros in the hector
code, these identifiers are provided in the R interface as functions.
Therefore, these objects must be called to use them; *e.g.*,
[`GETDATA()`](https://jgcri.github.io/hector/reference/msgtype.md)
instead of the more natural looking `GETDATA`.

## See also

[haloforcings](https://jgcri.github.io/hector/reference/haloforcings.md)
for forcings from halocarbons and
[rho](https://jgcri.github.io/hector/reference/rho.md) for parameters
related to radiative forcing efficiency.

Other capability identifiers:
[`CF4_CONSTRAIN()`](https://jgcri.github.io/hector/reference/haloconstrain.md),
[`CONCENTRATIONS_CH4()`](https://jgcri.github.io/hector/reference/methane.md),
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
[`RHO_BC()`](https://jgcri.github.io/hector/reference/rho.md),
[`TRACKING_DATE()`](https://jgcri.github.io/hector/reference/parameters.md)
