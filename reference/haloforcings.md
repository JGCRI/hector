# Identifiers for halocarbon forcings

These identifiers specify forcing values that can be provided by hector
via one of the myriad halocarbon components. All of the values
corresponding to these identifiers are read-only (*i.e.*, they can only
appear in
[`GETDATA`](https://jgcri.github.io/hector/reference/msgtype.md)
messages.) The forcings returned are the *relative* forcings, with the
base year (typically 1750) values subtracted off.

## Usage

``` r
RF_CF4()

RF_C2F6()

RF_HFC23()

RF_HFC32()

RF_HFC4310()

RF_HFC125()

RF_HFC134A()

RF_HFC143A()

RF_HFC227EA()

RF_HFC245FA()

RF_SF6()

RF_CFC11()

RF_CFC12()

RF_CFC113()

RF_CFC114()

RF_CFC115()

RF_CCL4()

RF_CH3CCL3()

RF_HCFC22()

RF_HCFC141B()

RF_HCFC142B()

RF_HALON1211()

RF_HALON1301()

RF_HALON2402()

RF_CH3CL()

RF_CH3BR()
```

## Functions

- `RF_CF4()`: Radiative forcing due to CF4

- `RF_C2F6()`: Radiative forcing due to C2F6

- `RF_HFC23()`: Radiative forcing due to HFC-23

- `RF_HFC32()`: Radiative forcing due to HFC-32

- `RF_HFC4310()`: Radiative forcing due to HFC-4310

- `RF_HFC125()`: Radiative forcing due to HFC-125

- `RF_HFC134A()`: Radiative forcing due to HFC-134a

- `RF_HFC143A()`: Radiative forcing due to HFC-143a

- `RF_HFC227EA()`: Radiative forcing due to HFC227ea

- `RF_HFC245FA()`: Radiative forcing due to HFC-245fa

- `RF_SF6()`: Radiative forcing due to sulfur hexafluoride

- `RF_CFC11()`: Radiative forcing due to CFC-11

- `RF_CFC12()`: Radiative forcing due to CFC-12

- `RF_CFC113()`: Radiative forcing due to CFC-113

- `RF_CFC114()`: Radiative forcing due to CFC-114

- `RF_CFC115()`: Radiative forcing due to CFC-115

- `RF_CCL4()`: Radiative forcing due to carbon tetrachloride

- `RF_CH3CCL3()`: Radiative forcing due to trichloroethane

- `RF_HCFC22()`: Radiative forcing due to HCFC-22

- `RF_HCFC141B()`: Radiative forcing due to HCFC-141b

- `RF_HCFC142B()`: Radiative forcing due to HCCFC-142b

- `RF_HALON1211()`: Radiative forcing due to halon-1211

- `RF_HALON1301()`: Radiative forcing due to halon-1301

- `RF_HALON2402()`: Radiative forcing due to halon-2402

- `RF_CH3CL()`: Radiative forcing due to chloromethane

- `RF_CH3BR()`: Radiative forcing due to bromomethane

## Note

Because these identifiers are provided as `#define` macros in the hector
code, these identifiers are provided in the R interface as functions.
Therefore, these objects must be called to use them; *e.g.*,
[`GETDATA()`](https://jgcri.github.io/hector/reference/msgtype.md)
instead of the more natural looking `GETDATA`.

## See also

[forcings](https://jgcri.github.io/hector/reference/forcings.md) for
forcings from other sources.

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
[`RF_TOTAL()`](https://jgcri.github.io/hector/reference/forcings.md),
[`RHO_BC()`](https://jgcri.github.io/hector/reference/rho.md),
[`TRACKING_DATE()`](https://jgcri.github.io/hector/reference/parameters.md)
