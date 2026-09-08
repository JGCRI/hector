# Rename an existing biome

This will create a new biome called \`newname\`, assign it all of the C
stocks and parameter values from biome \`oldname\`, and delete biome
\`oldname\`.

## Usage

``` r
rename_biome(core, oldname, newname)
```

## Arguments

- core:

  Handle to the Hector instance that is to be run.

- oldname:

  (character) Name of existing biome to be replaced

- newname:

  (character) Name of new biome
