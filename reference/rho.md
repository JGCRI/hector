# Identifiers for Hector forcing component parameters

These identifiers specify radiative forcing efficiency in hector these
values can be read and/or set by hectors forcing and halocarbon
components. the forcing component.

## Usage

``` r
RHO_BC()

RHO_OC()

RHO_NH3()

RHO_SO2()

RHO_CF4()

RHO_C2F6()

RHO_HFC23()

RHO_HFC32()

RHO_HFC4310()

RHO_HFC125()

RHO_HFC134A()

RHO_HFC143A()

RHO_HFC227EA()

RHO_HFC245FA()

RHO_SF6()

RHO_CFC11()

RHO_CFC12()

RHO_CFC113()

RHO_CFC114()

RHO_CFC115()

RHO_CCL4()

RHO_CH3CCL3()

RHO_HCFC22()

RHO_HCFC141B()

RHO_HCFC142B()

RHO_HALON1211()

RHO_HALON1301()

RHO_HALON2402()

RHO_CH3CL()

RHO_CH3BR()
```

## Functions

- `RHO_BC()`: a radiative forcing efficiency for BC aerosol-radiation
  interactions

- `RHO_OC()`: a radiative forcing efficiency for OC aerosol-radiation
  interactions

- `RHO_NH3()`: a radiative forcing efficiency for NH3 aerosol-radiation
  interactions

- `RHO_SO2()`: a radiative forcing efficiency for SO2

- `RHO_CF4()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for CF4

- `RHO_C2F6()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for C2F6

- `RHO_HFC23()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for HFC-23

- `RHO_HFC32()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for HFC-32

- `RHO_HFC4310()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for HFC-4310

- `RHO_HFC125()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for HFC-125

- `RHO_HFC134A()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for HFC-134a

- `RHO_HFC143A()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for HFC-143a

- `RHO_HFC227EA()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for HFC-227ea

- `RHO_HFC245FA()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for HFC-254fa

- `RHO_SF6()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for sulfur hexafluoride

- `RHO_CFC11()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for CFC-11

- `RHO_CFC12()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for CFC-12

- `RHO_CFC113()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for CFC-113

- `RHO_CFC114()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for CFC-114

- `RHO_CFC115()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for CFC-115

- `RHO_CCL4()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for carbon tetrachloride

- `RHO_CH3CCL3()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for trichloroethane '

- `RHO_HCFC22()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for HFC-22

- `RHO_HCFC141B()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for HFC-141b

- `RHO_HCFC142B()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for HCFC-142b

- `RHO_HALON1211()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for halon-1211 '

- `RHO_HALON1301()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for halon-1301

- `RHO_HALON2402()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for halon-2402 '

- `RHO_CH3CL()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for chloromethane

- `RHO_CH3BR()`: a radiative forcing efficiency for user-specified
  preindustrial concentration (Wm-2 pptv-1) for bromomethane

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
[`TRACKING_DATE()`](https://jgcri.github.io/hector/reference/parameters.md)
