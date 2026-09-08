# Create new biome

Create new biome

## Usage

``` r
create_biome(
  core,
  biome,
  veg_c0,
  detritus_c0,
  soil_c0,
  permafrost_c0,
  npp_flux0,
  warmingfactor,
  beta,
  q10_rh,
  f_nppv,
  f_nppd,
  f_litterd
)
```

## Arguments

- core:

  Hector core

- biome:

  Name of new biome

- veg_c0:

  Initial vegetation C pool

- detritus_c0:

  Initial detritus C pool

- soil_c0:

  Initial soil C pool

- permafrost_c0:

  Initial permafrost C pool

- npp_flux0:

  Initial net primary productivity

- warmingfactor:

  Temperature multiplier (default = \`1.0\`)

- beta:

  CO2 fertilization effect (default = \`0.36\`)

- q10_rh:

  Q10 of heterotrophic respiration (default = \`2.0\`)

- f_nppv:

  Fraction of NPP to vegetation (default = \`0.35\`)

- f_nppd:

  Fraction of NPP to detritus (default = \`0.60\`)

- f_litterd:

  Fraction of litter flux to detritus (default = \`0.98\`)

## Value

Hector core, invisibly

## Author

Alexey Shiklomanov
