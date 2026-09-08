# Identifiers for model parameters

These identifiers correspond to settable parameters that change the
model behavior and are subject to uncertainty. All of these can be set
using the
[`SETDATA`](https://jgcri.github.io/hector/reference/msgtype.md) message
type. Changing any of these parameters will typically invalidate the
hector core's internal state; therefore, after setting one or more of
these values you should call
[`reset`](https://jgcri.github.io/hector/reference/reset.md) before
attempting to run the model again. This will rerun the spinup and
produce a new internally consistent state. Attempting to run the model
without resetting first will usually produce an error (often with a
message about failing to conserve mass).

## Usage

``` r
TRACKING_DATE()

PREINDUSTRIAL_CO2()

BETA(biome = "")

Q10_RH(biome = "")

WARMINGFACTOR(biome = "")

F_NPPV(biome = "")

F_NPPD(biome = "")

F_LITTERD(biome = "")

ECS()

AERO_SCALE()

VOLCANIC_SCALE()

LO_WARMING_RATIO()

DIFFUSIVITY()
```

## Arguments

- biome:

  Biome for which to retrieve parameter. If missing or \`""\`, default
  to \`"global"\`.

## Functions

- `TRACKING_DATE()`: Start of carbon tracking (Year)

- `PREINDUSTRIAL_CO2()`: Preindustrial CO2 concentration (`"ppmv CO2"`)

- `BETA()`: CO2 fertilization factor (`"(unitless)"`)

- `Q10_RH()`: Heterotrophic respiration temperature sensitivity factor
  (`"(unitless)"`)

- `WARMINGFACTOR()`: Biome-specific warming factor (\`(unitless)\`)

- `F_NPPV()`: NPP fraction to vegetation (`"(unitless)"`)

- `F_NPPD()`: NPP fraction to detritus (`"(unitless)"`)

- `F_LITTERD()`: Litter fraction to detritus (`"(unitless)"`)

- `ECS()`: Equilibrium Climate Sensitivity (`"degC"`)

- `AERO_SCALE()`: Aerosol forcing scaling factor (`"(unitless)"`)

- `VOLCANIC_SCALE()`: Volcanic forcing scaling factor (`"(unitless)"`)

- `LO_WARMING_RATIO()`: Land-Ocean Warming Ratio (`"(unitless)"`), by
  default set to 0 meaning that the land ocean warming ratio is an
  emergent property of Hector's temperature component otherwise the user
  defined land ocean warming ratio will be used.

- `DIFFUSIVITY()`: Ocean heat diffusivity (`"cm2/s"`)

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
[`GLOBAL_TAS()`](https://jgcri.github.io/hector/reference/temperature.md),
[`NBP()`](https://jgcri.github.io/hector/reference/carboncycle.md),
[`OCEAN_UPTAKE()`](https://jgcri.github.io/hector/reference/ocean.md),
[`RF_CF4()`](https://jgcri.github.io/hector/reference/haloforcings.md),
[`RF_TOTAL()`](https://jgcri.github.io/hector/reference/forcings.md),
[`RHO_BC()`](https://jgcri.github.io/hector/reference/rho.md)
