# Identifiers for halocarbon concentration constraints

These identifiers correspond to concentration constraints for
halocarbons. In all cases, the expected input units are volumetric parts
per trillion (`"ppvt"`).

## Usage

``` r
CF4_CONSTRAIN()

C2F6_CONSTRAIN()

HFC23_CONSTRAIN()

HFC32_CONSTRAIN()

HFC4310_CONSTRAIN()

HFC125_CONSTRAIN()

HFC134A_CONSTRAIN()

HFC143A_CONSTRAIN()

HFC227EA_CONSTRAIN()

HFC245FA_CONSTRAIN()

SF6_CONSTRAIN()

CFC11_CONSTRAIN()

CFC12_CONSTRAIN()

CFC113_CONSTRAIN()

CFC114_CONSTRAIN()

CFC115_CONSTRAIN()

CCL4_CONSTRAIN()

CH3CCL3_CONSTRAIN()

HCFC22_CONSTRAIN()

HCFC141B_CONSTRAIN()

HCFC142B_CONSTRAIN()

HALON1211_CONSTRAIN()

HALON1301_CONSTRAIN()

HALON2402_CONSTRAIN()

CH3CL_CONSTRAIN()

CH3BR_CONSTRAIN()
```

## Functions

- `CF4_CONSTRAIN()`: Concentration constraint for CF4

- `C2F6_CONSTRAIN()`: Concentration constraint for C2F6

- `HFC23_CONSTRAIN()`: Concentration constraint for HFC-23

- `HFC32_CONSTRAIN()`: Concentration constraint for HFC-32

- `HFC4310_CONSTRAIN()`: Concentration constraint for HFC-4310

- `HFC125_CONSTRAIN()`: Concentration constraint for HFC-125

- `HFC134A_CONSTRAIN()`: Concentration constraint for HFC-134a

- `HFC143A_CONSTRAIN()`: Concentration constraint for HFC-143a

- `HFC227EA_CONSTRAIN()`: Concentration constraint for HFC-227ea

- `HFC245FA_CONSTRAIN()`: Concentration constraint for HFC-254fa

- `SF6_CONSTRAIN()`: Concentration constraint for sulfur hexafluoride

- `CFC11_CONSTRAIN()`: Concentration constraint for CFC-11

- `CFC12_CONSTRAIN()`: Concentration constraint for CFC-12

- `CFC113_CONSTRAIN()`: Concentration constraint for CFC-113

- `CFC114_CONSTRAIN()`: Concentration constraint for CFC-114

- `CFC115_CONSTRAIN()`: Concentration constraint for CFC-115

- `CCL4_CONSTRAIN()`: Concentration constraint for carbon tetrachloride

- `CH3CCL3_CONSTRAIN()`: Concentration constraint for trichloroethane

- `HCFC22_CONSTRAIN()`: Concentration constraint for HCFC-22

- `HCFC141B_CONSTRAIN()`: Concentration constraint for HCFC-141b

- `HCFC142B_CONSTRAIN()`: Concentration constraint for HCFC-142b

- `HALON1211_CONSTRAIN()`: Concentration constraint for halon-1211

- `HALON1301_CONSTRAIN()`: Concentration constraint for halon-1301

- `HALON2402_CONSTRAIN()`: Concentration constraint for halon-2402

- `CH3CL_CONSTRAIN()`: Concentration constraint for chloromethane

- `CH3BR_CONSTRAIN()`: Concentration constraint for bromomethane

## Note

Because these identifiers are provided as `#define` macros in the hector
code, these identifiers are provided in the R interface as functions.
Therefore, these objects must be called to use them; *e.g.*,
[`GETDATA()`](https://jgcri.github.io/hector/reference/msgtype.md)
instead of the more natural looking `GETDATA`.

## See also

Other capability identifiers:
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
[`RHO_BC()`](https://jgcri.github.io/hector/reference/rho.md),
[`TRACKING_DATE()`](https://jgcri.github.io/hector/reference/parameters.md)
