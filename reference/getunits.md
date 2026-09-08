# Get the hector unit strings for one or more variables

This function provides units only for variables that can be set in the
model using
[`setvar`](https://jgcri.github.io/hector/reference/setvar.md).
Variables read from the model with
[`fetchvars`](https://jgcri.github.io/hector/reference/fetchvars.md)
will have a units column in the data frame they are returned in.

## Usage

``` r
getunits(vars)
```

## Arguments

- vars:

  Capability strings for variables to get units for.

## Value

Character vector of unit strings. Variables not found will have `NA`
entries.
