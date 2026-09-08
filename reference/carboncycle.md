# Identifiers for variables in the Hector carbon cycle component

These identifiers correspond to variables that can be read and/or set in
the carbon cycle

## Usage

``` r
NBP()

CONCENTRATIONS_CO2()

NPP()

RH()

RH_DETRITUS()

RH_SOIL()

ATMOSPHERIC_CO2()

FFI_EMISSIONS()

DACCS_UPTAKE()

LUC_EMISSIONS()

LUC_UPTAKE()

CO2_CONSTRAIN()

VEG_C(biome = "")

DETRITUS_C(biome = "")

SOIL_C(biome = "")

PERMAFROST_C(biome = "")

THAWEDP_C(biome = "")

FRAC_FROZEN(biome = "")

FRAC_STATIC(biome = "")

PERMAFROST_MU(biome = "")

PERMAFROST_SIGMA(biome = "")

FRAC_DECOMP_CH4(biome = "")

EARTH_C()

NPP_FLUX0(biome = "")
```

## Arguments

- biome:

  Name of biome (leave empty for global)

## Functions

- `CO2_CONSTRAIN()`: Constrain atmospheric CO2 concentration
  (`"(ppmv CO2)"`)

- `VEG_C()`: Vegetation C pool (\`"Pg C"\`)

- `DETRITUS_C()`: Vegetation detritus C pool (\`"Pg C"\`)

- `SOIL_C()`: Soil C pool (\`"Pg C"\`)

- `PERMAFROST_C()`: Permafrost C pool (\`"Pg C"\`)

- `THAWEDP_C()`: Thawed permafrost C pool (\`"Pg C"\`)

- `FRAC_FROZEN()`: Fraction of permafrost still frozen (`"(unitless)"`)

- `FRAC_STATIC()`: Fraction of thawed permafrost that is static
  (`"(unitless)"`)

- `PERMAFROST_MU()`: Permafrost thaw mu parameter (`"(unitless)"`)

- `PERMAFROST_SIGMA()`: Permafrost thaw sigma parameter (`"(unitless)"`)

- `FRAC_DECOMP_CH4()`: Methane fraction of permafrost decomposition
  (`"(unitless)"`)

- `NPP_FLUX0()`: Initial net primary productivity (NPP)flux (\`"Pg C
  year^-1"\`)

## Note

See Woodard et al. (2021)

See Woodard et al. (2021)

## Output variables

These variables can be read using the
[`GETDATA`](https://jgcri.github.io/hector/reference/msgtype.md) message
type:

- CONCENTRATIONS_CO2:

  Atmospheric CO2 concentration

- ATMOSPHERIC_CO2:

  Atmospheric CO2 in units of C

- NBP:

  Net biome production. Annual global C flux from atmosphere into the
  land. A positive value means a net flux from atmosphere into land
  (i.e. land is a net carbon sink), while a negative value means a net
  flux from land into the atmosphere (i.e. land is a net carbon source).
  Note that despite the name–which follows disciplinary convention, see
  Chapin et al. 2006–this is currently a globally-averaged variable.

- FFI_EMISSIONS:

  Fossil fuel and industrial emissions

- LUC_EMISSIONS:

  Land use change emissions

- NPP:

  Net primary production

- RH:

  Heterotrophic respiration

- EARTH_C:

  Earth pool

## Input variables

- FFI_EMISSIONS:

  Fossil fuel and industrial emissions (`"Pg C/yr"`)

- LUC_EMISSIONS:

  Land use change emissions (`"Pg C/yr"`)

- CO2_CONSTRAIN:

  Prescribed atmospheric CO2 concentration (`"ppmv CO2"`)

- DACCS_UPTAKE:

  Direct air carbon capture and storage

- LUC_UPTAKE:

  Land use change uptake

## Note

Because these identifiers are provided as `#define` macros in the hector
code, these identifiers are provided in the R interface as functions.
Therefore, these objects must be called to use them; *e.g.*,
[`GETDATA()`](https://jgcri.github.io/hector/reference/msgtype.md)
instead of the more natural looking `GETDATA`.

## See also

[concentrations](https://jgcri.github.io/hector/reference/concentrations.md)
for other gas concentrations and
[emissions](https://jgcri.github.io/hector/reference/emissions.md) for
other gas emissions

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
[`OCEAN_UPTAKE()`](https://jgcri.github.io/hector/reference/ocean.md),
[`RF_CF4()`](https://jgcri.github.io/hector/reference/haloforcings.md),
[`RF_TOTAL()`](https://jgcri.github.io/hector/reference/forcings.md),
[`RHO_BC()`](https://jgcri.github.io/hector/reference/rho.md),
[`TRACKING_DATE()`](https://jgcri.github.io/hector/reference/parameters.md)
