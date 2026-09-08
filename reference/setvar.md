# Set values for a Hector variable

This function selects a variable by its capability name and sets the
requested values at the requested dates. The units must also be supplied
as a single string (heterogeneous units are not supported). These are
checked against the expected unit in the code, and an error is signaled
if they don't match (i.e., there is no attempt to convert units).

## Usage

``` r
setvar(core, dates, var, values, unit)
```

## Arguments

- core:

  Hector core object

- dates:

  Vector of dates

- var:

  Capability string for the variable to set

- values:

  Values to set. Must be either a single value or a vector the same
  length as dates.

- unit:

  Unit string. Can be set to NA for unitless variables.

## See also

Other main user interface functions:
[`fetchvars()`](https://jgcri.github.io/hector/reference/fetchvars.md),
[`get_tracking_data()`](https://jgcri.github.io/hector/reference/get_tracking_data.md),
[`newcore()`](https://jgcri.github.io/hector/reference/newcore.md),
[`reset()`](https://jgcri.github.io/hector/reference/reset.md),
[`run()`](https://jgcri.github.io/hector/reference/run.md),
[`shutdown()`](https://jgcri.github.io/hector/reference/shutdown.md)
