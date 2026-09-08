# Identifiers for quantities in the ocean component

These identifiers correspond to variables that can be read from the
ocean component using the
[`GETDATA`](https://jgcri.github.io/hector/reference/msgtype.md) message
type.

## Usage

``` r
OCEAN_UPTAKE()

OCEAN_C()

OCEAN_C_HL()

OCEAN_C_LL()

OCEAN_C_ML()

OCEAN_C_IO()

OCEAN_C_DO()

OCEAN_PREIND_C_SURF()

OCEAN_PREIND_C_ID()

TT()

TU()

TWI()

TID()

PH_HL()

PH_LL()

PH()

HL_OCEAN_UPTAKE()

LL_OCEAN_UPTAKE()

PCO2_HL()

PCO2_LL()

PCO2()

DIC_HL()

DIC()

DIC_LL()

SST_HL()

SST_LL()

CO3_LL()

CO3_HL()

CO3()
```

## Functions

- `OCEAN_UPTAKE()`: Atmosphere-ocean carbon flux

- `OCEAN_C()`: Ocean total carbon pool

- `OCEAN_C_HL()`: Ocean surface high-latitude carbon pool

- `OCEAN_C_LL()`: Ocean surface low-latitude carbon pool

- `OCEAN_C_ML()`: Ocean surface or mixed layer carbon pool

- `OCEAN_C_IO()`: Intermediate ocean carbon pool

- `OCEAN_C_DO()`: Deep ocean carbon pool

- `OCEAN_PREIND_C_SURF()`: Preindustrial surface ocean carbon pool

- `OCEAN_PREIND_C_ID()`: Preindustrial intermediate+deep ocean carbon
  pool

- `TT()`: Thermohaline overturning

- `TU()`: High-latitude overturning

- `TWI()`: Warm-intermediate exchange

- `TID()`: Intermediate-deep exchange

- `PH_HL()`: High-latitude pH

- `PH_LL()`: Low-latitude pH

- `PH()`: Ocean surface pH

- `HL_OCEAN_UPTAKE()`: Atmosphere-ocean carbon flux, high-latitude

- `LL_OCEAN_UPTAKE()`: Atmosphere-ocean carbon flux, low-latitude

- `PCO2_HL()`: Partial pressure of CO2, high-latitude

- `PCO2_LL()`: Partial pressure of CO2, low-latitude

- `PCO2()`: Ocean surface partial pressure of CO2

- `DIC_HL()`: Dissolved inorganic carbon, high-latitude

- `DIC()`: Ocean surface dissolved inorganic carbon

- `DIC_LL()`: Dissolved inorganic carbon, low-latitude

- `SST_HL()`: Absolute ocean surface temperature, high-latitude (deg C)

- `SST_LL()`: Absolute ocean surface temperature, low-latitude (deg C)

- `CO3_LL()`: Carbonate concentration, low-latitude

- `CO3_HL()`: Carbonate concentration, high-latitude

- `CO3()`: Ocean surface carbonate concentration

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
[`RF_CF4()`](https://jgcri.github.io/hector/reference/haloforcings.md),
[`RF_TOTAL()`](https://jgcri.github.io/hector/reference/forcings.md),
[`RHO_BC()`](https://jgcri.github.io/hector/reference/rho.md),
[`TRACKING_DATE()`](https://jgcri.github.io/hector/reference/parameters.md)
