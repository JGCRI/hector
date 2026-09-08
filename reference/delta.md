# Identifiers for Hector forcing component parameters

These identifiers specify the tropospheric adjustments for the
stratospheric-temperature adjusted radiative forcings. These values must
be a number between -1 and 1, and can be read and/or set by hectors
forcing and halocarbon components.

## Usage

``` r
DELTA_CO2()

RF_N2O()

DELTA_N2O()

DELTA_CF4()

DELTA_C2F6()

DELTA_HFC23()

DELTA_HFC32()

DELTA_HFC4310()

DELTA_HFC125()

DELTA_HFC134A()

DELTA_HFC143A()

DELTA_HFC227EA()

DELTA_HFC245FA()

DELTA_SF6()

DELTA_CFC11()

DELTA_CFC12()

DELTA_CFC113()

DELTA_CFC114()

DELTA_CFC115()

DELTA_CCL4()

DELTA_CH3CCL3()

DELTA_HCFC22()

DELTA_HCFC141B()

DELTA_HCFC142B()

DELTA_HALON1211()

DELTA_HALON1301()

DELTA_HALON2402()

DELTA_CH3CL()

DELTA_CH3BR()

DELTA_CH4()
```

## Functions

- `DELTA_CO2()`: the foring tropospheric adjustments for N2O

- `RF_N2O()`: the foring tropospheric adjustments for N2O

- `DELTA_N2O()`: Radiative forcing due to N2O

- `DELTA_CF4()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_C2F6()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_HFC23()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_HFC32()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing ' @export

- `DELTA_HFC4310()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_HFC125()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_HFC134A()`: the tropospheric adjustments used to convert from
  tratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_HFC143A()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_HFC227EA()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_HFC245FA()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_SF6()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_CFC11()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_CFC12()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_CFC113()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_CFC114()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_CFC115()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_CCL4()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_CH3CCL3()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_HCFC22()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_HCFC141B()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_HCFC142B()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_HALON1211()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_HALON1301()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_HALON2402()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_CH3CL()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_CH3BR()`: the tropospheric adjustments used to convert from
  stratospheric-temperature adjusted radiative forcing to effective
  radiative forcing

- `DELTA_CH4()`: Radiative forcing tropospheric adjustment for CH4

## Note

Because these identifiers are provided as `#define` macros in the hector
code, these identifiers are provided in the R interface as functions.
Therefore, these objects must be called to use them; *e.g.*,
[`GETDATA()`](https://jgcri.github.io/hector/reference/msgtype.md)
instead of the more natural looking `GETDATA`.

## See also

[haloforcings](https://jgcri.github.io/hector/reference/haloforcings.md)
for forcings from halocarbons and
[forcings](https://jgcri.github.io/hector/reference/forcings.md) forcing
values provided from the hector forcing component.

Other capability identifiers:
[`CF4_CONSTRAIN()`](https://jgcri.github.io/hector/reference/haloconstrain.md),
[`CONCENTRATIONS_CH4()`](https://jgcri.github.io/hector/reference/methane.md),
[`CONCENTRATIONS_N2O()`](https://jgcri.github.io/hector/reference/concentrations.md),
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
