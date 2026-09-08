# Create new biomes by splitting up an existing biome

Distributes vegetation, detritus, and soil C, and initial NPP flux from
an existing biome across multiple new biomes.

## Usage

``` r
split_biome(
  core,
  old_biome,
  new_biomes,
  fveg_c = rep(1/length(new_biomes), length(new_biomes)),
  fdetritus_c = fveg_c,
  fsoil_c = fveg_c,
  fpermafrost_c = fveg_c,
  fnpp_flux0 = fveg_c,
  ...
)
```

## Arguments

- core:

  Hector core

- old_biome:

  Name of biome that will be split up

- new_biomes:

  Names of biomes to be created

- fveg_c:

  Fractions of vegetation C distributed to each biome in `new_biomes`;
  must be the same length as it. Default is to split vegetation C evenly
  between all biomes.

- fdetritus_c:

  Fractions of detritus C distributed to each biome. Defaults to the
  same value as `fveg_c`.

- fsoil_c:

  Fractions of soil C distributed to each biome. Defaults to the same
  value as `fveg_c`.

- fpermafrost_c:

  Fraction of permafrost C distributed to each biome. Defaults to the
  same value as `fveg_c`.

- fnpp_flux0:

  Fraction of initial NPP flux distributed to each biome. Defaults to
  the same value as `fveg_c`.

- ...:

  Additional biome parameters passed to
  [`create_biome`](https://jgcri.github.io/hector/reference/create_biome.md).
