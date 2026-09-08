# Identifiers for halocarbon emissions

These identifiers correspond to emissions values for halocarbons. They
are (for now) write only, meaning they can only appear in
[`SETDATA`](https://jgcri.github.io/hector/reference/msgtype.md)
messages. In all cases, the expected input units are gigagrams (`"Gg"`).

## Usage

``` r
EMISSIONS_CF4()

EMISSIONS_C2F6()

EMISSIONS_HFC23()

EMISSIONS_HFC32()

EMISSIONS_HFC4310()

EMISSIONS_HFC125()

EMISSIONS_HFC134A()

EMISSIONS_HFC143A()

EMISSIONS_HFC227EA()

EMISSIONS_HFC245FA()

EMISSIONS_SF6()

EMISSIONS_CFC11()

EMISSIONS_CFC12()

EMISSIONS_CFC113()

EMISSIONS_CFC114()

EMISSIONS_CFC115()

EMISSIONS_CCL4()

EMISSIONS_CH3CCL3()

EMISSIONS_HCFC22()

EMISSIONS_HCFC141B()

EMISSIONS_HCFC142B()

EMISSIONS_HALON1211()

EMISSIONS_HALON1301()

EMISSIONS_HALON2402()

EMISSIONS_CH3CL()

EMISSIONS_CH3BR()
```

## Functions

- `EMISSIONS_CF4()`: Emissions for CF4

- `EMISSIONS_C2F6()`: Emissions for C2F6

- `EMISSIONS_HFC23()`: Emissions for HFC-23

- `EMISSIONS_HFC32()`: Emissions for HFC-32

- `EMISSIONS_HFC4310()`: Emissions for HFC-4310

- `EMISSIONS_HFC125()`: Emissions for HFC-125

- `EMISSIONS_HFC134A()`: Emissions for HFC-134a

- `EMISSIONS_HFC143A()`: Emissions for HFC-143a

- `EMISSIONS_HFC227EA()`: Emissions for HFC-227ea

- `EMISSIONS_HFC245FA()`: Emissions for HFC-254fa

- `EMISSIONS_SF6()`: Emissions for sulfur hexafluoride

- `EMISSIONS_CFC11()`: Emissions for CFC-11

- `EMISSIONS_CFC12()`: Emissions for CFC-12

- `EMISSIONS_CFC113()`: Emissions for CFC-113

- `EMISSIONS_CFC114()`: Emissions for CFC-114

- `EMISSIONS_CFC115()`: Emissions for CFC-115

- `EMISSIONS_CCL4()`: Emissions for carbon tetrachloride

- `EMISSIONS_CH3CCL3()`: Emissions for trichloroethane

- `EMISSIONS_HCFC22()`: Emissions for HCFC-22

- `EMISSIONS_HCFC141B()`: Emissions for HcFC-141b

- `EMISSIONS_HCFC142B()`: Emissions for HCFC-142b

- `EMISSIONS_HALON1211()`: Emissions for halon-1211

- `EMISSIONS_HALON1301()`: Emissions for halon-1301

- `EMISSIONS_HALON2402()`: Emissions for halon-2402

- `EMISSIONS_CH3CL()`: Emissions for chloromethane

- `EMISSIONS_CH3BR()`: Emissions for bromomethane

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
[`EMISSIONS_SO2()`](https://jgcri.github.io/hector/reference/so2.md),
[`FTOT_CONSTRAIN()`](https://jgcri.github.io/hector/reference/constraints.md),
[`GLOBAL_TAS()`](https://jgcri.github.io/hector/reference/temperature.md),
[`NBP()`](https://jgcri.github.io/hector/reference/carboncycle.md),
[`OCEAN_UPTAKE()`](https://jgcri.github.io/hector/reference/ocean.md),
[`RF_CF4()`](https://jgcri.github.io/hector/reference/haloforcings.md),
[`RF_TOTAL()`](https://jgcri.github.io/hector/reference/forcings.md),
[`RHO_BC()`](https://jgcri.github.io/hector/reference/rho.md),
[`TRACKING_DATE()`](https://jgcri.github.io/hector/reference/parameters.md)
